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


struct Nodes
{
    using Gaussian = iris::Gaussian<iris::InProcess, 0>;
    using Gradient = iris::Gradient<iris::InProcess>;

    using GaussianNode =
        iris::Node
        <
            GrayPngSource,
            Gaussian,
            iris::GaussianControl<iris::InProcess>
        >;

    using GradientNode = iris::GradientNode<GaussianNode>;

    using Canny = iris::Canny<float>;

    using CannyNode =
        iris::Node<GradientNode, Canny, iris::CannyControl<float>>;

    GrayPngSource source;
    GaussianNode gaussian;
    GradientNode gradient;
    CannyNode canny;

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
            cancelControl),

        canny(
            "canny",
            this->gradient,
            demoControl.canny,
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
    using Canny = iris::Canny<float>;
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
            DemoControl(this->demoModel_),
            &DemoBrain::OnSettings_),

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

        auto cannyResult = this->nodes_.canny.GetResult();

        if (cannyResult)
        {
            return cannyResult->Colorize();
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
        {
            std::lock_guard lock(this->mutex_);
            this->color_ = Color(settings.color);
        }

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
