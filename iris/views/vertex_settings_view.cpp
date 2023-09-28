#include "iris/views/vertex_settings_view.h"

#include <wxpex/labeled_widget.h>
#include <wxpex/slider.h>
#include <wxpex/check_box.h>
#include <wxpex/view.h>
#include <wxpex/radio_box.h>

#include "iris/views/defaults.h"


namespace iris
{


VertexSettingsView::VertexSettingsView(
    wxWindow *parent,
    VertexControl controls,
    const LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, "Vertex", borderStyle)
{
    using namespace wxpex;

    auto panel = this->GetPanel();

    auto enable = LabeledWidget(
        panel,
        "enable",
        new CheckBox(
            panel,
            "",
            controls.enable));
    
    auto window = LabeledWidget(
        panel,
        "window",
        new ValueSlider(
            panel,
            controls.window,
            controls.window.value));

    auto count = LabeledWidget(
        panel,
        "Count",
        new RadioBox(panel, controls.count));

    auto threads = wxpex::LabeledWidget(
        panel,
        "Threads",
        new wxpex::Field(panel, controls.threads));

    auto sizer = LayoutLabeled(
        layoutOptions,
        enable,
        window,
        count,
        threads);

    this->ConfigureTopSizer(std::move(sizer));
}


} // end namespace iris
