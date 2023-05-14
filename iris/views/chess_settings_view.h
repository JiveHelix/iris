#pragma once


#include <wxpex/labeled_widget.h>
#include <wxpex/collapsible.h>
#include "iris/chess_settings.h"


namespace iris
{


class ChessSettingsView: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    ChessSettingsView(
        wxWindow *parent,
        ChessControl controls,
        const LayoutOptions &layoutOptions = LayoutOptions{});
};


} // end namespace iris
