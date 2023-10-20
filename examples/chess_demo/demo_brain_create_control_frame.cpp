#include "demo_brain.h"


wxWindow * DemoBrain::CreateControls(wxWindow *parent)
{
    return new DemoControls(
        parent,
        this->GetUserControls(),
        this->demoControl_,
        this->demoControl_.nodeSettings);
}
