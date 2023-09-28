#include "chess_chain_settings_view.h"

#include <wxpex/labeled_widget.h>
#include <wxpex/check_box.h>
#include <wxpex/button.h>
#include <wxpex/indent_sizer.h>
#include "iris/views/mask_settings_view.h"
#include "iris/views/level_settings_view.h"
#include "iris/views/lines_chain_settings_view.h"
#include "iris/views/vertex_chain_settings_view.h"
#include "iris/views/chess_settings_view.h"
#include "iris/views/chess_shape_view.h"
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

    auto vertexChain =
        new VertexChainSettingsView(
            panel,
            controls.vertices,
            layoutOptions);

    auto linesChain =
        new LinesChainSettingsView(
            panel,
            controls.lines,
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
        vertexChain,
        linesChain,
        chess);

    sizer->Add(autoDetect, 0, wxALIGN_CENTER | wxTOP, 5);
    this->ConfigureTopSizer(std::move(sizer));
}


} // end namespace iris
