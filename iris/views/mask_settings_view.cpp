#include "iris/views/mask_settings_view.h"

#include <wxpex/layout_items.h>
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
    const MaskControl &controls,
    const draw::NodeSettingsControl *nodeSettingsControl,
    const LayoutOptions &layoutOptions)
    :
    draw::CollapsibleNodeSettingsView(parent, "Mask", nodeSettingsControl)
{
    auto panel = this->GetPanel();

    auto enable =
        new wxpex::CheckBox(panel, "Enable", controls.enable);

    auto showOutline =
        new wxpex::CheckBox(
            panel,
            "Show Outline",
            controls.showOutline);

    auto feather =
        new GaussianSettingsView<double>(
            panel,
            "Feather",
            controls.feather,
            nullptr,
            layoutOptions);

    auto sizer = wxpex::LayoutItems(
        wxpex::verticalItems,
        enable,
        showOutline,
        feather);

    this->ConfigureSizer(std::move(sizer));
}


} // end namespace iris
