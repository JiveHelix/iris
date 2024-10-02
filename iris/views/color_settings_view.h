#pragma once


#include <wxpex/collapsible.h>
#include <wxpex/labeled_widget.h>
#include <wxpex/slider.h>
#include <wxpex/check_box.h>
#include <draw/views/node_settings_view.h>

#include "iris/color_settings.h"
#include "iris/views/defaults.h"


namespace iris
{


template<typename Value>
class ColorSettingsView: public draw::CollapsibleNodeSettingsView
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    ColorSettingsView(
        wxWindow *parent,
        ColorControl<Value> control,
        std::optional<draw::NodeSettingsControl> nodeSettingsControl,
        const LayoutOptions &layoutOptions = LayoutOptions{})
        :
        draw::CollapsibleNodeSettingsView(parent, "Color", nodeSettingsControl)
    {
        auto panel = this->GetPanel();

        auto levelLow = wxpex::LabeledWidget(
            panel,
            "Low",
            new wxpex::ValueSlider(
                panel,
                control.range.low,
                control.range.low.value));

        auto levelHigh = wxpex::LabeledWidget(
            panel,
            "High",
            new wxpex::ValueSlider(
                panel,
                control.range.high,
                control.range.high.value));

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
