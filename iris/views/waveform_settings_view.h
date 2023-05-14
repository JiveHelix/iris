#pragma once


#include <wxpex/labeled_widget.h>
#include <wxpex/collapsible.h>

#include "iris/waveform_settings.h"


namespace iris
{


class WaveformSettingsView: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    WaveformSettingsView(
        wxWindow *parent,
        WaveformControl control,
        const LayoutOptions &layoutOptions = LayoutOptions{});
};


} // end namespace iris
