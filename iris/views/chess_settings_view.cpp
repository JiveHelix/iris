#include "chess_settings_view.h"

#include <wxpex/labeled_widget.h>
#include <wxpex/slider.h>
#include <wxpex/field.h>
#include <wxpex/check_box.h>


namespace iris
{


class PointsChessSettingsView: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    PointsChessSettingsView(
        wxWindow *parent,
        PointsChessControl controls,
        const LayoutOptions &layoutOptions = LayoutOptions{})
        :
        wxpex::Collapsible(parent, "Points")
    {
        using namespace wxpex;

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

        auto sizer = LayoutLabeled(
            layoutOptions,
            minimumPointsPerLine,
            maximumPointError,
            angleToleranceDegrees);

        this->ConfigureTopSizer(std::move(sizer));
    }
};


ChessSettingsView::ChessSettingsView(
    wxWindow *parent,
    ChessControl controls,
    const LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, "Chess")
{
    using namespace wxpex;

    auto enable = wxpex::LabeledWidget(
        this->GetPane(),
        "enable",
        new wxpex::CheckBox(this->GetPane(), "", controls.enable));

    auto usePoints = wxpex::LabeledWidget(
        this->GetPane(),
        "usePoints",
        new wxpex::CheckBox(this->GetPane(), "", controls.usePoints));

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

    auto pointsChess = LabeledWidget(
        this->GetPane(),
        "",
        new PointsChessSettingsView(
            this->GetPane(),
            controls.pointsChess,
            layoutOptions));
        
    auto minimumSpacing = LabeledWidget(
        this->GetPane(),
        "minimum spacing",
        new Field(
            this->GetPane(),
            controls.minimumSpacing));

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

    auto maximumSpacing = LabeledWidget(
        this->GetPane(),
        "maximum spacing",
        new Field(
            this->GetPane(),
            controls.maximumSpacing));

    auto ratioLimit = LabeledWidget(
        this->GetPane(),
        "spacing ratio threshold",
        new Field(
            this->GetPane(),
            controls.ratioLimit));

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
        enable,
        usePoints,
        rows,
        columns,
        pointsChess,
        minimumSpacing,
        groupOn,
        groupSeparationDegrees,
        minimumLinesPerGroup,
        maximumSpacing,
        ratioLimit,
        angleFilterLow,
        angleFilterHigh);

    this->ConfigureTopSizer(std::move(sizer));
}


} // end namespace iris
