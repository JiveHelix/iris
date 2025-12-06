#pragma once


#include <wxpex/labeled_widget.h>
#include <wxpex/collapsible.h>
#include "iris/chess_settings.h"
#include <draw/views/node_settings_view.h>


namespace iris
{


class ChessSettingsView: public draw::CollapsibleNodeSettingsView
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    ChessSettingsView(
        wxWindow *parent,
        const ChessControl &controls,
        const draw::NodeSettingsControl *nodeSettingsControl,
        const LayoutOptions &layoutOptions = LayoutOptions{});
};


} // end namespace iris
