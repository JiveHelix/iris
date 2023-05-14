#pragma once


#include <wxpex/collapsible.h>
#include <wxpex/labeled_widget.h>
#include <wxpex/slider.h>
#include <wxpex/check_box.h>

#include "iris/color_settings.h"


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
        auto levelLow = wxpex::LabeledWidget(
            this->GetPane(),
            "Low",
            new wxpex::ValueSlider(
                this->GetPane(),
                control.level.low,
                control.level.low.value));

        auto levelHigh = wxpex::LabeledWidget(
            this->GetPane(),
            "High",
            new wxpex::ValueSlider(
                this->GetPane(),
                control.level.high,
                control.level.high.value));

        auto turboOn = wxpex::LabeledWidget(
            this->GetPane(),
            "Turbo",
            new wxpex::CheckBox(this->GetPane(), "", control.turbo));

        auto sizer = LayoutLabeled(
            layoutOptions,
            levelLow,
            levelHigh,
            turboOn);

        this->ConfigureTopSizer(sizer.release());
    }
};


} // end namespace iris
