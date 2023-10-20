#pragma once


#include <wxpex/collapsible.h>
#include <wxpex/labeled_widget.h>
#include "iris/mask_settings.h"
#include "iris/views/node_settings_view.h"


namespace iris
{


class MaskSettingsView: public NodeSettingsView
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    MaskSettingsView(
        wxWindow *parent,
        MaskControl controls,
        std::optional<NodeSettingsControl> nodeSettingsControl,
        const LayoutOptions &layoutOptions = LayoutOptions{});
};


} // end namespace iris
