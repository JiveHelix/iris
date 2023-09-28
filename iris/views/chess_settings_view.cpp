#include "chess_settings_view.h"

#include <wxpex/labeled_widget.h>
#include <wxpex/slider.h>
#include <wxpex/field.h>
#include <wxpex/check_box.h>


namespace iris
{


class VertexChessSettingsView: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    VertexChessSettingsView(
        wxWindow *parent,
        VertexChessControl controls,
        const LayoutOptions &layoutOptions = LayoutOptions{})
        :
        wxpex::Collapsible(parent, "Points")
    {
        using namespace wxpex;

        auto minimumPointsPerLine = LabeledWidget(
            this->GetPanel(),
            "minimum points per line",
            new ValueSlider(
                this->GetPanel(),
                controls.minimumPointsPerLine,
                controls.minimumPointsPerLine.value));

        auto maximumPointError = LabeledWidget(
            this->GetPanel(),
            "maximum point error",
            new Field(
                this->GetPanel(),
                controls.maximumPointError));

        auto angleToleranceDegrees = LabeledWidget(
            this->GetPanel(),
            "angle tolerance (degrees)",
            new Field(
                this->GetPanel(),
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
        this->GetPanel(),
        "enable",
        new wxpex::CheckBox(this->GetPanel(), "", controls.enable));

    auto useVertices = wxpex::LabeledWidget(
        this->GetPanel(),
        "Use Vertices",
        new wxpex::CheckBox(this->GetPanel(), "", controls.useVertices));

    auto rows = LabeledWidget(
        this->GetPanel(),
        "Row count",
        new Field(
            this->GetPanel(),
            controls.rowCount));

    auto columns = LabeledWidget(
        this->GetPanel(),
        "Column count",
        new Field(
            this->GetPanel(),
            controls.columnCount));

    auto vertexChess = LabeledWidget(
        this->GetPanel(),
        "",
        new VertexChessSettingsView(
            this->GetPanel(),
            controls.vertexChess,
            layoutOptions));

    auto minimumSpacing = LabeledWidget(
        this->GetPanel(),
        "minimum spacing",
        new Field(
            this->GetPanel(),
            controls.minimumSpacing));

    auto groupOn = wxpex::LabeledWidget(
        this->GetPanel(),
        "Group",
        new wxpex::CheckBox(this->GetPanel(), "", controls.enableGroup));

    auto groupSeparationDegrees = LabeledWidget(
        this->GetPanel(),
        "group separation (degrees)",
        new Field(
            this->GetPanel(),
            controls.groupSeparationDegrees));

    auto minimumLinesPerGroup = LabeledWidget(
        this->GetPanel(),
        "minimum lines per group",
        new Field(
            this->GetPanel(),
            controls.minimumLinesPerGroup));

    auto maximumSpacing = LabeledWidget(
        this->GetPanel(),
        "maximum spacing",
        new Field(
            this->GetPanel(),
            controls.maximumSpacing));

    auto ratioLimit = LabeledWidget(
        this->GetPanel(),
        "spacing ratio threshold",
        new Field(
            this->GetPanel(),
            controls.ratioLimit));

    auto angleFilterLow = wxpex::LabeledWidget(
        this->GetPanel(),
        "Angle filter low",
        new wxpex::ValueSlider(
            this->GetPanel(),
            controls.angleFilter.low,
            controls.angleFilter.low.value));

    auto angleFilterHigh = wxpex::LabeledWidget(
        this->GetPanel(),
        "Angle filter high",
        new wxpex::ValueSlider(
            this->GetPanel(),
            controls.angleFilter.high,
            controls.angleFilter.high.value));

    auto sizer = LayoutLabeled(
        layoutOptions,
        enable,
        useVertices,
        rows,
        columns,
        vertexChess,
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
