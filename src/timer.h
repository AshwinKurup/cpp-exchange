#ifndef TIMER_H
#define TIMER_H

#include <iostream>
#include <chrono>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h> // Required for custom formatters

using namespace std;
using namespace chrono;

class Timer {
public:
    steady_clock::time_point start;
    string taskName;

    // Constructor to start the timer and optionally assign a task name
    Timer(const string& task = "Unnamed Task") : taskName(task), start(steady_clock::now()) {}

    // Log the time elapsed since the timer was created
    ~Timer() {
        auto end = steady_clock::now();
        auto duration = duration_cast<milliseconds>(end - start);

        // Log the elapsed time using spdlog
        spdlog::info("{} took {} ms", taskName, duration.count());
    }
};

#endif // TIMER_H
