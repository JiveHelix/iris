#pragma once


#include <wxpex/labeled_widget.h>
#include <wxpex/collapsible.h>
#include <wxpex/slider.h>
#include <wxpex/field.h>
#include <wxpex/check_box.h>
#include <wxpex/view.h>

#include "iris/harris_settings.h"
#include "iris/views/defaults.h"


namespace iris
{


template<typename Control>
class HarrisSettingsView: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    HarrisSettingsView(
        wxWindow *parent,
        Control controls,
        const LayoutOptions &layoutOptions = LayoutOptions{})
        :
        wxpex::Collapsible(parent, "Harris")
    {
        using namespace wxpex;

        auto pane = this->GetBorderPane(borderStyle);

        auto enable = LabeledWidget(
            pane,
            "Enable",
            new CheckBox(pane, "", controls.enable));

        auto alpha = LabeledWidget(
            pane,
            "alpha",
            new ValueSlider(
                pane,
                controls.alpha,
                controls.alpha.value));

        auto sigma = LabeledWidget(
            pane,
            "sigma",
            new ValueSlider(
                pane,
                controls.sigma,
                controls.sigma.value));

        auto threshold = LabeledWidget(
            pane,
            "threshold",
            new ValueSlider(
                pane,
                controls.threshold,
                controls.threshold.value));

        auto suppress = LabeledWidget(
            pane,
            "suppress",
            new CheckBox(pane, "", controls.suppress));

        auto window = LabeledWidget(
            pane,
            "window",
            new ValueSlider(
                pane,
                controls.window,
                controls.window.value));

        auto threads = LabeledWidget(
            pane,
            "threads",
            new Field(
                pane,
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

        this->ConfigureBorderPane(borderPixels, std::move(sizer));
    }
};


} // end namespace iris
