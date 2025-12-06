#pragma once

#include <mutex>
#include <condition_variable>
#include <thread>
#include <draw/views/pixel_view_settings.h>
#include <iris/node.h>


enum class DisplayState
{
    waiting,
    processing
};


class DisplayThread
{
public:
    using ProcessFunction =
        std::function<std::shared_ptr<draw::Pixels> (void)>;

    DisplayThread(
        draw::AsyncPixelsControl asyncPixelsControl,
        iris::CancelControl cancelControl,
        const ProcessFunction &processFunction);

    ~DisplayThread();

    void Display();

    void Shutdown();

private:
    void DisplayLoop_();

private:
    draw::AsyncPixelsControl asyncPixelsControl_;
    iris::CancelControl cancelControl_;
    ProcessFunction processFunction_;

    mutable std::mutex mutex_;
    std::condition_variable condition_;
    DisplayState displayState_;
    bool displayLoopIsRunning_;
    bool displayRequested_;
    std::thread displayThread_;
};
