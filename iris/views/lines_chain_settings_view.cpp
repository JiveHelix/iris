#include "lines_chain_settings_view.h"

#include <wxpex/labeled_widget.h>
#include <wxpex/check_box.h>
#include <wxpex/indent_sizer.h>
#include <draw/views/lines_shape_view.h>
#include "iris/views/canny_chain_settings_view.h"
#include "iris/views/hough_settings_view.h"
#include "iris/views/defaults.h"


namespace iris
{


LinesChainSettingsView::LinesChainSettingsView(
    wxWindow *parent,
    LinesChainControl controls,
    const LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, "Lines Chain", borderStyle)
{
    using namespace wxpex;

    auto panel = this->GetPanel();

    auto enable =
        new wxpex::CheckBox(panel, "enable", controls.enable);

    auto cannyChain = new CannyChainSettingsView(
        panel,
        controls.cannyChain,
        layoutOptions);

    auto hough =
        new HoughSettingsView<double>(
            panel,
            controls.hough,
            layoutOptions);

    auto linesShape = new draw::LinesShapeView(
        panel,
        "Lines Shape",
        controls.shape,
        layoutOptions);

    auto sizer = LayoutItems(
        verticalItems,
        enable,
        cannyChain,
        hough,
        linesShape);

    this->ConfigureTopSizer(std::move(sizer));
}


} // end namespace iris
