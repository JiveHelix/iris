#pragma once


#include "../common/user.h"
#include "demo_settings.h"


class DemoControls: public wxPanel
{
public:
    DemoControls(
        wxWindow *parent,
        UserControl userControl,
        DemoControl control,
        iris::ChessChainNodeSettingsControl nodeSettings);

private:
    std::unique_ptr<wxpex::MenuShortcuts> shortcuts_;
};


