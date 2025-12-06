#pragma once


#include <wxpex/labeled_widget.h>
#include <wxpex/collapsible.h>
#include "iris/vertex_chain_settings.h"


namespace iris
{


class VertexChainSettingsView: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    VertexChainSettingsView(
        wxWindow *parent,
        const VertexChainControl &controls,
        const VertexChainNodeSettingsControl *nodeSettingsControl = nullptr,
        const LayoutOptions &layoutOptions = LayoutOptions{});
};


} // end namespace iris
