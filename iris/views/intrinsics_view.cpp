#include "iris/views/intrinsics_view.h"
#include <wxpex/view.h>


namespace iris
{


IntrinsicsView::IntrinsicsView(
    wxWindow *parent,
    const std::string &name,
    const tau::IntrinsicsControl<double> &control,
    const LayoutOptions &layoutOptions)
    :
    wxpex::StaticBox(parent, name)
{
    using wxpex::CreateView;

    auto pixelSize_um = wxpex::LabeledWidget(
        this,
        "Pixel Size (um)",
        CreateView<1>(this, control.pixelSize_um));

    auto focalLengthX_mm = wxpex::LabeledWidget(
        this,
        "Focal Length X (mm)",
        CreateView<4>(this, control.focalLengthX_mm));

    auto focalLengthY_mm = wxpex::LabeledWidget(
        this,
        "Focal Length Y (mm)",
        CreateView<4>(this, control.focalLengthY_mm));

    auto principalX_pixels = wxpex::LabeledWidget(
        this,
        "Principal Point X (pixels)",
        CreateView<4>(this, control.principalX_pixels));

    auto principalY_pixels = wxpex::LabeledWidget(
        this,
        "Principal Point Y (pixels)",
        CreateView<4>(this, control.principalY_pixels));

    auto skew = wxpex::LabeledWidget(
        this,
        "Skew",
        CreateView<5>(this, control.skew));

    auto controlsSizer = LayoutLabeled(
        layoutOptions,
        pixelSize_um,
        focalLengthX_mm,
        focalLengthY_mm,
        principalX_pixels,
        principalY_pixels,
        skew);

    this->ConfigureSizer(std::move(controlsSizer));
}


} // end namespace iris
