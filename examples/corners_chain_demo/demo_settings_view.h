#pragma once


#include <wx/scrolwin.h>
#include "demo_settings.h"


class DemoSettingsView: public wxScrolled<wxPanel>
{
public:
    DemoSettingsView(
        wxWindow *parent,
        DemoControl control);
};
