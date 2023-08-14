#include "canny_chain_settings_view.h"

#include <wxpex/labeled_widget.h>
#include <wxpex/check_box.h>
#include <wxpex/indent_sizer.h>
#include "iris/views/gaussian_settings_view.h"
#include "iris/views/gradient_settings_view.h"
#include "iris/views/canny_settings_view.h"
#include "iris/views/defaults.h"


namespace iris
{


CannyChainSettingsView::CannyChainSettingsView(
    wxWindow *parent,
    CannyChainControl controls,
    const LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, "Canny Chain")
{
    using namespace wxpex;

    auto pane = this->GetBorderPane(borderStyle);

    auto enable =
        new wxpex::CheckBox(pane, "enable", controls.enable);

    auto gaussian =
        new GaussianSettingsView<int32_t>(
            pane,
            "Gaussian Blur",
            controls.gaussian,
            layoutOptions);

    auto gradient =
        new GradientSettingsView<int32_t>(
            pane,
            controls.gradient,
            layoutOptions);

    auto canny =
        new CannySettingsView<double>(
            pane,
            controls.canny,
            layoutOptions);

    auto sizer = wxpex::LayoutItems(
        wxpex::verticalItems,
        enable,
        gaussian,
        gradient,
        canny);

    this->ConfigureBorderPane(5, std::move(sizer));
}


} // end namespace iris
