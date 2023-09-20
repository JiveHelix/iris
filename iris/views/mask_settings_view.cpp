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
    wxpex::Collapsible(parent, "Mask", borderStyle)
{
    auto panel = this->GetPanel();

    auto enable =
        new wxpex::CheckBox(panel, "Enable", controls.enable);

    auto showOutline =
        new wxpex::CheckBox(
            panel,
            "Show Outline",
            controls.showOutline);

    auto polygon =
        new draw::PolygonView(
            panel,
            "Polygon",
            controls.polygon,
            layoutOptions);

    auto feather =
        new GaussianSettingsView<double>(
            panel,
            "Feather",
            controls.feather,
            layoutOptions);

    auto sizer = wxpex::LayoutItems(
        wxpex::verticalItems,
        enable,
        showOutline,
        polygon,
        feather);

    this->ConfigureTopSizer(std::move(sizer));
}


} // end namespace iris
