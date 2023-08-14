#pragma once


#include <wxpex/collapsible.h>
#include <wxpex/labeled_widget.h>
#include <wxpex/slider.h>
#include <wxpex/radio_box.h>
#include <wxpex/check_box.h>
#include <wxpex/view.h>
#include <wxpex/button.h>

#include "iris/gradient_settings.h"


namespace iris
{


template<typename Value>
class GradientSettingsView: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    GradientSettingsView(
        wxWindow *parent,
        GradientControl<Value> controls,
        const LayoutOptions &layoutOptions = LayoutOptions{})
        :
        wxpex::Collapsible(parent, "Gradient")
    {
        using namespace wxpex;

        auto enable = LabeledWidget(
            this->GetPane(),
            "Enable",
            new CheckBox(this->GetPane(), "", controls.enable));

        auto scale = LabeledWidget(
            this->GetPane(),
            "Scale",
            new ValueSlider(
                this->GetPane(),
                controls.scale,
                controls.scale.value));

        auto size = LabeledWidget(
            this->GetPane(),
            "Size",
            new RadioBox
            <
                iris::DerivativeSize::Control,
                iris::DerivativeSize::SizeToString
            >(this->GetPane(), controls.size));

        auto maximum = LabeledWidget(
            this->GetPane(),
            "Maximum",
            new View(this->GetPane(), controls.maximum));

        auto threads = wxpex::LabeledWidget(
            this->GetPane(),
            "Threads",
            new wxpex::Field(this->GetPane(), controls.threads));

        auto percentile = wxpex::LabeledWidget(
            this->GetPane(),
            "Percentile",
            new wxpex::Field(this->GetPane(), controls.percentile));

        auto detect = new wxpex::Button(
            this->GetPane(),
            "Detect",
            controls.autoDetectSettings);

        auto sizer = LayoutLabeled(
            layoutOptions,
            enable,
            scale,
            size,
            maximum,
            threads,
            percentile);

        auto topSizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
        topSizer->Add(sizer.release(), 0, wxEXPAND | wxBOTTOM, 3);
        topSizer->Add(detect, 0, wxALIGN_CENTER);

        this->ConfigureTopSizer(std::move(topSizer));
    }
};


extern template class GradientSettingsView<int32_t>;


} // end namespace iris
