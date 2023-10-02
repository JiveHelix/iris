#include "chess_chain_settings_view.h"

#include <wxpex/labeled_widget.h>
#include <wxpex/check_box.h>
#include <wxpex/button.h>
#include <wxpex/indent_sizer.h>
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
    ChessChainControl controls,
    const LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, "Chess Chain", borderStyle)
{
    using namespace wxpex;

    auto panel = this->GetPanel();

    auto enable =
        new wxpex::CheckBox(panel, "enable", controls.enable);

    auto mask =
        new MaskSettingsView(
            panel,
            controls.mask,
            layoutOptions);

    auto level =
        new LevelSettingsView<int32_t>(
            panel,
            "Level",
            controls.level,
            layoutOptions);

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

    auto canny =
        new CannySettingsView<double>(
            panel,
            controls.canny,
            layoutOptions);

    // Line detection
    auto hough =
        new HoughSettingsView<double>(
            panel,
            controls.hough,
            layoutOptions);

    auto linesShape = new draw::LinesShapeView(
        panel,
        "Lines Shape",
        controls.linesShape,
        layoutOptions);

    // Vertex detection
    auto harris =
        new HarrisSettingsView(
            panel,
            controls.harris,
            layoutOptions);

    auto verticesSettings =
        new VertexSettingsView(
            panel,
            controls.vertices,
            layoutOptions);

    auto verticesShape =
        new draw::PointsShapeView(
            panel,
            "Vertex Shape",
            controls.verticesShape,
            layoutOptions);

    auto chess =
        new ChessSettingsView(
            panel,
            controls.chess,
            layoutOptions);

    auto autoDetect =
        new wxpex::Button(panel, "Auto", controls.autoDetectSettings);

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
    this->ConfigureTopSizer(std::move(sizer));
}


} // end namespace iris
