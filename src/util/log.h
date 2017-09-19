#pragma once

#include "types.h"

#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <chrono>
#include <iomanip>
#include <ctime>

#define LOG_COLOR
#define LOG_TIMESTAMP

namespace hop {

//
// Example usage:
// Log("name") << INFO << ... << ...;
// Log() << DEBUG << ... << ...;
//

enum LogLevel
{
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Log
{
public:
    Log(const std::string& name = "")
        : m_name(name), m_log_off(false)
    {
    }

    ~Log()
    {
        if (!m_log_off && !m_stream.str().empty())
        {
            m_stream << std::endl;
            std::cerr << m_stream.str();
        }
    }

    Log& operator<<(LogLevel level)
    {
        if (level < g_level)
        {
            m_log_off = true;
            return *this;
        }
        else
        {
            make_header(level, ++g_num);
            return *this;
        }
    }

    template <typename T>
    Log& operator<<(const T& value)
    {
        if (!m_log_off)
            m_stream << value;
        return *this;
    }

    static void set_log_level(LogLevel level)
    {
        g_level = level;
    }

private:

    void make_header(LogLevel level, int num)
    {
    #ifdef LOG_COLOR
        m_stream << "\033[";
        switch (level) {
        case DEBUG:
            m_stream << "1;32m"; break;
        case INFO:
            m_stream << "1;34m"; break;
        case WARNING:
            m_stream << "1;33m"; break;
        case ERROR:
            m_stream << "1;31m"; break;
        }
    #endif

    #ifdef LOG_TIMESTAMP
        using namespace std::chrono;
        auto timepoint = high_resolution_clock::now();
        std::time_t t = system_clock::to_time_t(timepoint);
        auto millis = duration_cast<milliseconds>(timepoint.time_since_epoch());
        uint64 millis_remainder = millis.count() % 1000;

        m_stream << std::put_time(std::localtime(&t), "[%H:%M:%S.")
                 << std::setfill('0') << std::setw(3) << millis_remainder << "] ";
    #endif

        const char* levels[] = { "DEBUG", "INFO", "WARN", "ERROR" };
        m_stream << '[' << levels[(int)level % 4] << "] " << std::setw(3) << num << ". ";

    #ifdef LOG_COLOR
        m_stream << "\033[0m";
    #endif

        if (!m_name.empty())
            m_stream << "[" << m_name << "] ";
    }

private:
    std::ostringstream m_stream;
    std::string m_name;
    bool m_log_off;
    static int g_num;
    static LogLevel g_level;
};

} // namespace hop
