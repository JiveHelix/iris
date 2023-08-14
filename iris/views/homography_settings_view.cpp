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
    wxpex::Collapsible(parent, "Homograpy")
{
    using namespace wxpex;
    using SensorSizeView = SizeView<double, 0>;
    using ValueField = typename SensorSizeView::ValueField;

    auto pane = this->GetBorderPane(borderStyle);

    auto sensorSize = wxpex::LabeledWidget(
        pane,
        "Sensor Size (pixels)",
        new SensorSizeView(
            pane,
            control.sensorSize_pixels));

    auto pixelSize = wxpex::LabeledWidget(
        pane,
        "Pixel Size (microns)",
        new ValueField(
            pane,
            control.pixelSize_microns));

    auto squareSize = wxpex::LabeledWidget(
        pane,
        "Square Size (mm)",
        new ValueField(
            pane,
            control.squareSize_mm));

    auto sizer = LayoutLabeled(
        layoutOptions,
        sensorSize,
        pixelSize,
        squareSize);

    this->ConfigureBorderPane(borderPixels, std::move(sizer));
}


} // end namespace iris
