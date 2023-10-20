#include "demo_brain.h"


DemoBrain::DemoBrain()
    :
    Brain<DemoBrain>(),
    observer_(this, UserControl(this->user_)),
    demoModel_(),
    demoControl_(this->demoModel_),

    maskBrain_(
        this->demoControl_.chess.mask,
        this->userControl_.pixelView),

    demoEndpoint_(this, this->demoControl_, &DemoBrain::OnSettings_),
    pngIsLoaded_(false),
    filters_(this->demoControl_),
    mutex_(),
    condition_(),
    displayState_(DisplayState::waiting),
    displayRequested_(false),
    displayLoopIsRunning_(true),
    displayThread_(
        std::bind(&DemoBrain::DisplayLoop_, this))
{

}
