#pragma once


#include "iris/mask_settings.h"
#include <wxpex/collapsible.h>
#include <wxpex/labeled_widget.h>


namespace iris
{


class MaskSettingsView: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    MaskSettingsView(
        wxWindow *parent,
        MaskControl controls,
        const LayoutOptions &layoutOptions = LayoutOptions{});
};


} // end namespace iris
