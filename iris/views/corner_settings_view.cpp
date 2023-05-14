#include "iris/views/corner_settings_view.h"

#include <wxpex/labeled_widget.h>
#include <wxpex/slider.h>
#include <wxpex/radio_box.h>
#include <wxpex/view.h>


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

    auto window = LabeledWidget(
        this->GetPane(),
        "window",
        new ValueSlider(
            this->GetPane(),
            controls.window,
            controls.window.value));

    auto count = LabeledWidget(
        this->GetPane(),
        "count",
        new ValueSlider(
            this->GetPane(),
            controls.count,
            controls.count.value));

    auto sizer = LayoutLabeled(
        layoutOptions,
        window,
        count);

    this->ConfigureTopSizer(sizer.release());
}


} // end namespace iris
