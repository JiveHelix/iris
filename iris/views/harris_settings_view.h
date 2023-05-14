#pragma once


#include <wxpex/labeled_widget.h>
#include <wxpex/collapsible.h>
#include <wxpex/slider.h>
#include <wxpex/field.h>
#include <wxpex/check_box.h>
#include <wxpex/view.h>

#include "iris/harris_settings.h"


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

        auto enable = LabeledWidget(
            this->GetPane(),
            "Enable",
            new CheckBox(this->GetPane(), "", controls.enable));

        auto alpha = LabeledWidget(
            this->GetPane(),
            "alpha",
            new ValueSlider(
                this->GetPane(),
                controls.alpha,
                controls.alpha.value));

        auto sigma = LabeledWidget(
            this->GetPane(),
            "sigma",
            new ValueSlider(
                this->GetPane(),
                controls.sigma,
                controls.sigma.value));

        auto threshold = LabeledWidget(
            this->GetPane(),
            "threshold",
            new ValueSlider(
                this->GetPane(),
                controls.threshold,
                controls.threshold.value));

        auto suppress = LabeledWidget(
            this->GetPane(),
            "suppress",
            new CheckBox(this->GetPane(), "", controls.suppress));

        auto window = LabeledWidget(
            this->GetPane(),
            "window",
            new ValueSlider(
                this->GetPane(),
                controls.window,
                controls.window.value));

        auto threads = LabeledWidget(
            this->GetPane(),
            "threads",
            new Field(
                this->GetPane(),
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

        this->ConfigureTopSizer(sizer.release());
    }
};


} // end namespace iris
