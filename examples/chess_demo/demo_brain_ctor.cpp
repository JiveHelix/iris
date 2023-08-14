#include "demo_brain.h"


DemoBrain::DemoBrain()
    :
    Brain<DemoBrain>(),
    observer_(this, UserControl(this->user_)),
    demoModel_(),

    maskBrain_(
        iris::MaskControl(this->demoModel_.chess.mask),
        this->userControl_.pixelView),

    demoEndpoint_(this, DemoControl(this->demoModel_), &DemoBrain::OnSettings_),
    pngIsLoaded_(false),
    filters_(DemoControl(this->demoModel_)),
    mutex_(),
    condition_(),
    displayState_(DisplayState::waiting),
    displayRequested_(false),
    displayLoopIsRunning_(true),
    displayThread_(
        std::bind(&DemoBrain::DisplayLoop_, this))
{

}
