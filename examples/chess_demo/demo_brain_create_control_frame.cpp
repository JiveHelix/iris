#include "demo_brain.h"


wxWindow * DemoBrain::CreateControls(wxWindow *parent)
{
    return new DemoControls(
        parent,
        this->GetUserControls(),
        DemoControl(this->demoModel_));
}
