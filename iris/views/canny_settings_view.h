#pragma once


#include <wxpex/collapsible.h>
#include <wxpex/labeled_widget.h>
#include <wxpex/slider.h>
#include <wxpex/radio_box.h>
#include <wxpex/check_box.h>
#include <wxpex/view.h>

#include "iris/canny_settings.h"
#include "iris/views/defaults.h"


namespace iris
{


template<typename Value>
class CannySettingsView: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    CannySettingsView(
        wxWindow *parent,
        CannyControl<Value> controls,
        const LayoutOptions &layoutOptions = LayoutOptions{})
        :
        wxpex::Collapsible(parent, "Canny")
    {
        using namespace wxpex;

        auto pane = this->GetBorderPane(borderStyle);

        auto enable = wxpex::LabeledWidget(
            pane,
            "Enable",
            new wxpex::CheckBox(pane, "", controls.enable));

        auto high = LabeledWidget(
            pane,
            "High",
            new ValueSlider(
                pane,
                controls.range.high,
                controls.range.high.value));

        auto low = LabeledWidget(
            pane,
            "Low",
            new ValueSlider(
                pane,
                controls.range.low,
                controls.range.low.value));

        auto depth = wxpex::LabeledWidget(
            pane,
            "Depth",
            new wxpex::Field(pane, controls.depth));

        auto threads = wxpex::LabeledWidget(
            pane,
            "Threads",
            new wxpex::Field(pane, controls.threads));

        auto sizer = LayoutLabeled(
            layoutOptions,
            enable,
            high,
            low,
            threads,
            depth);

        this->ConfigureBorderPane(borderPixels, std::move(sizer));
    }
};


extern template class CannySettingsView<double>;


} // end namespace iris
