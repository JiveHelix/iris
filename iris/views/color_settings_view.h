#pragma once


#include <wxpex/collapsible.h>
#include <wxpex/labeled_widget.h>
#include <wxpex/slider.h>
#include <wxpex/check_box.h>

#include "iris/color_settings.h"
#include "iris/views/defaults.h"


namespace iris
{


template<typename Value>
class ColorSettingsView: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    ColorSettingsView(
        wxWindow *parent,
        ColorControl<Value> control,
        const LayoutOptions &layoutOptions = LayoutOptions{})
        :
        wxpex::Collapsible(parent, "Color")
    {
        auto pane = this->GetBorderPane(borderStyle);

        auto levelLow = wxpex::LabeledWidget(
            pane,
            "Low",
            new wxpex::ValueSlider(
                pane,
                control.level.low,
                control.level.low.value));

        auto levelHigh = wxpex::LabeledWidget(
            pane,
            "High",
            new wxpex::ValueSlider(
                pane,
                control.level.high,
                control.level.high.value));

        auto turboOn = wxpex::LabeledWidget(
            pane,
            "Turbo",
            new wxpex::CheckBox(pane, "", control.turbo));

        auto sizer = LayoutLabeled(
            layoutOptions,
            levelLow,
            levelHigh,
            turboOn);

        this->ConfigureBorderPane(borderPixels, std::move(sizer));
    }
};


} // end namespace iris
