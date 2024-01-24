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
    std::optional<CannyChainNodeSettingsControl> nodeSettings,
    const LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, "Canny Chain", borderStyle)
{
    using namespace wxpex;

    auto panel = this->GetPanel();

    auto enable =
        new wxpex::CheckBox(panel, "enable", controls.enable);

    auto gaussian =
        new GaussianSettingsView<int32_t>(
            panel,
            "Gaussian Blur",
            controls.gaussian,
            (nodeSettings)
                ? nodeSettings->gaussian
                : draw::OptionalNodeSettings{},
            layoutOptions);

    auto gradient =
        new GradientSettingsView<int32_t>(
            panel,
            controls.gradient,
            (nodeSettings)
                ? nodeSettings->gradient
                : draw::OptionalNodeSettings{},
            layoutOptions);

    auto canny =
        new CannySettingsView<double>(
            panel,
            controls.canny,
            (nodeSettings)
                ? nodeSettings->canny
                : draw::OptionalNodeSettings{},
            layoutOptions);

    auto sizer = wxpex::LayoutItems(
        wxpex::verticalItems,
        enable,
        gaussian,
        gradient,
        canny);

    this->ConfigureTopSizer(std::move(sizer));
}


} // end namespace iris
