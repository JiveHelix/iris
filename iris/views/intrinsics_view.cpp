#include "iris/views/intrinsics_view.h"

#include <wxpex/field.h>
#include <wxpex/file_field.h>
#include <wxpex/converter.h>


namespace iris
{


template<typename Control, size_t precision>
struct ValueField_
{
    using Converter = wxpex::PrecisionConverter<Control, precision>;
    using Type = wxpex::Field<Control, Converter>;
};


template<typename Control, size_t precision>
using ValueField = typename ValueField_<Control, precision>::Type;


IntrinsicsView::IntrinsicsView(
    wxWindow *parent,
    const std::string &name,
    tau::IntrinsicsControl<double> control,
    const LayoutOptions &layoutOptions)
    :
    wxpex::StaticBox(parent, name)
{
    using Control =
        decltype(tau::IntrinsicsControl<double>::pixelSize_um);

    auto pixelSize_um = wxpex::LabeledWidget(
        this,
        "Pixel Size (um)",
        new ValueField<Control, 1>(
            this,
            control.pixelSize_um));

    auto focalLengthX_mm = wxpex::LabeledWidget(
        this,
        "Focal Length X (mm)",
        new ValueField<Control, 4>(
            this,
            control.focalLengthX_mm));

    auto focalLengthY_mm = wxpex::LabeledWidget(
        this,
        "Focal Length Y (mm)",
        new ValueField<Control, 4>(
            this,
            control.focalLengthY_mm));

    auto principalX_pixels = wxpex::LabeledWidget(
        this,
        "Principal Point X (pixels)",
        new ValueField<Control, 4>(
            this,
            control.principalX_pixels));

    auto principalY_pixels = wxpex::LabeledWidget(
        this,
        "Principal Point Y (pixels)",
        new ValueField<Control, 4>(
            this,
            control.principalY_pixels));

    auto skew = wxpex::LabeledWidget(
        this,
        "Skew",
        new ValueField<Control, 5>(
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
