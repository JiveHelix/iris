#include "chess_chain_settings_view.h"

#include <wxpex/labeled_widget.h>
#include <wxpex/check_box.h>
#include <wxpex/button.h>
#include <wxpex/indent_sizer.h>
#include "iris/views/mask_settings_view.h"
#include "iris/views/level_settings_view.h"
#include "iris/views/lines_chain_settings_view.h"
#include "iris/views/corners_chain_settings_view.h"
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
    wxpex::Collapsible(parent, "Chess Chain")
{
    using namespace wxpex;

    auto pane = this->GetBorderPane(borderStyle);

    auto enable =
        new wxpex::CheckBox(pane, "enable", controls.enable);

    auto mask =
        new MaskSettingsView(
            pane,
            controls.mask,
            layoutOptions);

    auto level =
        new LevelSettingsView<int32_t>(
            pane,
            "Level",
            controls.level,
            layoutOptions);

    auto cornersChain =
        new CornersChainSettingsView(
            pane,
            controls.corners,
            layoutOptions);

    auto linesChain =
        new LinesChainSettingsView(
            pane,
            controls.lines,
            layoutOptions);

    auto chess =
        new ChessSettingsView(
            pane,
            controls.chess,
            layoutOptions);

    auto autoDetect =
        new wxpex::Button(pane, "Auto", controls.autoDetectSettings);

    auto sizer = wxpex::LayoutItems(
        verticalItems,
        enable,
        mask,
        level,
        cornersChain,
        linesChain,
        chess);

    sizer->Add(autoDetect, 0, wxALIGN_CENTER | wxTOP, 5);
    this->ConfigureBorderPane(5, std::move(sizer));
}


} // end namespace iris
