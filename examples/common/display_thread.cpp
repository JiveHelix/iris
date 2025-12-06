#include "display_thread.h"



DisplayThread::DisplayThread(
    draw::AsyncPixelsControl asyncPixelsControl,
    iris::CancelControl cancelControl,
    const ProcessFunction &processFunction)
    :
    asyncPixelsControl_(asyncPixelsControl),
    cancelControl_(cancelControl),
    processFunction_(processFunction),
    mutex_(),
    condition_(),
    displayState_(),
    displayLoopIsRunning_(true),
    displayRequested_(false),

    displayThread_(std::bind(&DisplayThread::DisplayLoop_, this))
{

}


DisplayThread::~DisplayThread()
{
    this->Shutdown();
}


void DisplayThread::Display()
{
    std::lock_guard lock(this->mutex_);

    if (this->displayState_ == DisplayState::processing)
    {
        this->cancelControl_.Set(true);
    }

    this->displayRequested_ = true;
    this->condition_.notify_one();
}


void DisplayThread::Shutdown()
{
    if (!this->displayThread_.joinable())
    {
        return;
    }

    {
        std::lock_guard lock(this->mutex_);
        this->displayLoopIsRunning_ = false;
        this->cancelControl_.Set(true);
        this->condition_.notify_one();
    }

    this->displayThread_.join();
}


void DisplayThread::DisplayLoop_()
{
    while (this->displayLoopIsRunning_)
    {
        {
            std::unique_lock lock(this->mutex_);

            if (!this->displayRequested_)
            {
                // Sleep until display is requested
                this->displayState_ = DisplayState::waiting;

                this->condition_.wait(
                    lock,
                    [this]() -> bool
                    {
                        return this->displayRequested_
                            || !this->displayLoopIsRunning_;
                    });
            }

            if (!this->displayLoopIsRunning_)
            {
                return;
            }

            this->displayState_ = DisplayState::processing;
        }

        auto pixels = this->processFunction_();

        {
            std::lock_guard lock(this->mutex_);

            if (this->cancelControl_.Get())
            {
                this->cancelControl_.Set(false);
                continue;
            }
            else
            {
                this->displayRequested_ = false;
            }
        }

        if (pixels)
        {
            this->asyncPixelsControl_.Set(pixels);
        }
    }
}
