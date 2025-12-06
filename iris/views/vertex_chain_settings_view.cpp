#include "vertex_chain_settings_view.h"

#include <wxpex/labeled_widget.h>
#include <wxpex/layout_items.h>
#include <wxpex/check_box.h>
#include <wxpex/indent_sizer.h>
#include <draw/views/points_shape_view.h>
#include "iris/views/gaussian_settings_view.h"
#include "iris/views/gradient_settings_view.h"
#include "iris/views/harris_settings_view.h"
#include "iris/views/vertex_settings_view.h"
#include "iris/views/defaults.h"


namespace iris
{


VertexChainSettingsView::VertexChainSettingsView(
    wxWindow *parent,
    const VertexChainControl &controls,
    const VertexChainNodeSettingsControl *nodeSettings,
    const LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, "Vertex Chain", borderStyle)
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
                ? &nodeSettings->gaussian
                : nullptr,
            layoutOptions);

    auto gradient =
        new GradientSettingsView<int32_t>(
            panel,
            controls.gradient,
            (nodeSettings)
                ? &nodeSettings->gradient
                : nullptr,
            layoutOptions);

    auto harris =
        new HarrisSettingsView(
            panel,
            controls.harris,
            (nodeSettings)
                ? &nodeSettings->harris
                : nullptr,
            layoutOptions);

    auto vertexSettings =
        new VertexSettingsView(
            panel,
            controls.vertex,
            (nodeSettings)
                ? &nodeSettings->vertex
                : nullptr,
            layoutOptions);

    auto pointsShape =
        new draw::PointsShapeView(
            panel,
            "Vertex Shape",
            controls.shape,
            layoutOptions);

    auto sizer = wxpex::LayoutItems(
        wxpex::verticalItems,
        enable,
        gaussian,
        gradient,
        harris,
        vertexSettings,
        pointsShape);

    this->ConfigureSizer(std::move(sizer));
}


} // end namespace iris
