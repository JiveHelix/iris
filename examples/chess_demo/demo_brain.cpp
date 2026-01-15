#include "demo_brain.h"
#include "../common/png_settings.h"


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
    return this->filters_.color.Filter(value);
}


void DemoBrain::OnSettings_(const DemoSettings &)
{
    if (this->pngIsLoaded_)
    {
        this->Display();
    }
}


wxWindow * DemoBrain::CreateControls(wxWindow *parent)
{
    return new DemoControls(
        parent,
        this->GetUserControls(),
        this->demoControl_,
        this->demoControl_.nodeSettings);
}


void DemoBrain::Shutdown()
{
    this->displayThread_.Shutdown();
    this->GrayPngBrain<DemoBrain>::Shutdown();
}


void DemoBrain::Display()
{
    this->displayThread_.Display();
}


void DemoBrain::LoadGrayPng(const draw::GrayPng<PngPixel> &png)
{
    int32_t maximum = pngMaximum;

    this->userControl_.pixelView.asyncShapes.Set(
        draw::Shapes::MakeResetter());

    // Prevent drawing until new dimensions and source data are
    // synchronized.
    this->pngIsLoaded_ = false;

    this->demoModel_.color.range.high.SetMaximum(maximum);
    this->demoModel_.color.range.high.Set(maximum);
    this->demoModel_.maximum.Set(maximum);
    this->demoModel_.imageSize.Set(png.GetSize());
    this->filters_.source.SetData(png.GetValues().template cast<int32_t>());

    this->pngIsLoaded_ = true;

    this->filters_.chess.AutoDetectSettings();

    this->Display();
}


std::shared_ptr<draw::Pixels> DemoBrain::Process()
{
    this->userControl_.pixelView.asyncShapes.Set(
        draw::Shapes::MakeResetter());

    auto chainResult = this->filters_.chess.GetChainResults();

    this->maskBrain_.UpdateDisplay();

    if (!chainResult)
    {
        return {};
    }

    auto nodeSettings = this->demoControl_.nodeSettings.Get();

    return chainResult->Display(
        this->userControl_.pixelView.asyncShapes,
        this->demoModel_.chess.linesShape.Get(),
        this->demoModel_.chess.verticesShape.Get(),
        this->demoModel_.chessShape.Get(),
        this->filters_.color,
        {},
        &nodeSettings);
}
