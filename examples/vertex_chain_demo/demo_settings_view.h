#pragma once


#include <wxpex/scrolled.h>
#include "demo_settings.h"


class DemoSettingsView: public wxpex::Scrolled
{
public:
    DemoSettingsView(
        wxWindow *parent,
        DemoControl control);
};
