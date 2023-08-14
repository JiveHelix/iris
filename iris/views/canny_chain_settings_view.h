#pragma once


#include <wxpex/labeled_widget.h>
#include <wxpex/collapsible.h>
#include "iris/canny_chain_settings.h"


namespace iris
{


class CannyChainSettingsView: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    CannyChainSettingsView(
        wxWindow *parent,
        CannyChainControl controls,
        const LayoutOptions &layoutOptions = LayoutOptions{});
};


} // end namespace iris
