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
    if (!this->displayThread_.joinable())
    {
        Brain<DemoBrain>::Shutdown();
        return;
    }

    {
        std::lock_guard lock(this->mutex_);
        this->displayLoopIsRunning_ = false;
        this->filters_.cancel.Set(true);
        this->condition_.notify_one();
    }

    this->displayThread_.join();

    Brain<DemoBrain>::Shutdown();
}


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


void DemoBrain::DisplayLoop_()
{
    while (this->displayLoopIsRunning_)
    {
        {
            std::unique_lock lock(this->mutex_);

            if (!this->displayRequested_)
            {
                // Sleep until display is requested
                this->displayState_ = DisplayState::waiting;

                this->condition_.wait(
                    lock,
                    [this]() -> bool
                    {
                        return this->displayRequested_
                            || !this->displayLoopIsRunning_;
                    });
            }

            if (!this->displayLoopIsRunning_)
            {
                return;
            }

            this->displayState_ = DisplayState::processing;
        }

        auto pixels = this->Process();

        {
            std::lock_guard lock(this->mutex_);

            if (this->filters_.cancel.Get())
            {
                this->filters_.cancel.Set(false);
                continue;
            }
            else
            {
                this->displayRequested_ = false;
            }
        }

        if (pixels)
        {
            this->userControl_.pixelView.asyncPixels.Set(pixels);
        }
    }
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

    return chainResult->Display(
        this->userControl_.pixelView.asyncShapes,
        this->demoModel_.chess.linesShape.Get(),
        this->demoModel_.chess.verticesShape.Get(),
        this->demoModel_.chessShape.Get(),
        this->filters_.color,
        {},
        this->demoControl_.nodeSettings.Get());
}
