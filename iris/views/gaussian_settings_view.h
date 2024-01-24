#pragma once


#include <wxpex/collapsible.h>
#include <wxpex/labeled_widget.h>
#include <wxpex/check_box.h>
#include <wxpex/field.h>
#include <wxpex/slider.h>
#include <wxpex/view.h>

#include <draw/views/node_settings_view.h>
#include "iris/gaussian_settings.h"
#include "iris/views/defaults.h"


namespace iris
{


template<typename Value>
class GaussianSettingsView: public draw::NodeSettingsView
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    GaussianSettingsView(
        wxWindow *parent,
        const std::string &name,
        GaussianControl<Value> controls,
        std::optional<draw::NodeSettingsControl> nodeSettingsControl,
        const LayoutOptions &layoutOptions)
        :
        draw::NodeSettingsView(parent, name, nodeSettingsControl)
    {
        auto panel = this->GetPanel();

        auto enable = wxpex::LabeledWidget(
            panel,
            "Enable",
            new wxpex::CheckBox(panel, "", controls.enable));

        auto sigma = wxpex::LabeledWidget(
            panel,
            "sigma",
            new wxpex::ValueSlider(
                panel,
                controls.sigma,
                controls.sigma.value));

        auto threshold = wxpex::LabeledWidget(
            panel,
            "threshold",
            new wxpex::Field(panel, controls.threshold));

        auto threads = wxpex::LabeledWidget(
            panel,
            "Threads",
            new wxpex::Field(panel, controls.threads));

        auto sizer = wxpex::LayoutLabeled(
            layoutOptions,
            enable,
            sigma,
            threshold,
            threads);

        this->ConfigureTopSizer(std::move(sizer));
    }
};


extern template class GaussianSettingsView<int32_t>;


} // end namespace iris
