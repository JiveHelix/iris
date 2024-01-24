#pragma once


#include <wxpex/collapsible.h>
#include <wxpex/labeled_widget.h>
#include <wxpex/slider.h>
#include <wxpex/button.h>
#include <wxpex/check_box.h>

#include <draw/views/node_settings_view.h>
#include "iris/level_settings.h"


namespace iris
{


template<typename Value>
class LevelSettingsView: public draw::NodeSettingsView
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    LevelSettingsView(
        wxWindow *parent,
        const std::string &name,
        LevelControl<Value> control,
        std::optional<draw::NodeSettingsControl> nodeSettingsControl,
        const LayoutOptions &layoutOptions = LayoutOptions{})
        :
        draw::NodeSettingsView(parent, name, nodeSettingsControl)
    {
        auto detect = new wxpex::Button(
            this->GetPanel(),
            "Detect",
            control.autoDetectSettings);

        auto detectMargin = wxpex::LabeledWidget(
            this->GetPanel(),
            "Detect Margin",
            new wxpex::ValueSlider(
                this->GetPanel(),
                control.detectMargin,
                control.detectMargin.value));

        auto enable = wxpex::LabeledWidget(
            this->GetPanel(),
            "enable",
            new wxpex::CheckBox(this->GetPanel(), "", control.enable));

        auto levelLow = wxpex::LabeledWidget(
            this->GetPanel(),
            "Low",
            new wxpex::ValueSlider(
                this->GetPanel(),
                control.range.low,
                control.range.low.value));

        auto levelHigh = wxpex::LabeledWidget(
            this->GetPanel(),
            "High",
            new wxpex::ValueSlider(
                this->GetPanel(),
                control.range.high,
                control.range.high.value));

        auto rangeSizer = LayoutLabeled(
            layoutOptions,
            detectMargin,
            enable,
            levelLow,
            levelHigh);

        auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
        sizer->Add(detect, 0, wxBOTTOM);
        sizer->Add(rangeSizer.release(), 1, wxEXPAND);
        this->ConfigureTopSizer(std::move(sizer));
    }
};


extern template class LevelSettingsView<int32_t>;


} // end namespace iris
