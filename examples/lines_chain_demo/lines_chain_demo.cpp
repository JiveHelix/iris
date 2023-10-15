#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <wxpex/app.h>

#include <draw/pixels.h>
#include <draw/png.h>


#include <iris/views/mask_brain.h>

#include "../common/about_window.h"
#include "../common/observer.h"
#include "../common/brain.h"
#include "../common/png_settings.h"

#include "demo_settings.h"
#include "demo_controls.h"
#include "filters.h"


template<typename T>
struct HoughUserFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::houghView, "houghView"));
};


template<template<typename> typename T>
struct HoughUserTemplate
{
    T<draw::PixelViewGroupMaker> houghView;
};


using HoughUserGroup = pex::Group<HoughUserFields, HoughUserTemplate>;

using HoughUserControl = typename HoughUserGroup::Control;
using HoughUserModel = typename HoughUserGroup::Model;


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
        demoControl_(this->demoModel_),

        maskBrain_(
            iris::MaskControl(this->demoModel_.mask),
            this->userControl_.pixelView),

        demoEndpoint_(
            this,
            this->demoControl_,
            &DemoBrain::OnSettings_),

        houghEndpoint_(
            this,
            this->demoControl_.lines.hough,
            &DemoBrain::OnHoughSettings_),

        houghUser_{},
        houghUserControl_(this->houghUser_),
        pngIsLoaded_(false),
        filters_(this->demoControl_),
        mutex_(),
        condition_(),
        displayState_(DisplayState::waiting),
        displayRequested_(false),
        displayLoopIsRunning_(true),
        displayThread_(
            std::bind(&DemoBrain::DisplayLoop_, this)),
        doDisplay_([this](){this->Display();})
    {

    }

    ~DemoBrain()
    {
        this->Shutdown();
    }

    std::string GetAppName() const
    {
        return "Lines Demo";
    }

    void LoadPng(const draw::GrayPng<PngPixel> &png)
    {
        // Prevent drawing until new dimensions and source data are
        // synchronized.
        this->pngIsLoaded_ = false;

        this->demoModel_.imageSize.Set(png.GetSize());
        this->filters_.source.SetData(png.GetValues().template cast<int32_t>());

        this->pngIsLoaded_ = true;

        this->filters_.level.AutoDetectSettings();
        this->filters_.lines.AutoDetectSettings();

        this->Display();
    }

    void ExportPng()
    {
        draw::WritePng(
            "hough.png",
            *this->houghUserControl_.houghView.pixels.Get());
    }

    wxWindow * CreateControls(wxWindow *parent)
    {
        auto window = new DemoControls(
            parent,
            this->GetUserControls(),
            this->demoControl_);

        this->OnHoughSettings_(this->demoControl_.lines.hough.Get());
        return window;
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
        wxAboutBox(MakeAboutDialogInfo("Lines Demo"));
    }

    std::shared_ptr<draw::Pixels> MakePixels(
        const iris::ProcessMatrix &value) const
    {
        return std::make_shared<draw::Pixels>(
            this->filters_.color.Filter(value));
    }

    std::shared_ptr<draw::Pixels> Process()
    {
        this->userControl_.pixelView.asyncShapes.Set(
            draw::Shapes::MakeResetter());

        this->maskBrain_.UpdateDisplay();

        auto linesResult = this->filters_.lines.GetChainResults();

        if (!linesResult)
        {
            auto levelResult = this->filters_.level.GetResult();

            if (!levelResult)
            {
                return {};
            }

            return this->MakePixels(*levelResult);
        }

        return linesResult->Display(
            this->userControl_.pixelView.asyncShapes,
            this->demoModel_.lines.shape.Get(),
            this->filters_.color,
            this->houghUserControl_.houghView.asyncPixels);
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
        if (this->displayLoopIsRunning_)
        {
            {
                std::lock_guard lock(this->mutex_);
                this->displayLoopIsRunning_ = false;
                this->filters_.cancel.Set(true);
                this->condition_.notify_one();
            }

            this->displayThread_.join();
        }

        this->houghView_.Close();
        Brain<DemoBrain>::Shutdown();
    }

private:
    void OnSettings_(const DemoSettings &)
    {
        this->doDisplay_();
    }

    void OnHoughEnable_(bool isEnabled)
    {
        if (isEnabled)
        {
            if (!this->houghView_)
            {
                this->houghView_ = {
                    new draw::PixelFrame(
                        this->houghUserControl_.houghView,
                        "Hough Space"),
                    MakeShortcuts(this->GetUserControls())};
            }

            this->houghView_.Get()->Show();
        }
        else if (this->houghView_)
        {
            this->houghView_.Get()->Show(false);
        }
    }

    void OnHoughSettings_(const iris::HoughSettings<double> &houghSettings)
    {
        this->houghUser_.houghView.viewSettings.imageSize.Set(
            draw::Size(
                static_cast<int>(houghSettings.thetaCount),
                static_cast<int>(houghSettings.rhoCount)));

        this->OnHoughEnable_(houghSettings.enable);
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

                if (this->filters_.cancel.Get())
                {
                    this->filters_.cancel.Set(false);
                }

                this->displayState_ = DisplayState::processing;
            }

            auto pixels = this->Process();

            {
                std::lock_guard lock(this->mutex_);
                if (this->filters_.cancel.Get())
                {
                    this->filters_.cancel.Set(false);
                }

                if (pixels)
                {
                    // Processing succeeded.
                    // Go back to the top of the loop and wait.
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
    DemoControl demoControl_;
    iris::MaskBrain maskBrain_;
    pex::Endpoint<DemoBrain, DemoControl> demoEndpoint_;
    pex::Endpoint<DemoBrain, iris::HoughControl<double>> houghEndpoint_;
    HoughUserModel houghUser_;
    HoughUserControl houghUserControl_;
    wxpex::ShortcutWindow houghView_;
    bool pngIsLoaded_;
    Filters filters_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    DisplayState displayState_;
    std::atomic_bool displayRequested_;
    std::atomic_bool displayLoopIsRunning_;
    std::thread displayThread_;
    wxpex::CallAfter doDisplay_;
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP_CONSOLE(wxpex::App<DemoBrain>)
