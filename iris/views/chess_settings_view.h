#pragma once


#include <wxpex/labeled_widget.h>
#include <wxpex/collapsible.h>
#include "iris/chess_settings.h"
#include "iris/views/node_settings_view.h"


namespace iris
{


class ChessSettingsView: public NodeSettingsView
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    ChessSettingsView(
        wxWindow *parent,
        ChessControl controls,
        std::optional<NodeSettingsControl> nodeSettingsControl,
        const LayoutOptions &layoutOptions = LayoutOptions{});
};


} // end namespace iris
