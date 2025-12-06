#pragma once


#include <chrono>
#include <iostream>
#include <string>


class Timer
{
public:
    using Period = std::chrono::duration<double, std::micro>;
    using Clock = std::chrono::steady_clock;
    using TimePoint = std::chrono::time_point<Clock, Period>;

    Timer(const std::string &message)
        :
        active_(true),
        message_(message),
        begin_(Clock::now())
    {

    }

    Timer(const Timer &) = delete;
    Timer(Timer &&) = delete;

    Timer & operator=(const Timer &) = delete;
    Timer & operator=(Timer &&) = delete;

    ~Timer()
    {
        this->Report();
    }

    void Report()
    {
        if (!this->active_)
        {
            return;
        }

        Clock::time_point end = Clock::now();

        auto count =
            std::chrono::duration_cast<std::chrono::microseconds>(
                end - this->begin_).count();

        std::cout << this->message_ << " (us): " << count << std::endl;

        this->active_ = false;
    }

private:
    bool active_;
    std::string message_;
    Clock::time_point begin_;
};
