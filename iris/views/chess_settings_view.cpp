#include "chess_settings_view.h"

#include <wxpex/labeled_widget.h>
#include <wxpex/slider.h>
#include <wxpex/field.h>
#include <wxpex/check_box.h>


namespace iris
{


ChessSettingsView::ChessSettingsView(
    wxWindow *parent,
    ChessControl controls,
    const LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, "Chess")
{
    using namespace wxpex;

    auto rows = LabeledWidget(
        this->GetPane(),
        "Row count",
        new Field(
            this->GetPane(),
            controls.rowCount));

    auto columns = LabeledWidget(
        this->GetPane(),
        "Column count",
        new Field(
            this->GetPane(),
            controls.columnCount));

    auto minimumPointsPerLine = LabeledWidget(
        this->GetPane(),
        "minimum points per line",
        new ValueSlider(
            this->GetPane(),
            controls.minimumPointsPerLine,
            controls.minimumPointsPerLine.value));

    auto maximumPointError = LabeledWidget(
        this->GetPane(),
        "maximum point error",
        new Field(
            this->GetPane(),
            controls.maximumPointError));

    auto angleToleranceDegrees = LabeledWidget(
        this->GetPane(),
        "angle tolerance (degrees)",
        new Field(
            this->GetPane(),
            controls.angleToleranceDegrees));

    auto lineSeparation = LabeledWidget(
        this->GetPane(),
        "line separation",
        new Field(
            this->GetPane(),
            controls.lineSeparation));

    auto groupOn = wxpex::LabeledWidget(
        this->GetPane(),
        "Group",
        new wxpex::CheckBox(this->GetPane(), "", controls.enableGroup));

    auto groupSeparationDegrees = LabeledWidget(
        this->GetPane(),
        "group separation (degrees)",
        new Field(
            this->GetPane(),
            controls.groupSeparationDegrees));

    auto minimumLinesPerGroup = LabeledWidget(
        this->GetPane(),
        "minimum lines per group",
        new Field(
            this->GetPane(),
            controls.minimumLinesPerGroup));

    auto spacingLimit = LabeledWidget(
        this->GetPane(),
        "spacing limit",
        new Field(
            this->GetPane(),
            controls.spacingLimit));

    auto angleFilterLow = wxpex::LabeledWidget(
        this->GetPane(),
        "Angle filter low",
        new wxpex::ValueSlider(
            this->GetPane(),
            controls.angleFilter.low,
            controls.angleFilter.low.value));

    auto angleFilterHigh = wxpex::LabeledWidget(
        this->GetPane(),
        "Angle filter high",
        new wxpex::ValueSlider(
            this->GetPane(),
            controls.angleFilter.high,
            controls.angleFilter.high.value));

    auto sizer = LayoutLabeled(
        layoutOptions,
        rows,
        columns,
        minimumPointsPerLine,
        maximumPointError,
        angleToleranceDegrees,
        lineSeparation,
        groupOn,
        groupSeparationDegrees,
        minimumLinesPerGroup,
        spacingLimit,
        angleFilterLow,
        angleFilterHigh);

    this->ConfigureTopSizer(sizer.release());
}


} // end namespace iris
