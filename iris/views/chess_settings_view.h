#pragma once


#include <wxpex/labeled_widget.h>
#include <wxpex/collapsible.h>
#include "iris/chess_settings.h"
#include <draw/views/node_settings_view.h>


namespace iris
{


class ChessSettingsView: public draw::NodeSettingsView
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    ChessSettingsView(
        wxWindow *parent,
        ChessControl controls,
        std::optional<draw::NodeSettingsControl> nodeSettingsControl,
        const LayoutOptions &layoutOptions = LayoutOptions{});
};


} // end namespace iris
