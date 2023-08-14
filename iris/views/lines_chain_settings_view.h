#pragma once


#include <wxpex/labeled_widget.h>
#include <wxpex/collapsible.h>
#include "iris/lines_chain_settings.h"


namespace iris
{


class LinesChainSettingsView: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    LinesChainSettingsView(
        wxWindow *parent,
        LinesChainControl controls,
        const LayoutOptions &layoutOptions = LayoutOptions{});
};


} // end namespace iris
