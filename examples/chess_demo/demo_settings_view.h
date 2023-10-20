#pragma once


#include <wxpex/scrolled.h>
#include "demo_settings.h"
#include <iris/chess_chain_settings.h>
#include <iris/chess_chain_node_settings.h>


class DemoSettingsView: public wxpex::Scrolled
{
public:
    DemoSettingsView(
        wxWindow *parent,
        DemoControl control,
        iris::ChessChainNodeSettingsControl nodeSettings);
};
