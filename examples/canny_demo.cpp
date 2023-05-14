#include <iostream>
#include <wxpex/app.h>
#include <wxpex/file_field.h>

#include <iris/pixels.h>

#include <iris/gaussian_settings.h>
#include <iris/gradient_settings.h>
#include <iris/canny_settings.h>
#include <iris/color_settings.h>

#include <iris/gaussian.h>
#include <iris/gradient.h>
#include <iris/canny.h>
#include <iris/color.h>

#include <iris/views/gaussian_settings_view.h>
#include <iris/views/gradient_settings_view.h>
#include <iris/views/canny_settings_view.h>
#include <iris/views/color_settings_view.h>

#include "common/about_window.h"
#include "common/observer.h"
#include "common/brain.h"


template<typename T>
struct DemoFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::gaussian, "gaussian"),
        fields::Field(&T::gradient, "gradient"),
        fields::Field(&T::canny, "canny"),
        fields::Field(&T::color, "color"));
};


using InProcess = int32_t;


template<template<typename> typename T>
struct DemoTemplate
{
    T<iris::GaussianGroupMaker<InProcess>> gaussian;
    T<iris::GradientGroupMaker<InProcess>> gradient;
    T<iris::CannyGroupMaker<float>> canny;
    T<iris::ColorGroupMaker<InProcess>> color;
};


using DemoGroup = pex::Group<DemoFields, DemoTemplate>;
using DemoSettings = typename DemoGroup::Plain;
using DemoModel = typename DemoGroup::Model;
using DemoControl = typename DemoGroup::template Control<void>;

template<typename Observer>
using DemoTerminus = typename DemoGroup::template Terminus<Observer>;


class DemoMainFrame: public wxFrame
{
public:
    DemoMainFrame(
        UserControl userControl,
        DemoControl control)
        :
        wxFrame(nullptr, wxID_ANY, "Canny Demo"),
        shortcuts_(
            std::make_unique<wxpex::MenuShortcuts>(
                wxpex::Window(this),
                MakeShortcuts(userControl)))
    {
        this->SetMenuBar(this->shortcuts_->GetMenuBar());

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

        auto gaussian = new iris::GaussianSettingsView<InProcess>(
            this,
            control.gaussian,
            layoutOptions);

        gaussian->Expand();

        auto gradient = new iris::GradientSettingsView<InProcess>(
            this,
            control.gradient,
            layoutOptions);

        gradient->Expand();

        auto canny = new iris::CannySettingsView<float>(
            this,
            control.canny,
            layoutOptions);

        canny->Expand();

        auto color = new iris::ColorSettingsView<InProcess>(
            this,
            control.color,
            layoutOptions);

        sizer->Add(fileSelector, 0, wxEXPAND | wxBOTTOM, 5);
        sizer->Add(gaussian, 1, wxEXPAND | wxBOTTOM, 5);
        sizer->Add(gradient, 1, wxEXPAND | wxBOTTOM, 5);
        sizer->Add(canny, 1, wxEXPAND | wxBOTTOM, 5);
        sizer->Add(color, 1, wxEXPAND | wxBOTTOM, 5);
        auto topSizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
        topSizer->Add(sizer.release(), 1, wxEXPAND | wxALL, 5);
        this->SetSizerAndFit(topSizer.release());
    }

private:
    std::unique_ptr<wxpex::MenuShortcuts> shortcuts_;
};


class DemoBrain: public Brain<DemoBrain>
{
public:
    using Gaussian = iris::Gaussian<InProcess, 0>;
    using Gradient = iris::Gradient<InProcess>;
    using Canny = iris::Canny<float>;
    using Color = iris::Color<InProcess>;

    DemoBrain()
        :
        observer_(this, UserControl(this->user_)),
        demoModel_(),
        demoTerminus_(this, this->demoModel_),
        gaussian_(this->demoModel_.gaussian.Get()),
        gradient_(this->demoModel_.gradient.Get()),
        canny_(this->demoModel_.canny.Get()),
        color_(this->demoModel_.color.Get())
    {
        this->demoModel_.gradient.enable = false;
        this->demoModel_.gradient.maximumInput.Set(8096);
        this->demoModel_.color.level.SetMaximumValue(8096);
        this->demoModel_.color.level.high.Set(8096);
        this->color_ = Color(this->demoModel_.color.Get());
        this->demoTerminus_.Connect(&DemoBrain::OnSettings_);
    }

    wxpex::Window CreateControlFrame()
    {
        return new DemoMainFrame(
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
        wxAboutBox(MakeAboutDialogInfo("Canny Demo"));
    }

    using ProcessMatrix =
        Eigen::Matrix
        <
            InProcess,
            Eigen::Dynamic,
            Eigen::Dynamic,
            Eigen::RowMajor
        >;

    iris::Pixels MakePixels(const ProcessMatrix &value) const
    {
        return this->color_.Filter(value);
    }

    iris::Pixels Process() const
    {
        std::lock_guard lock(this->mutex_);

        auto scale =
            static_cast<float>(this->demoModel_.color.level.high.GetMaximum());

        ProcessMatrix processed =
            this->png_.GetValue(scale).template cast<InProcess>();

        if (this->demoModel_.gaussian.enable)
        {
            processed = this->gaussian_.FilterExtend(processed);
        }

        if (!this->demoModel_.gradient.enable)
        {
            return this->MakePixels(processed);
        }

        // Gradient is enabled.
        auto gradientResult = this->gradient_.Filter(processed);

        if (!this->demoModel_.canny.enable)
        {
            return gradientResult.Colorize<uint8_t>();
        }

        return this->canny_.Filter(gradientResult).Colorize<uint8_t>();
    }

    void Display()
    {
        if (this->png_)
        {
            this->user_.pixelView.pixels.Set(this->Process());
        }
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

        this->Display();
    }

private:
    Observer<DemoBrain> observer_;
    DemoModel demoModel_;
    DemoTerminus<DemoBrain> demoTerminus_;
    Gaussian gaussian_;
    Gradient gradient_;
    Canny canny_;
    Color color_;
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP_CONSOLE(wxpex::App<DemoBrain>)
