#pragma once


#include <draw/node_settings.h>
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
        std::optional<LinesChainNodeSettingsControl> nodeSettingsControl,
        const LayoutOptions &layoutOptions = LayoutOptions{});
};


} // end namespace iris
