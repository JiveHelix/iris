#include "demo_brain.h"


wxpex::Window DemoBrain::CreateControlFrame()
{
    auto window = wxpex::Window(new MainFrame(
        this->GetUserControls(),
        DemoControl(this->demoModel_)));

    return window;
}
