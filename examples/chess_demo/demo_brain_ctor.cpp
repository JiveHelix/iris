#include "demo_brain.h"


DemoBrain::DemoBrain()
    :
    GrayPngBrain<DemoBrain>(),
    observer_(this, UserControl(this->user_)),
    demoModel_(),
    demoControl_(this->demoModel_),

    maskBrain_(
        this->demoControl_.chess.mask,
        this->userControl_.pixelView),

    demoEndpoint_(this, this->demoControl_, &DemoBrain::OnSettings_),
    pngIsLoaded_(false),
    filters_(this->demoControl_),

    displayThread_(
        this->userControl_.pixelView.asyncPixels,
        iris::CancelControl(this->cancel_),
        std::bind(&DemoBrain::Process, this))
{
    this->demoModel_.color.turbo.Set(false);
}
