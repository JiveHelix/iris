#pragma once


#include <optional>
#include <wxpex/labeled_widget.h>
#include <wxpex/collapsible.h>
#include "iris/chess_chain_settings.h"
#include "iris/chess_chain_node_settings.h"


namespace iris
{


class ChessChainSettingsView: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    ChessChainSettingsView(
        wxWindow *parent,
        ChessChainControl control,
        std::optional<ChessChainNodeSettingsControl> nodeSettingsControl,
        const LayoutOptions &layoutOptions = LayoutOptions{});
};


} // end namespace iris
