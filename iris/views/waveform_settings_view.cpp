#include "waveform_settings_view.h"

#include <wxpex/labeled_widget.h>
#include <wxpex/slider.h>
#include <wxpex/check_box.h>


namespace iris
{


WaveformSettingsView::WaveformSettingsView(
    wxWindow *parent,
    WaveformControl control,
    const LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, "Waveform")
{
    auto enable = wxpex::LabeledWidget(
        this->GetPane(),
        "Enable",
        new wxpex::CheckBox(
            this->GetPane(),
            "",
            control.enable));

    auto levelCount = wxpex::LabeledWidget(
        this->GetPane(),
        "Level count",
        new wxpex::ValueSlider(
            this->GetPane(),
            control.levelCount,
            control.levelCount.value));

    auto columnCount = wxpex::LabeledWidget(
        this->GetPane(),
        "Column count",
        new wxpex::ValueSlider(
            this->GetPane(),
            control.columnCount,
            control.columnCount.value));

    auto verticalScale = wxpex::LabeledWidget(
        this->GetPane(),
        "Vertical Scale",
        new wxpex::ValueSlider(
            this->GetPane(),
            control.verticalScale,
            control.verticalScale.value));

    auto brightnessLow = wxpex::LabeledWidget(
        this->GetPane(),
        "Brightness low",
        new wxpex::ValueSlider(
            this->GetPane(),
            control.brightness.low,
            control.brightness.low.value));

    auto brightnessHigh = wxpex::LabeledWidget(
        this->GetPane(),
        "Brightness high",
        new wxpex::ValueSlider(
            this->GetPane(),
            control.brightness.high,
            control.brightness.high.value));

    auto brightnessCount = wxpex::LabeledWidget(
        this->GetPane(),
        "Brightness count",
        new wxpex::ValueSlider(
            this->GetPane(),
            control.brightnessCount,
            control.brightnessCount.value));

    auto sizer = LayoutLabeled(
        layoutOptions,
        enable,
        levelCount,
        columnCount,
        verticalScale,
        brightnessLow,
        brightnessHigh,
        brightnessCount);

    this->ConfigureTopSizer(sizer.release());
}


} // end namespace iris
