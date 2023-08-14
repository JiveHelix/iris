#pragma once


#include <wxpex/labeled_widget.h>
#include <wxpex/collapsible.h>
#include "iris/homography_settings.h"


namespace iris
{


class HomographySettingsView: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    HomographySettingsView(
        wxWindow *parent,
        HomographyControl control,
        const LayoutOptions &layoutOptions = LayoutOptions{});
};


} // end namespace iris
