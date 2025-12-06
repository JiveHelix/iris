#include "chess_chain_settings_view.h"

#include <wxpex/labeled_widget.h>
#include <wxpex/layout_items.h>
#include <wxpex/check_box.h>
#include <wxpex/button.h>
#include <wxpex/indent_sizer.h>
#include <draw/node_settings.h>
#include <draw/views/lines_shape_view.h>
#include <draw/views/points_shape_view.h>
#include "iris/views/mask_settings_view.h"
#include "iris/views/level_settings_view.h"
#include "iris/views/gaussian_settings_view.h"
#include "iris/views/gradient_settings_view.h"
#include "iris/views/canny_settings_view.h"
#include "iris/views/hough_settings_view.h"
#include "iris/views/harris_settings_view.h"
#include "iris/views/vertex_settings_view.h"
#include "iris/views/chess_settings_view.h"
#include "iris/views/defaults.h"


namespace iris
{


ChessChainSettingsView::ChessChainSettingsView(
    wxWindow *parent,
    const ChessChainControl &control,
    const ChessChainNodeSettingsControl *nodeSettings,
    const LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, "Chess Chain", borderStyle)
{
    using namespace wxpex;

    auto panel = this->GetPanel();

    auto enable =
        new wxpex::CheckBox(panel, "enable", control.enable);

    auto mask =
        new MaskSettingsView(
            panel,
            control.mask,
            (nodeSettings) ? &nodeSettings->mask : nullptr,
            layoutOptions);

    auto level =
        new LevelSettingsView<int32_t>(
            panel,
            "Level",
            control.level,
            (nodeSettings) ? &nodeSettings->level : nullptr,
            layoutOptions);

    auto gaussian =
        new GaussianSettingsView<int32_t>(
            panel,
            "Gaussian Blur",
            control.gaussian,
            (nodeSettings) ? &nodeSettings->gaussian : nullptr,
            layoutOptions);

    auto gradient =
        new GradientSettingsView<int32_t>(
            panel,
            control.gradient,
            (nodeSettings)
                ? &nodeSettings->gradient
                : nullptr,
            layoutOptions);

    auto canny =
        new CannySettingsView<double>(
            panel,
            control.canny,
            (nodeSettings) ? &nodeSettings->canny : nullptr,
            layoutOptions);

    // Line detection
    auto hough =
        new HoughSettingsView<double>(
            panel,
            control.hough,
            (nodeSettings) ? &nodeSettings->hough : nullptr,
            layoutOptions);

    auto linesShape = new draw::LinesShapeView(
        panel,
        "Lines Shape",
        control.linesShape,
        layoutOptions);

    // Vertex detection
    auto harris =
        new HarrisSettingsView(
            panel,
            control.harris,
            (nodeSettings)
                ? &nodeSettings->harris
                : nullptr,
            layoutOptions);

    auto verticesSettings =
        new VertexSettingsView(
            panel,
            control.vertices,
            (nodeSettings)
                ? &nodeSettings->vertices
                : nullptr,
            layoutOptions);

    auto verticesShape =
        new draw::PointsShapeView(
            panel,
            "Vertex Shape",
            control.verticesShape,
            layoutOptions);

    auto chess =
        new ChessSettingsView(
            panel,
            control.chess,
            (nodeSettings)
                ? &nodeSettings->chess
                : nullptr,
            layoutOptions);

    auto autoDetect =
        new wxpex::Button(panel, "Auto", control.autoDetectSettings);

    auto sizer = wxpex::LayoutItems(
        verticalItems,
        enable,
        mask,
        level,
        gaussian,
        gradient,
        canny,
        hough,
        linesShape,
        harris,
        verticesSettings,
        verticesShape,
        chess);

    sizer->Add(autoDetect, 0, wxALIGN_CENTER | wxTOP, 5);
    this->ConfigureSizer(std::move(sizer));
}


} // end namespace iris
