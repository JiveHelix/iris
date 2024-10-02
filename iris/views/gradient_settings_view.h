#pragma once


#include <wxpex/collapsible.h>
#include <wxpex/labeled_widget.h>
#include <wxpex/slider.h>
#include <wxpex/radio_box.h>
#include <wxpex/check_box.h>
#include <wxpex/view.h>
#include <wxpex/button.h>

#include <draw/views/node_settings_view.h>
#include "iris/gradient_settings.h"


namespace iris
{


template<typename Value>
class GradientSettingsView: public draw::CollapsibleNodeSettingsView
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    GradientSettingsView(
        wxWindow *parent,
        GradientControl<Value> controls,
        std::optional<draw::NodeSettingsControl> nodeSettingsControl,
        const LayoutOptions &layoutOptions = LayoutOptions{})
        :
        draw::CollapsibleNodeSettingsView(
            parent,
            "Gradient",
            nodeSettingsControl)
    {
        using namespace wxpex;

        auto enable = LabeledWidget(
            this->GetPanel(),
            "Enable",
            new CheckBox(this->GetPanel(), "", controls.enable));

        auto scale = LabeledWidget(
            this->GetPanel(),
            "Scale",
            new ValueSlider(
                this->GetPanel(),
                controls.scale,
                controls.scale.value));

        auto size = LabeledWidget(
            this->GetPanel(),
            "Size",
            new RadioBox
            <
                iris::DerivativeSize::Control,
                iris::DerivativeSize::SizeToString
            >(this->GetPanel(), controls.size));

        auto maximum = LabeledWidget(
            this->GetPanel(),
            "Maximum",
            new View(this->GetPanel(), controls.maximum));

        auto threads = wxpex::LabeledWidget(
            this->GetPanel(),
            "Threads",
            new wxpex::Field(this->GetPanel(), controls.threads));

        auto percentile = wxpex::LabeledWidget(
            this->GetPanel(),
            "Percentile",
            new wxpex::Field(this->GetPanel(), controls.percentile));

        auto detect = new wxpex::Button(
            this->GetPanel(),
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
