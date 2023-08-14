#include "demo_brain.h"


void DemoBrain::SaveSettings() const
{
    std::cout << "TODO: Persist the processing settings." << std::endl;
}


void DemoBrain::LoadSettings()
{
    std::cout << "TODO: Restore the processing settings." << std::endl;
}


void DemoBrain::ShowAbout()
{
    wxAboutBox(MakeAboutDialogInfo("Chess Demo"));
}


std::shared_ptr<draw::Pixels>
DemoBrain::MakePixels(const iris::ProcessMatrix &value) const
{
    return std::make_shared<draw::Pixels>(this->filters_.color.Filter(value));
}


void DemoBrain::OnSettings_(const DemoSettings &)
{
    if (this->pngIsLoaded_)
    {
        this->Display();
    }
}
