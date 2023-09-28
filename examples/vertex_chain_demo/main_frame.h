#pragma once


#include "../common/user.h"
#include "demo_settings.h"


class MainFrame: public wxFrame
{
public:
    MainFrame(
        UserControl userControl,
        DemoControl control);

private:
    std::unique_ptr<wxpex::MenuShortcuts> shortcuts_;
};


