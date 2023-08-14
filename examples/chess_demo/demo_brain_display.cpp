#include "demo_brain.h"


void DemoBrain::Display()
{
    if (!this->pngIsLoaded_)
    {
        return;
    }

    std::lock_guard lock(this->mutex_);

    if (this->displayState_ == DisplayState::processing)
    {
        this->filters_.cancel.Set(true);
    }

    this->displayRequested_ = true;
    this->condition_.notify_one();
}
