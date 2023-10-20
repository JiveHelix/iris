#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <wxpex/app.h>
#include <wxpex/file_field.h>

#include <draw/pixels.h>

#include <iris/mask.h>
#include <iris/color_settings.h>
#include <iris/node.h>
#include <iris/color.h>

#include <iris/views/mask_settings_view.h>
#include <iris/views/mask_brain.h>
#include <iris/views/color_settings_view.h>

#include "common/about_window.h"
#include "common/observer.h"
#include "common/brain.h"
#include "common/png_settings.h"


template<typename T>
struct DemoFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::mask, "mask"),
        fields::Field(&T::color, "color"));
};


using InProcess = int32_t;


template<template<typename> typename T>
struct DemoTemplate
{
    T<iris::MaskGroupMaker> mask;
    T<iris::ColorGroupMaker<InProcess>> color;

    static constexpr auto fields = DemoFields<DemoTemplate>::fields;
};


using DemoGroup = pex::Group<DemoFields, DemoTemplate>;
using DemoSettings = typename DemoGroup::Plain;
using DemoModel = typename DemoGroup::Model;
using DemoControl = typename DemoGroup::Control;


DECLARE_OUTPUT_STREAM_OPERATOR(DemoSettings)


class DemoControls: public wxPanel
{
public:
    DemoControls(
        wxWindow *parent,
        UserControl userControl,
        DemoControl control)
        :
        wxPanel(parent, wxID_ANY)
    {
        auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);

        wxpex::LayoutOptions layoutOptions{};
        layoutOptions.labelFlags = wxALIGN_RIGHT;

        wxpex::FileDialogOptions options{};
        options.message = "Choose a PNG file";
        options.wildcard = "*.png";

        auto fileSelector = new wxpex::FileField(
            this,
            userControl.fileName,
            options);

        auto mask = new iris::MaskSettingsView(
            this,
            control.mask,
            {},
            layoutOptions);

        mask->Expand();

        auto color = new iris::ColorSettingsView<InProcess>(
            this,
            control.color,
            {},
            layoutOptions);

        sizer->Add(fileSelector, 0, wxEXPAND | wxBOTTOM, 5);
        sizer->Add(mask, 0, wxEXPAND | wxBOTTOM, 5);
        sizer->Add(color, 0, wxEXPAND | wxBOTTOM, 5);
        auto topSizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
        topSizer->Add(sizer.release(), 1, wxEXPAND | wxALL, 5);
        this->SetSizerAndFit(topSizer.release());
    }
};


struct Filters
{
    using SourceNode = iris::Source<iris::ProcessMatrix>;
    using Mask = iris::Mask<InProcess>;
    using Color = iris::ThreadsafeColor<InProcess>;

    using MaskNode = iris::Node<SourceNode, Mask, iris::MaskControl>;

    iris::Cancel cancel;
    SourceNode source;
    MaskNode mask;
    Color color;

    template<typename Controls>
    Filters(Controls controls)
        :
        cancel(false),
        source(),
        mask(
            "mask",
            this->source,
            controls.mask,
            iris::CancelControl(this->cancel)),
        color(controls.color)
    {

    }
};


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
        this->demoModel_.color.range.SetMaximumValue(8096);
        this->demoModel_.color.range.high.Set(8096);
    }

    std::string GetAppName() const
    {
        return "Mask Demo";
    }

    void LoadPng(const draw::GrayPng<PngPixel> &png)
    {
        int32_t maximum = pngMaximum;

        // Prevent drawing until new dimensions and source data are
        // synchronized.
        this->pngIsLoaded_ = false;

        this->demoModel_.color.range.high.SetMaximum(maximum);
        this->demoModel_.color.range.high.Set(maximum);
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
        wxAboutBox(MakeAboutDialogInfo("Mask Demo"));
    }

    std::shared_ptr<draw::Pixels>
    MakePixels(const iris::ProcessMatrix &value) const
    {
        return std::make_shared<draw::Pixels>(
            this->filters_.color.Filter(value));
    }

    std::shared_ptr<draw::Pixels> Process()
    {
        auto maskResult = this->filters_.mask.GetResult();

        if (maskResult && !this->filters_.cancel.Get())
        {
            return this->MakePixels(*maskResult);
        }

        return {};
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
