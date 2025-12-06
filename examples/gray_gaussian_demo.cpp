// #define TAU_CONVOLVE_TRANSPOSE_MAJOR
#include <iostream>
#include <wxpex/app.h>
#include <wxpex/wxshim_app.h>
#include <wxpex/file_field.h>
#include <wxpex/layout_items.h>

#include <draw/pixels.h>
#include <tau/color_map_settings.h>
#include <draw/views/color_map_settings_view.h>

#include <iris/node.h>
#include <iris/gaussian_settings.h>

#include <iris/gaussian.h>
#include <iris/color_map.h>

#include <iris/views/gaussian_settings_view.h>

#include "common/observer.h"
#include "common/gray_png_brain.h"
#include "common/display_thread.h"
#include "common/timer.h"



template<typename T>
struct DemoFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::gaussian, "gaussian"),
        fields::Field(&T::color, "color"));
};


template<template<typename> typename T>
struct DemoTemplate
{
    T<iris::GaussianGroup<iris::InProcess>> gaussian;
    T<tau::ColorMapSettingsGroup<iris::InProcess>> color;
};


using DemoGroup = pex::Group<DemoFields, DemoTemplate>;
using DemoSettings = typename DemoGroup::Plain;
using DemoModel = typename DemoGroup::Model;
using DemoControl = typename DemoGroup::DefaultControl;


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
        wxpex::LayoutOptions layoutOptions{};
        layoutOptions.labelFlags = wxALIGN_RIGHT;

        wxpex::FileDialogOptions options{};
        options.message = "Choose a PNG file";
        options.wildcard = "*.png";

        auto fileSelector = new wxpex::FileField(
            this,
            userControl.fileName,
            options);

        auto gaussian = new iris::GaussianSettingsView<iris::InProcess>(
            this,
            "Gaussian Blur",
            control.gaussian,
            nullptr,
            layoutOptions);

        gaussian->Expand();

        auto color = new draw::ColorMapSettingsView<iris::InProcess>(
            this,
            control.color,
            nullptr,
            layoutOptions);

        color->Expand();

        auto sizer = wxpex::LayoutItems(
            wxpex::verticalItems,
            fileSelector,
            gaussian,
            color);

        auto topSizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
        topSizer->Add(sizer.release(), 1, wxEXPAND | wxALL, 5);
        this->SetSizerAndFit(topSizer.release());
    }
};


class DemoBrain: public GrayPngBrain<DemoBrain>
{
public:
    using Gaussian = iris::Gaussian<iris::InProcess, 0>;
    using Color = tau::ColorMap<iris::InProcess>;

    DemoBrain()
        :
        GrayPngBrain<DemoBrain>(),
        observer_(this, UserControl(this->user_)),
        demoModel_(),

        demoEndpoint_(
            this,
            this->demoModel_,
            &DemoBrain::OnSettings_),

        gaussian_(this->demoModel_.gaussian.Get()),
        color_(this->demoModel_.color.Get()),

        displayThread_(
            this->userControl_.pixelView.asyncPixels,
            iris::CancelControl(this->cancel_),
            std::bind(&DemoBrain::Process, this))
    {
        auto defer = pex::MakeDefer(this->demoModel_);
        this->demoModel_.color.maximum.Set(pngMaximum);
        defer.color.range.high.Set(pngMaximum);
        this->color_ = Color(this->demoModel_.color.Get());
    }

    void Display()
    {
        this->displayThread_.Display();
    }

    void Shutdown()
    {
        this->displayThread_.Shutdown();
        this->GrayPngBrain<DemoBrain>::Shutdown();
    }

    std::string GetAppName() const
    {
        return "Gray Gaussian Demo";
    }

    wxWindow * CreateControls(wxWindow *parent)
    {
        return new DemoControls(
            parent,
            this->GetUserControls(),
            DemoControl(this->demoModel_));
    }

    std::shared_ptr<draw::Pixels> Process()
    {
        bool enabled;
        tau::MonoImage<iris::InProcess> sourcePixels;
        Gaussian gaussian;
        Color color;
        tau::Margins margins;

        {
            std::lock_guard lock(this->sourceMutex_);

            if (!this->png_)
            {
                return {};
            }
        }

        {
            std::lock_guard lock(this->mutex_);
            enabled = this->demoModel_.gaussian.enable.Get();
            gaussian = this->gaussian_;
            color = this->color_;

        }

        {
            std::lock_guard lock(this->sourceMutex_);
            auto minimumMargins = tau::Margins::Create(gaussian.GetSize() / 2);
            margins = this->source_.GetMargins();

            if (!margins.Contains(minimumMargins))
            {
                // Our existing margins do not contain the new requirement.
                // Create new margins.
                this->source_.SetMargins(minimumMargins);
                margins = minimumMargins;
            }

            sourcePixels = *this->source_.GetResult();
        }

        tau::MonoImage<iris::InProcess> processed(
            sourcePixels.rows(),
            sourcePixels.cols());

        if (enabled)
        {
            gaussian.Filter(sourcePixels, processed);
        }
        else
        {
            processed = sourcePixels;
        }

        return color.Filter(margins.RemoveMargin(processed));
    }

private:
    void OnSettings_(const DemoSettings &settings)
    {
        Timer onSettingsTimer("OnSettings");

        {
            Timer lockTimer("acquire lock");
            std::lock_guard lock(this->mutex_);
            lockTimer.Report();

            this->gaussian_ = Gaussian(settings.gaussian);
            this->color_ = Color(settings.color);
        }

        onSettingsTimer.Report();

        Timer displayTimer("Display");
        this->displayThread_.Display();
    }

private:
    Observer<DemoBrain> observer_;
    DemoModel demoModel_;
    pex::Endpoint<DemoBrain, DemoControl> demoEndpoint_;
    Gaussian gaussian_;
    Color color_;
    DisplayThread displayThread_;
};


// Creates the main function for us, and initializes the app's run loop.
wxshimAPP(wxpex::App<DemoBrain>)
