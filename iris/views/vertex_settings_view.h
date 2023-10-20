#pragma once


#include <wxpex/labeled_widget.h>
#include <wxpex/collapsible.h>

#include "iris/vertex_settings.h"
#include "iris/views/node_settings_view.h"


namespace iris
{


class VertexSettingsView: public NodeSettingsView
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    VertexSettingsView(
        wxWindow *parent,
        VertexControl controls,
        std::optional<NodeSettingsControl> nodeSettingsControl,
        const LayoutOptions &layoutOptions = LayoutOptions{});
};


} // end namespace iris
