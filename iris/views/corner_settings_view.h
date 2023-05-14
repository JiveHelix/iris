#pragma once


#include <wxpex/labeled_widget.h>
#include <wxpex/collapsible.h>

#include "iris/corner_settings.h"


namespace iris
{


class CornerSettingsView: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    CornerSettingsView(
        wxWindow *parent,
        CornerControl controls,
        const LayoutOptions &layoutOptions = LayoutOptions{});
};


} // end namespace iris
