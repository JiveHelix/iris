#pragma once


#include <wxpex/labeled_widget.h>
#include <wxpex/collapsible.h>
#include "iris/corners_chain_settings.h"


namespace iris
{


class CornersChainSettingsView: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    CornersChainSettingsView(
        wxWindow *parent,
        CornersChainControl controls,
        const LayoutOptions &layoutOptions = LayoutOptions{});
};


} // end namespace iris
