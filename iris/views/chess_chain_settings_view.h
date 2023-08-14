#pragma once


#include <wxpex/labeled_widget.h>
#include <wxpex/collapsible.h>
#include "iris/chess_chain_settings.h"


namespace iris
{


class ChessChainSettingsView: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    ChessChainSettingsView(
        wxWindow *parent,
        ChessChainControl controls,
        const LayoutOptions &layoutOptions = LayoutOptions{});
};


} // end namespace iris
