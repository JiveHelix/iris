#include "iris/views/intrinsics_view.h"

#include <wxpex/field.h>
#include <wxpex/file_field.h>


namespace iris
{


IntrinsicsView::IntrinsicsView(
    wxWindow *parent,
    const std::string &name,
    tau::IntrinsicsControl<double> control,
    const LayoutOptions &layoutOptions)
    :
    wxpex::StaticBox(parent, name)
{
    using namespace wxpex;

    auto pixelSize_um = LabeledWidget(
        this,
        "Pixel Size (um)",
        new Field(
            this,
            control.pixelSize_um));

    auto focalLengthX_mm = LabeledWidget(
        this,
        "Focal Length X (mm)",
        new Field(
            this,
            control.focalLengthX_mm));

    auto focalLengthY_mm = LabeledWidget(
        this,
        "Focal Length Y (mm)",
        new Field(
            this,
            control.focalLengthY_mm));

    auto principalX_pixels = LabeledWidget(
        this,
        "Principal Point X (pixels)",
        new Field(
            this,
            control.principalX_pixels));

    auto principalY_pixels = LabeledWidget(
        this,
        "Principal Point Y (pixels)",
        new Field(
            this,
            control.principalY_pixels));

    auto skew = LabeledWidget(
        this,
        "Skew",
        new Field(
            this,
            control.skew));

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
