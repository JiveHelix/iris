#include "demo_brain.h"


void DemoBrain::DisplayLoop_()
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

        auto pixels = this->Process();

        {
            std::lock_guard lock(this->mutex_);

            if (this->filters_.cancel.Get())
            {
                this->filters_.cancel.Set(false);
                continue;
            }
            else
            {
                this->displayRequested_ = false;
            }
        }

        if (pixels)
        {
            this->userControl_.pixelView.asyncPixels.Set(pixels);
        }
    }
}
