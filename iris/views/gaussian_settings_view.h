#pragma once


#include <wxpex/collapsible.h>
#include <wxpex/labeled_widget.h>
#include <wxpex/check_box.h>
#include <wxpex/field.h>
#include <wxpex/slider.h>
#include <wxpex/view.h>

#include "iris/gaussian_settings.h"
#include "iris/views/defaults.h"


namespace iris
{


template<typename Value>
class GaussianSettingsView: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    GaussianSettingsView(
        wxWindow *parent,
        const std::string &name,
        GaussianControl<Value> controls,
        const LayoutOptions &layoutOptions)
        :
        wxpex::Collapsible(parent, name)
    {
        auto pane = this->GetBorderPane(borderStyle);

        auto enable = wxpex::LabeledWidget(
            pane,
            "Enable",
            new wxpex::CheckBox(pane, "", controls.enable));

        auto sigma = wxpex::LabeledWidget(
            pane,
            "sigma",
            new wxpex::ValueSlider(
                pane,
                controls.sigma,
                controls.sigma.value));

        auto threshold = wxpex::LabeledWidget(
            pane,
            "threshold",
            new wxpex::Field(pane, controls.threshold));

        auto threads = wxpex::LabeledWidget(
            pane,
            "Threads",
            new wxpex::Field(pane, controls.threads));

        auto sizer = wxpex::LayoutLabeled(
            layoutOptions,
            enable,
            sigma,
            threshold,
            threads);

        this->ConfigureBorderPane(borderPixels, std::move(sizer));
    }
};


extern template class GaussianSettingsView<int32_t>;


} // end namespace iris
