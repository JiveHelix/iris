#include "iris/views/corner_settings_view.h"

#include <wxpex/labeled_widget.h>
#include <wxpex/slider.h>
#include <wxpex/check_box.h>
#include <wxpex/view.h>
#include <wxpex/radio_box.h>

#include "iris/views/defaults.h"


namespace iris
{


CornerSettingsView::CornerSettingsView(
    wxWindow *parent,
    CornerControl controls,
    const LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, "Corner")
{
    using namespace wxpex;

    auto pane = this->GetBorderPane(borderStyle);

    auto enable = LabeledWidget(
        pane,
        "enable",
        new CheckBox(
            pane,
            "",
            controls.enable));
    
    auto window = LabeledWidget(
        pane,
        "window",
        new ValueSlider(
            pane,
            controls.window,
            controls.window.value));

    auto count = LabeledWidget(
        pane,
        "Count",
        new RadioBox(pane, controls.count));

    auto threads = wxpex::LabeledWidget(
        pane,
        "Threads",
        new wxpex::Field(pane, controls.threads));

    auto sizer = LayoutLabeled(
        layoutOptions,
        enable,
        window,
        count,
        threads);

    this->ConfigureBorderPane(borderPixels, std::move(sizer));
}


} // end namespace iris
