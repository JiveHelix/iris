#pragma once

#include <optional>
#include <wxpex/collapsible.h>
#include <pex/endpoint.h>

#include "iris/node_settings.h"



namespace iris
{


class NodeSettingsView: public wxpex::Collapsible
{
public:
    NodeSettingsView(
        wxWindow *parent,
        const std::string &nodeName,
        std::optional<NodeSettingsControl> control);

private:
    void OnHighlight_(bool isHighlighted);

    void OnLeftDown_(wxMouseEvent &event);

    using Endpoint =
        pex::Endpoint
        <
            NodeSettingsView,
            decltype(NodeSettingsControl::highlight)
        >;

    std::optional<NodeSettingsControl> control_;
    Endpoint highlightEndpoint_;
};


} // end namespace iris
