#include "demo_brain.h"


void DemoBrain::Shutdown()
{
    {
        std::lock_guard lock(this->mutex_);
        this->displayLoopIsRunning_ = false;
        this->filters_.cancel.Set(true);
        this->condition_.notify_one();
    }

    this->displayThread_.join();

    Brain<DemoBrain>::Shutdown();
}
