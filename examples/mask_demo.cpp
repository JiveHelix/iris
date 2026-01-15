#define ENABLE_NODE_LOG

#include <mutex>
#include <condition_variable>
#include <thread>
#include <wxpex/app.h>
#include <wxpex/wxshim_app.h>
#include <wxpex/file_field.h>

#include <draw/pixels.h>
#include <tau/color_map_settings.h>
#include <draw/views/color_map_settings_view.h>

#include <iris/mask.h>
#include <iris/node.h>
#include <iris/color_map.h>

#include <iris/views/mask_settings_view.h>
#include <iris/views/mask_brain.h>

#include "common/about_window.h"
#include "common/observer.h"
#include "common/gray_png_brain.h"
#include "common/png_settings.h"
#include "common/display_thread.h"


template<typename T>
struct DemoFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::mask, "mask"),
        fields::Field(&T::color, "color"));
};


template<template<typename> typename T>
struct DemoTemplate
{
    T<iris::MaskGroup> mask;
    T<tau::ColorMapSettingsGroup<iris::InProcess>> color;

    static constexpr auto fields = DemoFields<DemoTemplate>::fields;
};


using DemoGroup = pex::Group<DemoFields, DemoTemplate>;
using DemoSettings = typename DemoGroup::Plain;
using DemoModel = typename DemoGroup::Model;
using DemoControl = typename DemoGroup::DefaultControl;


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

        auto color = new draw::ColorMapSettingsView<iris::InProcess>(
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
    using Mask = iris::Mask<iris::InProcess>;
    using Color = iris::ThreadsafeColorMap<iris::InProcess>;

    using MaskNode = iris::Node<SourceNode, Mask, iris::MaskControl>;

    SourceNode source;
    MaskNode mask;
    Color color;

    template<typename Controls>
    Filters(Controls controls, const iris::CancelControl &cancelControl)
        :
        source(),
        mask(
            "mask",
            this->source,
            controls.mask,
            cancelControl),
        color(controls.color)
    {

    }
};


class DemoBrain: public GrayPngBrain<DemoBrain>
{
public:
    using Base = GrayPngBrain<DemoBrain>;

    DemoBrain()
        :
        Base(),
        observer_(this, UserControl(this->user_)),
        demoModel_(),

        maskBrain_(
            iris::MaskControl(this->demoModel_.mask),
            this->userControl_.pixelView),

        demoEndpoint_(
            this,
            DemoControl(this->demoModel_),
            &DemoBrain::OnSettings_),

        filters_(
            DemoControl(this->demoModel_), iris::CancelControl(this->cancel_)),

        displayThread_(
            this->userControl_.pixelView.asyncPixels,
            iris::CancelControl(this->cancel_),
            std::bind(&DemoBrain::Process, this))
    {
        this->demoModel_.color.maximum.Set(pngMaximum);
        this->demoModel_.color.range.high.Set(pngMaximum);
    }

    std::string GetAppName() const
    {
        return "Mask Demo";
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
        return this->filters_.color.Filter(value);
    }

    std::shared_ptr<draw::Pixels> Process()
    {
        {
            std::lock_guard lock(this->sourceMutex_);

            if (!this->png_)
            {
                return {};
            }
        }

        auto maskResult = this->filters_.mask.GetResult();

        if (maskResult && !this->cancel_.Get())
        {
            return this->MakePixels(*maskResult);
        }

        return {};
    }

    void Display()
    {
        this->displayThread_.Display();
    }

    void Shutdown()
    {
        this->displayThread_.Shutdown();
        Brain<DemoBrain>::Shutdown();
    }

    void SetPngData(const SourceType &data)
    {
        this->filters_.source.SetData(data);
    }

private:
    void OnSettings_(const DemoSettings &)
    {
        if (this->png_)
        {
            this->Display();
        }
    }

private:
    Observer<DemoBrain> observer_;
    DemoModel demoModel_;
    iris::MaskBrain maskBrain_;
    pex::Endpoint<DemoBrain, DemoControl> demoEndpoint_;
    Filters filters_;
    DisplayThread displayThread_;
};


// Creates the main function for us, and initializes the app's run loop.
wxshimAPP(wxpex::App<DemoBrain>)
