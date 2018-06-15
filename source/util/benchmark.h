//
// Created by cicerolp on 6/15/18.
//

#pragma once

#include <ctime>
#include <chrono>

#include <iomanip>
#include <cstdint>

#include <iostream>

#ifdef _POSIX_TIMERS
/*
CLOCK_REALTIME    System-wide realtime clock. Setting this clock requires appropriate privileges.
CLOCK_MONOTONIC    Clock that cannot be set and represents monotonic time since some unspecified starting point.
CLOCK_PROCESS_CPUTIME_ID     High-resolution per-process timer from the CPU.
CLOCK_THREAD_CPUTIME_ID    Thread-specific CPU-time clock.
*/

template<clockid_t T>
struct unixTimer {
    struct timespec t_start ,t_stop;
    uint64_t diff;

    unixTimer(){
    }

    void inline start(){
        clock_gettime(T, &t_start);
    }

    void inline stop(){
        clock_gettime(T, &t_stop);
    }

    double milliseconds(){
        return ((double) nanoseconds() ) / 1e6;
    }

    uint64_t nanoseconds(){
        diff = 1e9 * (t_stop.tv_sec - t_start.tv_sec) + t_stop.tv_nsec - t_start.tv_nsec;
        return diff;
    }
};
#endif

template<typename T>
struct stdTimer {
  std::chrono::time_point<T> _start, _stop;

  stdTimer() {
  }

  void start() {
    _start = T::now();
  }

  void stop() {
    _stop = T::now();
  }

  std::chrono::duration<double> diff() {
    return _stop - _start;
  }

  double milliseconds() {
    return std::chrono::duration<double, std::milli>(diff()).count();
  }
  double nanoseconds() {
    return std::chrono::duration<double, std::nano>(diff()).count();
  }
};

#ifdef _POSIX_TIMERS
using Timer = unixTimer<CLOCK_MONOTONIC>; //POSIX Timers
#else
// time resolution: high_resolution_clock
// represents the clock with the smallest tick period provided by the implementation.
using Timer = stdTimer<std::chrono::steady_clock>; //STL timers
#endif

// variadic c++ template
void inline printcsv() {} // termination version

template<typename First, typename ...Rest>
void inline printcsv(First &&first, Rest &&...rest) {
  std::cout << std::forward<First>(first) << " ; ";
  printcsv(std::forward<Rest>(rest)...);
}

// prepend some extra information to the stream
#define PRINTCSVL(...) do { \
    std::cout << "[" << __FILE__<<":"<< std::setw(4) << __LINE__<< "] ; ";\
    printcsv( __VA_ARGS__ ) ; \
    std::cout << std::endl ;\
} while (0)

#define PRINTCSVF(...) do { \
    std::cout << "[ " << __FUNCTION__ <<" ] ; ";\
    printcsv( __VA_ARGS__ ) ; \
    std::cout << std::endl ;\
} while (0)