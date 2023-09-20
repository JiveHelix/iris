#include "corners_chain_settings_view.h"

#include <wxpex/labeled_widget.h>
#include <wxpex/check_box.h>
#include <wxpex/indent_sizer.h>
#include <draw/views/points_shape_view.h>
#include "iris/views/gaussian_settings_view.h"
#include "iris/views/gradient_settings_view.h"
#include "iris/views/harris_settings_view.h"
#include "iris/views/corner_settings_view.h"
#include "iris/views/defaults.h"


namespace iris
{


CornersChainSettingsView::CornersChainSettingsView(
    wxWindow *parent,
    CornersChainControl controls,
    const LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, "Corners Chain", borderStyle)
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
            layoutOptions);

    auto gradient =
        new GradientSettingsView<int32_t>(
            panel,
            controls.gradient,
            layoutOptions);

    auto harris =
        new HarrisSettingsView(
            panel,
            controls.harris,
            layoutOptions);

    auto corner =
        new CornerSettingsView(
            panel,
            controls.corner,
            layoutOptions);

    auto pointsShape =
        new draw::PointsShapeView(
            panel,
            "Corners Shape",
            controls.shape,
            layoutOptions);

    auto sizer = wxpex::LayoutItems(
        wxpex::verticalItems,
        enable,
        gaussian,
        gradient,
        harris,
        corner,
        pointsShape);

    this->ConfigureTopSizer(std::move(sizer));
}


} // end namespace iris
