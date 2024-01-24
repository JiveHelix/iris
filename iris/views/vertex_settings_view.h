#pragma once


#include <wxpex/labeled_widget.h>
#include <wxpex/collapsible.h>

#include <draw/views/node_settings_view.h>
#include "iris/vertex_settings.h"


namespace iris
{


class VertexSettingsView: public draw::NodeSettingsView
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    VertexSettingsView(
        wxWindow *parent,
        VertexControl controls,
        std::optional<draw::NodeSettingsControl> nodeSettingsControl,
        const LayoutOptions &layoutOptions = LayoutOptions{});
};


} // end namespace iris
