#include <iostream>
#include <wxpex/app.h>
#include <wxpex/wxshim_app.h>
#include <wxpex/file_field.h>
#include <wxpex/layout_items.h>

#include <draw/pixels.h>
#include <tau/color_map_settings.h>
#include <draw/views/color_map_settings_view.h>

#include <iris/gaussian_settings.h>
#include <iris/gradient_settings.h>
#include <iris/canny_settings.h>

#include <iris/gaussian.h>
#include <iris/gradient.h>
#include <iris/canny.h>
#include <iris/color_map.h>

#include <iris/views/gaussian_settings_view.h>
#include <iris/views/gradient_settings_view.h>
#include <iris/views/canny_settings_view.h>

#include "common/observer.h"
#include "common/gray_png_brain.h"
#include "common/display_thread.h"


template<typename T>
struct DemoFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::gaussian, "gaussian"),
        fields::Field(&T::gradient, "gradient"),
        fields::Field(&T::canny, "canny"),
        fields::Field(&T::color, "color"));
};


template<template<typename> typename T>
struct DemoTemplate
{
    T<iris::GaussianGroup<iris::InProcess>> gaussian;
    T<iris::GradientGroup<iris::InProcess>> gradient;
    T<iris::CannyGroup<float>> canny;
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

        auto gradient = new iris::GradientSettingsView<iris::InProcess>(
            this,
            control.gradient,
            nullptr,
            layoutOptions);

        gradient->Expand();

        auto canny = new iris::CannySettingsView<float>(
            this,
            control.canny,
            nullptr,
            layoutOptions);

        canny->Expand();

        auto color = new draw::ColorMapSettingsView<iris::InProcess>(
            this,
            control.color,
            nullptr,
            layoutOptions);

        auto sizer = wxpex::LayoutItems(
            wxpex::verticalItems,
            fileSelector,
            gaussian,
            gradient,
            canny,
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
    using Gradient = iris::Gradient<iris::InProcess>;
    using Canny = iris::Canny<float>;
    using Color = tau::ColorMap<iris::InProcess>;

    DemoBrain()
        :
        GrayPngBrain<DemoBrain>(),
        observer_(this, UserControl(this->user_)),
        demoModel_(),

        demoEndpoint_(
            this,
            DemoControl(this->demoModel_),
            &DemoBrain::OnSettings_),

        gaussian_(this->demoModel_.gaussian.Get()),
        gradient_(this->demoModel_.gradient.Get()),
        canny_(this->demoModel_.canny.Get()),
        color_(this->demoModel_.color.Get()),

        displayThread_(
            this->userControl_.pixelView.asyncPixels,
            iris::CancelControl(this->cancel_),
            std::bind(&DemoBrain::Process, this))
    {
        this->demoModel_.gradient.enable = false;
        this->demoModel_.gradient.maximum.Set(pngMaximum);
        this->demoModel_.color.maximum.Set(pngMaximum);
        this->demoModel_.color.range.high.Set(pngMaximum);
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
        return "Canny Demo";
    }

    wxWindow * CreateControls(wxWindow *parent)
    {
        return new DemoControls(
            parent,
            this->GetUserControls(),
            DemoControl(this->demoModel_));
    }

    std::shared_ptr<draw::Pixels>
    MakePixels(const iris::ProcessMatrix &value) const
    {
        return this->color_.Filter(value);
    }

    std::shared_ptr<draw::Pixels> Process()
    {
        bool gaussianEnabled;
        bool gradientEnabled;
        bool cannyEnabled;
        tau::MonoImage<iris::InProcess> sourcePixels;
        Gaussian gaussian;
        Gradient gradient;
        Canny canny;
        Color color;
        float scale;
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
            gaussianEnabled = this->demoModel_.gaussian.enable.Get();
            gradientEnabled = this->demoModel_.gradient.enable.Get();
            cannyEnabled = this->demoModel_.canny.enable.Get();
            gaussian = this->gaussian_;
            gradient = this->gradient_;
            canny = this->canny_;
            color = this->color_;

            scale = static_cast<float>(
                this->demoModel_.color.range.high.GetMaximum());
        }

        {
            std::lock_guard lock(this->sourceMutex_);

            auto minimumMargins =
                tau::Margins::Create(
                    std::max(
                        gaussian.GetSize(),
                        gradient.GetSize()) / 2);

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

        std::lock_guard lock(this->mutex_);

        tau::MonoImage<iris::InProcess> processed(
            sourcePixels.rows(),
            sourcePixels.cols());

        if (gaussianEnabled)
        {
            gaussian.Filter(sourcePixels, processed);
        }
        else
        {
            processed = sourcePixels;
        }

        if (!gradientEnabled)
        {
            return this->MakePixels(processed);
        }

        // Gradient is enabled.
        typename Gradient::Result gradientResult{};
        gradient.Filter(processed, gradientResult);

        if (!cannyEnabled)
        {
            return gradientResult.Colorize(margins);
        }

        typename Canny::Result cannyResult{};
        canny.Filter(gradientResult, cannyResult);

        return cannyResult.Colorize(margins);
    }

private:
    void OnSettings_(const DemoSettings &settings)
    {
        {
            std::lock_guard lock(this->mutex_);
            this->gaussian_ = Gaussian(settings.gaussian);
            this->gradient_ = Gradient(settings.gradient);
            this->canny_ = Canny(settings.canny);
            this->color_ = Color(settings.color);
        }

        this->displayThread_.Display();
    }

private:
    Observer<DemoBrain> observer_;
    DemoModel demoModel_;
    pex::Endpoint<DemoBrain, DemoControl> demoEndpoint_;
    Gaussian gaussian_;
    Gradient gradient_;
    Canny canny_;
    Color color_;
    DisplayThread displayThread_;
};


// Creates the main function for us, and initializes the app's run loop.
wxshimAPP(wxpex::App<DemoBrain>)
