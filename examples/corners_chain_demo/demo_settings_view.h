#pragma once


#include <wxpex/ignores.h>

WXSHIM_PUSH_IGNORES
#include <wx/scrolwin.h>
WXSHIM_POP_IGNORES

#include "demo_settings.h"


class DemoSettingsView: public wxScrolled<wxPanel>
{
public:
    DemoSettingsView(
        wxWindow *parent,
        DemoControl control);
};
