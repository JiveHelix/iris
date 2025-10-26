#pragma once


#include <wxpex/static_box.h>
#include <wxpex/labeled_widget.h>
#include <wxpex/layout_items.h>
#include <wxpex/slider.h>
#include <wxpex/button.h>

#include "iris/level_settings.h"


namespace iris
{


struct DetectTag {};


template<typename Value>
class DetectSettingsView: public wxpex::StaticBox
{
public:
    using Base = wxpex::StaticBox;

    DetectSettingsView(
        wxWindow *parent,
        const LevelControl<Value> &control)
        :
        Base(parent, "Auto Detect Settings")
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

        auto sizer = wxpex::LayoutItems(
            wxpex::verticalItems,
            detect,
            detectMargin.Layout());

        this->ConfigureSizer(std::move(sizer));
    }
};


} // end namespace iris
