#pragma once

#include "types.h"

#include <functional>

// If this is 0, the system will use as
// many threads as there are cores on the machine
// otherwise, it will use the number specified here
#define NUM_THREADS 0

namespace hop {

void parallel_for(std::function<void (uint64)> func, uint64 count, uint64 chunk_size = 1);

void parallel_cleanup();

void parallel_shutdown();

extern thread_local int thread_index;

} // namespace hop
