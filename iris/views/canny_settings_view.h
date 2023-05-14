#pragma once


#include <wxpex/collapsible.h>
#include <wxpex/labeled_widget.h>
#include <wxpex/slider.h>
#include <wxpex/radio_box.h>
#include <wxpex/view.h>

#include "iris/canny_settings.h"


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

        auto enable = wxpex::LabeledWidget(
            this->GetPane(),
            "Enable",
            new wxpex::CheckBox(this->GetPane(), "", controls.enable));

        auto high = LabeledWidget(
            this->GetPane(),
            "High",
            new ValueSlider(
                this->GetPane(),
                controls.range.high,
                controls.range.high.value));

        auto low = LabeledWidget(
            this->GetPane(),
            "Low",
            new ValueSlider(
                this->GetPane(),
                controls.range.low,
                controls.range.low.value));

        auto depth = wxpex::LabeledWidget(
            this->GetPane(),
            "Depth",
            new wxpex::Field(this->GetPane(), controls.depth));

        auto threads = wxpex::LabeledWidget(
            this->GetPane(),
            "Threads",
            new wxpex::Field(this->GetPane(), controls.threads));

        auto sizer = LayoutLabeled(
            layoutOptions,
            enable,
            high,
            low,
            threads,
            depth);

        this->ConfigureTopSizer(sizer.release());
    }
};


} // end namespace iris
