#pragma once


#include <wxpex/labeled_widget.h>
#include <wxpex/collapsible.h>
#include <wxpex/slider.h>
#include <wxpex/field.h>
#include <wxpex/check_box.h>
#include <wxpex/view.h>

#include <draw/views/node_settings_view.h>
#include "iris/harris_settings.h"
#include "iris/views/defaults.h"


namespace iris
{


template<typename Control>
class HarrisSettingsView: public draw::CollapsibleNodeSettingsView
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    HarrisSettingsView(
        wxWindow *parent,
        Control controls,
        std::optional<draw::NodeSettingsControl> nodeSettingsControl,
        const LayoutOptions &layoutOptions = LayoutOptions{})
        :
        draw::CollapsibleNodeSettingsView(parent, "Harris", nodeSettingsControl)
    {
        using namespace wxpex;

        auto panel = this->GetPanel();

        auto enable = LabeledWidget(
            panel,
            "Enable",
            new CheckBox(panel, "", controls.enable));

        auto alpha = LabeledWidget(
            panel,
            "alpha",
            new ValueSlider(
                panel,
                controls.alpha,
                controls.alpha.value));

        auto sigma = LabeledWidget(
            panel,
            "sigma",
            new ValueSlider(
                panel,
                controls.sigma,
                controls.sigma.value));

        auto threshold = LabeledWidget(
            panel,
            "threshold",
            new ValueSlider(
                panel,
                controls.threshold,
                controls.threshold.value));

        auto suppress = LabeledWidget(
            panel,
            "suppress",
            new CheckBox(panel, "", controls.suppress));

        auto window = LabeledWidget(
            panel,
            "window",
            new ValueSlider(
                panel,
                controls.window,
                controls.window.value));

        auto threads = LabeledWidget(
            panel,
            "threads",
            new Field(
                panel,
                controls.threads));

        auto sizer = LayoutLabeled(
            layoutOptions,
            enable,
            alpha,
            sigma,
            threshold,
            suppress,
            window,
            threads);

        this->ConfigureTopSizer(std::move(sizer));
    }
};


} // end namespace iris
