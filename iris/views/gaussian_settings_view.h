#pragma once


#include <wxpex/collapsible.h>
#include <wxpex/labeled_widget.h>
#include <wxpex/check_box.h>
#include <wxpex/field.h>
#include <wxpex/slider.h>
#include <wxpex/view.h>

#include "iris/gaussian_settings.h"


namespace iris
{


template<typename Value>
class GaussianSettingsView: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    GaussianSettingsView(
        wxWindow *parent,
        GaussianControl<Value> controls,
        const LayoutOptions &layoutOptions)
        :
        wxpex::Collapsible(parent, "Gaussian Blur")
    {
        auto enable = wxpex::LabeledWidget(
            this->GetPane(),
            "Enable",
            new wxpex::CheckBox(this->GetPane(), "", controls.enable));

        auto sigma = wxpex::LabeledWidget(
            this->GetPane(),
            "sigma",
            new wxpex::ValueSlider(
                this->GetPane(),
                controls.sigma,
                controls.sigma.value));

        auto threshold = wxpex::LabeledWidget(
            this->GetPane(),
            "threshold",
            new wxpex::Field(this->GetPane(), controls.threshold));

        auto threads = wxpex::LabeledWidget(
            this->GetPane(),
            "Threads",
            new wxpex::Field(this->GetPane(), controls.threads));

        auto sizer = wxpex::LayoutLabeled(
            layoutOptions,
            enable,
            sigma,
            threshold,
            threads);

        this->ConfigureTopSizer(sizer.release());
    }
};


} // end namespace iris
