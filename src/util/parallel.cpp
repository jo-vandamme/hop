#include "util/parallel.h"
#include "hop.h"
#include "types.h"
#include "math/math.h"

#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>

namespace hop {

namespace {

class ParallelForLoop
{
public:
    ParallelForLoop(std::function<void(uint64)> func_1d, uint64 max_index, uint64 chunk_size)
        : func_1d(func_1d), max_index(max_index), chunck_size(chunk_size)
    {
    }

    bool finished() const
    {
        return next_index >= max_index && active_workers == 0;
    }

public:
    ParallelForLoop* next = nullptr;
    int active_workers = 0;
    uint64 next_index = 0;

    std::function<void(int)> func_1d;
    const uint64 max_index;
    const uint64 chunck_size;
};

std::vector<std::thread> g_threads;
bool g_shutdown_threads = false;
ParallelForLoop* g_work_list = nullptr;
std::mutex g_work_list_mutex;
std::condition_variable g_work_list_condition;

void worker_thread_func(int tindex)
{
    thread_index = tindex;
    std::unique_lock<std::mutex> lock(g_work_list_mutex);
    while (!g_shutdown_threads)
    {
        if (!g_work_list)
        {
            g_work_list_condition.wait(lock);
        }
        else
        {
            ParallelForLoop& loop = *g_work_list;

            uint64 index_start = loop.next_index;
            uint64 index_end = min(index_start + loop.chunck_size, loop.max_index);
            loop.next_index = index_end;
            if (loop.next_index == loop.max_index)
                g_work_list = loop.next;
            ++loop.active_workers;

            lock.unlock();
            for (uint64 index = index_start; index < index_end; ++index)
            {
                if (loop.func_1d)
                    loop.func_1d(index);
            }
            lock.lock();
            --loop.active_workers;

            if (loop.finished())
                g_work_list_condition.notify_all();
        }
    }
}

} // anonymous namespace

thread_local int thread_index;

void parallel_for(std::function<void (uint64)> func, uint64 count, uint64 chunk_size)
{
    if (NUM_THREADS == 1 || count < chunk_size)
    {
        for (uint64 i = 0; i < count; ++i)
            func(i);
        return;
    }

    if (g_threads.empty())
    {
        thread_index = 0;
        for (unsigned int i = 0; i < std::thread::hardware_concurrency() - 1; ++i)
            g_threads.push_back(std::thread(worker_thread_func, i + 1));
    }

    ParallelForLoop loop(func, count, chunk_size);

    g_work_list_mutex.lock();
    loop.next = g_work_list;
    g_work_list = &loop;
    g_work_list_mutex.unlock();

    std::unique_lock<std::mutex> lock(g_work_list_mutex);
    g_work_list_condition.notify_all();

    while (!loop.finished())
    {
        uint64 index_start = loop.next_index;
        uint64 index_end = min(index_start + loop.chunck_size, loop.max_index);
        loop.next_index = index_end;
        if (loop.next_index == loop.max_index)
            g_work_list = loop.next;
        ++loop.active_workers;

        lock.unlock();
        for (uint64 index = index_start; index < index_end; ++index)
        {
            if (loop.func_1d)
                loop.func_1d(index);
        }
        lock.lock();
        --loop.active_workers;
    }
}

void parallel_shutdown()
{
    std::lock_guard<std::mutex> lock(g_work_list_mutex);
    g_shutdown_threads = true;
    g_work_list_condition.notify_all();
}

void parallel_cleanup()
{
    if (g_threads.empty())
        return;

    {
        std::lock_guard<std::mutex> lock(g_work_list_mutex);
        g_shutdown_threads = true;
        g_work_list_condition.notify_all();
    }

    for (std::thread &thread : g_threads)
        thread.join();

    g_threads.erase(g_threads.begin(), g_threads.end());

    g_shutdown_threads = false;
}

} // namespace hop
