#pragma once


#include <wxpex/labeled_widget.h>
#include <wxpex/collapsible.h>

#include <draw/views/node_settings_view.h>
#include "iris/vertex_settings.h"


namespace iris
{


class VertexSettingsView: public draw::CollapsibleNodeSettingsView
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    VertexSettingsView(
        wxWindow *parent,
        const VertexControl &controls,
        const draw::NodeSettingsControl *nodeSettingsControl = nullptr,
        const LayoutOptions &layoutOptions = LayoutOptions{});
};


} // end namespace iris
