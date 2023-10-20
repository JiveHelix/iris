#pragma once


#include <wxpex/collapsible.h>
#include <wxpex/labeled_widget.h>
#include <wxpex/slider.h>
#include <wxpex/radio_box.h>
#include <wxpex/check_box.h>
#include <wxpex/view.h>

#include "iris/canny_settings.h"
#include "iris/views/defaults.h"
#include "iris/views/node_settings_view.h"


namespace iris
{


template<typename Value>
class CannySettingsView: public NodeSettingsView
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    CannySettingsView(
        wxWindow *parent,
        CannyControl<Value> controls,
        std::optional<NodeSettingsControl> nodeSettingsControl,
        const LayoutOptions &layoutOptions = LayoutOptions{})
        :
        NodeSettingsView(parent, "Canny", nodeSettingsControl)
    {
        using namespace wxpex;

        auto panel = this->GetPanel();

        auto enable = wxpex::LabeledWidget(
            panel,
            "Enable",
            new wxpex::CheckBox(panel, "", controls.enable));

        auto high = LabeledWidget(
            panel,
            "High",
            new ValueSlider(
                panel,
                controls.range.high,
                controls.range.high.value));

        auto low = LabeledWidget(
            panel,
            "Low",
            new ValueSlider(
                panel,
                controls.range.low,
                controls.range.low.value));

        auto depth = wxpex::LabeledWidget(
            panel,
            "Depth",
            new wxpex::Field(panel, controls.depth));

        auto threads = wxpex::LabeledWidget(
            panel,
            "Threads",
            new wxpex::Field(panel, controls.threads));

        auto sizer = LayoutLabeled(
            layoutOptions,
            enable,
            high,
            low,
            threads,
            depth);

        this->ConfigureTopSizer(std::move(sizer));
    }
};


extern template class CannySettingsView<double>;


} // end namespace iris
