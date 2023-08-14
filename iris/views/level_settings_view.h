#pragma once


#include <wxpex/collapsible.h>
#include <wxpex/labeled_widget.h>
#include <wxpex/slider.h>
#include <wxpex/button.h>
#include <wxpex/check_box.h>

#include "iris/level_settings.h"


namespace iris
{


template<typename Value>
class LevelSettingsView: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    LevelSettingsView(
        wxWindow *parent,
        const std::string &name,
        LevelControl<Value> control,
        const LayoutOptions &layoutOptions = LayoutOptions{})
        :
        wxpex::Collapsible(parent, name)
    {
        auto detect = new wxpex::Button(
            this->GetPane(),
            "Detect",
            control.autoDetectSettings);

        auto detectMargin = wxpex::LabeledWidget(
            this->GetPane(),
            "Detect Margin",
            new wxpex::ValueSlider(
                this->GetPane(),
                control.detectMargin,
                control.detectMargin.value));

        auto enable = wxpex::LabeledWidget(
            this->GetPane(),
            "enable",
            new wxpex::CheckBox(this->GetPane(), "", control.enable));

        auto levelLow = wxpex::LabeledWidget(
            this->GetPane(),
            "Low",
            new wxpex::ValueSlider(
                this->GetPane(),
                control.range.low,
                control.range.low.value));

        auto levelHigh = wxpex::LabeledWidget(
            this->GetPane(),
            "High",
            new wxpex::ValueSlider(
                this->GetPane(),
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
