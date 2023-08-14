#include "iris/views/mask_settings_view.h"

#include <wxpex/check_box.h>
#include <wxpex/field.h>
#include <wxpex/slider.h>
#include <wxpex/view.h>
#include <wxpex/border_sizer.h>
#include <draw/views/polygon_view.h>

#include "iris/views/gaussian_settings_view.h"
#include "iris/views/defaults.h"


namespace iris
{


MaskSettingsView::MaskSettingsView(
    wxWindow *parent,
    MaskControl controls,
    const LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, "Mask")
{
    auto pane = this->GetBorderPane(borderStyle);

    auto enable =
        new wxpex::CheckBox(pane, "Enable", controls.enable);

    auto showOutline =
        new wxpex::CheckBox(
            pane,
            "Show Outline",
            controls.showOutline);

    auto polygon =
        new draw::PolygonView(
            pane,
            "Polygon",
            controls.polygon,
            layoutOptions);

    auto feather =
        new GaussianSettingsView<double>(
            pane,
            "Feather",
            controls.feather,
            layoutOptions);

    auto sizer = wxpex::LayoutItems(
        wxpex::verticalItems,
        enable,
        showOutline,
        polygon,
        feather);

    this->ConfigureBorderPane(5, std::move(sizer));
}


} // end namespace iris
