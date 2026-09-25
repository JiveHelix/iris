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
#include <iris/gradient_settings.h>

#include <iris/gaussian.h>
#include <iris/gradient.h>
#include <iris/color_map.h>

#include <iris/views/gaussian_settings_view.h>
#include <iris/views/gradient_settings_view.h>

#include "common/observer.h"
#include "common/gray_png_brain.h"
#include "common/display_thread.h"
#include "common/timer.h"


template<template<typename> typename T>
struct DemoSchema
{
    T<iris::GaussianGroup<iris::InProcess>> gaussian;
    T<iris::GradientGroup<iris::InProcess>> gradient;
    T<tau::ColorMapSettingsGroup<iris::InProcess>> color;
};


using DemoGroup = pex::Group<DemoSchema>;
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
            gradient,
            color);

        auto topSizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
        topSizer->Add(sizer.release(), 1, wxEXPAND | wxALL, 5);
        this->SetSizerAndFit(topSizer.release());
    }
};


struct Nodes
{
    using Gaussian = iris::Gaussian<iris::InProcess, 0>;
    using Gradient = iris::Gradient<iris::InProcess>;

    using GaussianNode =
        iris::Node<GrayPngSource, Gaussian, iris::GaussianControl<iris::InProcess>>;

    GrayPngSource source;
    GaussianNode gaussian;
    iris::GradientNode<GaussianNode> gradient;

    Nodes(
        const iris::CancelControl &cancelControl,
        const DemoControl &demoControl)
        :
        source(),

        gaussian(
            "gaussian",
            this->source,
            demoControl.gaussian,
            cancelControl),

        gradient(
            this->gaussian,
            demoControl.gradient,
            cancelControl)
    {

    }
};


class DemoBrain: public GrayPngBrain<DemoBrain>
{
public:
    using Base = GrayPngBrain<DemoBrain>;

    using Gaussian = iris::Gaussian<iris::InProcess, 0>;
    using Gradient = iris::Gradient<iris::InProcess>;
    using Color = tau::ColorMap<iris::InProcess>;

    DemoBrain()
        :
        GrayPngBrain<DemoBrain>(),
        observer_(this, UserControl(this->user_)),
        demoModel_(),

        nodes_(
            iris::CancelControl(this->cancel_),
            DemoControl(this->demoModel_)),

        demoEndpoint_(
            this,
            this->demoModel_,
            &DemoBrain::OnSettings_),

        color_(this->demoModel_.color.Get()),

        displayThread_(
            this->userControl_.pixelView.asyncPixels,
            iris::CancelControl(this->cancel_),
            std::bind(&DemoBrain::Process, this))
    {
        auto defer = pex::MakeDefer(this->demoModel_);
        this->demoModel_.color.maximum.Set(pngMaximum);
        defer.gradient.maximum.Set(pngMaximum);
        defer.gradient.enable = false;
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
        Color color;

        {
            std::lock_guard lock(this->sourceMutex_);

            if (!this->png_)
            {
                return {};
            }

            color = this->color_;
        }

        auto gradientResult = this->nodes_.gradient.GetResult();

        if (gradientResult)
        {
            return gradientResult->Colorize();
        }

        auto margins = this->nodes_.source.GetMargins();
        auto gaussianResult = this->nodes_.gaussian.GetResult();

        if (gaussianResult)
        {
            return color.Filter(margins.RemoveMargin(*gaussianResult));
        }

        auto sourcePixels = this->nodes_.source.GetResult();

        return color.Filter(margins.RemoveMargin(*sourcePixels));
    }

    void SetPngData(const SourceType &data)
    {
        this->nodes_.source.SetData(data);
    }

private:
    void OnSettings_(const DemoSettings &settings)
    {
        Timer onSettingsTimer("OnSettings");

        {
            Timer lockTimer("acquire lock");
            std::lock_guard lock(this->mutex_);
            lockTimer.Report();

            this->color_ = Color(settings.color);
        }

        onSettingsTimer.Report();

        Timer displayTimer("Display");
        this->displayThread_.Display();
    }


private:
    Observer<DemoBrain> observer_;
    DemoModel demoModel_;
    Nodes nodes_;
    pex::Endpoint<DemoBrain, DemoControl> demoEndpoint_;
    Color color_;
    DisplayThread displayThread_;
};


// Creates the main function for us, and initializes the app's run loop.
wxshimAPP(wxpex::App<DemoBrain>)
