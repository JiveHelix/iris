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
        wxpex::Collapsible(parent, "Color", borderStyle)
    {
        auto panel = this->GetPanel();

        auto levelLow = wxpex::LabeledWidget(
            panel,
            "Low",
            new wxpex::ValueSlider(
                panel,
                control.level.low,
                control.level.low.value));

        auto levelHigh = wxpex::LabeledWidget(
            panel,
            "High",
            new wxpex::ValueSlider(
                panel,
                control.level.high,
                control.level.high.value));

        auto turboOn = wxpex::LabeledWidget(
            panel,
            "Turbo",
            new wxpex::CheckBox(panel, "", control.turbo));

        auto sizer = LayoutLabeled(
            layoutOptions,
            levelLow,
            levelHigh,
            turboOn);

        this->ConfigureTopSizer(std::move(sizer));
    }
};


} // end namespace iris
