#include "homography_settings_view.h"

#include <wxpex/labeled_widget.h>
#include <wxpex/slider.h>
#include <wxpex/field.h>
#include <wxpex/check_box.h>
#include "iris/views/size_view.h"
#include "iris/views/defaults.h"


namespace iris
{


HomographySettingsView::HomographySettingsView(
    wxWindow *parent,
    HomographyControl control,
    const LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, "Homography", borderStyle)
{
    using namespace wxpex;
    using SensorSizeView = SizeView<double, 0>;

    using PixelSizeControl = decltype(HomographyControl::pixelSize_microns);
    using Converter = wxpex::PrecisionConverter<PixelSizeControl, 1>;
    using ValueField = wxpex::Field<PixelSizeControl, Converter>;

    auto panel = this->GetPanel();

    auto sensorSize = wxpex::LabeledWidget(
        panel,
        "Sensor Size (pixels)",
        new SensorSizeView(
            panel,
            control.sensorSize_pixels));

    auto pixelSize = wxpex::LabeledWidget(
        panel,
        "Pixel Size (microns)",
        new ValueField(
            panel,
            control.pixelSize_microns));

    auto squareSize = wxpex::LabeledWidget(
        panel,
        "Square Size (mm)",
        new ValueField(
            panel,
            control.squareSize_mm));

    auto sizer = LayoutLabeled(
        layoutOptions,
        sensorSize,
        pixelSize,
        squareSize);

    this->ConfigureSizer(std::move(sizer));
}


} // end namespace iris
