#pragma once


#include <wxpex/labeled_widget.h>
#include <wxpex/collapsible.h>

#include "iris/vertex_settings.h"


namespace iris
{


class VertexSettingsView: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    VertexSettingsView(
        wxWindow *parent,
        VertexControl controls,
        const LayoutOptions &layoutOptions = LayoutOptions{});
};


} // end namespace iris
