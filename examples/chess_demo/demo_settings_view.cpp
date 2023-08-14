#include "demo_settings_view.h"

#include <iris/views/chess_chain_settings_view.h>
#include <iris/views/chess_shape_view.h>
#include <iris/views/color_settings_view.h>


DemoSettingsView::DemoSettingsView(
    wxWindow *parent,
    DemoControl control)
    :
    wxpex::Scrolled(parent)
{
    wxpex::LayoutOptions layoutOptions{};
    layoutOptions.labelFlags = wxALIGN_RIGHT;

    auto chess = new iris::ChessChainSettingsView(
        this,
        control.chess,
        layoutOptions);

    chess->Expand();

    auto chessShape = new iris::ChessShapeView(
        this,
        control.chessShape,
        layoutOptions);

    auto color = new iris::ColorSettingsView<int32_t>(
        this,
        control.color,
        layoutOptions);

    auto sizer = wxpex::LayoutItems(
        wxpex::verticalItems,
        chess,
        chessShape,
        color);

    this->ConfigureTopSizer(
        wxpex::verticalScrolled.MinimumHeight(600),
        std::move(sizer));
};
