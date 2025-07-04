#include "demo_settings_view.h"

#include <wxpex/layout_items.h>
#include <draw/color_map_settings.h>
#include <draw/views/color_map_settings_view.h>
#include <iris/views/chess_chain_settings_view.h>
#include <iris/views/chess_shape_view.h>


DemoSettingsView::DemoSettingsView(
    wxWindow *parent,
    DemoControl control,
    iris::ChessChainNodeSettingsControl nodeSettings)
    :
    wxpex::Scrolled(parent)
{
    wxpex::LayoutOptions layoutOptions{};
    layoutOptions.labelFlags = wxALIGN_RIGHT;

    auto chess = new iris::ChessChainSettingsView(
        this,
        control.chess,
        nodeSettings,
        layoutOptions);

    chess->Expand();

    auto chessShape = new iris::ChessShapeView(
        this,
        control.chessShape,
        layoutOptions);

    auto color = new draw::ColorMapSettingsView<int32_t>(
        this,
        control.color,
        {},
        layoutOptions);

    auto sizer = wxpex::LayoutItems(
        wxpex::verticalItems,
        chess,
        chessShape,
        color);

    this->ConfigureSizer(
        wxpex::verticalScrolled.MinimumHeight(600),
        std::move(sizer));
};
