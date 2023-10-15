#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <wxpex/app.h>
#include <wxpex/file_field.h>

#include <iris/views/mask_brain.h>

#include "../common/about_window.h"
#include "../common/observer.h"
#include "../common/brain.h"
#include "../common/png_settings.h"

#include "demo_settings.h"
#include "demo_controls.h"
#include "filters.h"


enum class DisplayState
{
    waiting,
    processing
};


class DemoBrain: public Brain<DemoBrain>
{
public:
    DemoBrain()
        :
        Brain<DemoBrain>(),
        observer_(this, UserControl(this->user_)),
        demoModel_(),

        maskBrain_(
            iris::MaskControl(this->demoModel_.mask),
            this->userControl_.pixelView),

        demoEndpoint_(
            this,
            DemoControl(this->demoModel_),
            &DemoBrain::OnSettings_),

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

    std::string GetAppName() const
    {
        return "Vertex Demo";
    }

    void LoadPng(const draw::GrayPng<PngPixel> &png)
    {
        int32_t maximum = pngMaximum;

        // Prevent drawing until new dimensions and source data are
        // synchronized.
        this->pngIsLoaded_ = false;

        this->demoModel_.color.range.high.SetMaximum(maximum);
        this->demoModel_.color.range.high.Set(maximum);
        this->demoModel_.maximum.Set(maximum);
        this->filters_.source.SetData(png.GetValues().template cast<int32_t>());

        this->pngIsLoaded_ = true;

        this->Display();
    }

    wxWindow * CreateControls(wxWindow *parent)
    {
        return new DemoControls(
            parent,
            this->GetUserControls(),
            DemoControl(this->demoModel_));
    }

    void SaveSettings() const
    {
        std::cout << "TODO: Persist the processing settings." << std::endl;
    }

    void LoadSettings()
    {
        std::cout << "TODO: Restore the processing settings." << std::endl;
    }

    void ShowAbout()
    {
        wxAboutBox(MakeAboutDialogInfo("Vertex Demo"));
    }

    std::shared_ptr<draw::Pixels>
    MakePixels(const iris::ProcessMatrix &value) const
    {
        return std::make_shared<draw::Pixels>(
            this->filters_.color.Filter(value));
    }

    std::shared_ptr<draw::Pixels> Process()
    {
        this->userControl_.pixelView.asyncShapes.Set(
            draw::Shapes::MakeResetter());

        auto vertexResult = this->filters_.vertexChain.GetChainResults();

        if (!vertexResult)
        {
            auto levelResult = this->filters_.level.GetResult();

            if (!levelResult)
            {
                return {};
            }

            return this->MakePixels(*levelResult);
        }

        return vertexResult->Display(
            this->userControl_.pixelView.asyncShapes,
            this->demoModel_.vertexChain.shape.Get(),
            this->filters_.color);
    }

    void Display()
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

    void Shutdown()
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

private:
    void OnSettings_(const DemoSettings &)
    {
        if (this->pngIsLoaded_)
        {
            this->Display();
        }
    }

    void DisplayLoop_()
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

private:
    Observer<DemoBrain> observer_;
    DemoModel demoModel_;
    iris::MaskBrain maskBrain_;
    pex::Endpoint<DemoBrain, DemoControl> demoEndpoint_;
    bool pngIsLoaded_;
    Filters filters_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    DisplayState displayState_;
    std::atomic_bool displayRequested_;
    std::atomic_bool displayLoopIsRunning_;
    std::thread displayThread_;
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP_CONSOLE(wxpex::App<DemoBrain>)
