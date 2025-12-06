#pragma once


#include <wxpex/collapsible.h>
#include <wxpex/labeled_widget.h>

#include <draw/views/node_settings_view.h>
#include "iris/mask_settings.h"


namespace iris
{


class MaskSettingsView: public draw::CollapsibleNodeSettingsView
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    MaskSettingsView(
        wxWindow *parent,
        const MaskControl &controls,
        const draw::NodeSettingsControl * nodeSettingsControl = nullptr,
        const LayoutOptions &layoutOptions = LayoutOptions{});
};


} // end namespace iris
