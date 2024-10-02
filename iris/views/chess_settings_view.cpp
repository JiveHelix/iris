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
    std::optional<draw::NodeSettingsControl> nodeSettingsControl,
    const LayoutOptions &layoutOptions)
    :
    draw::CollapsibleNodeSettingsView(parent, "Chess", nodeSettingsControl)
{
    using namespace wxpex;

    auto enable = wxpex::LabeledWidget(
        this->GetPanel(),
        "enable",
        new wxpex::CheckBox(this->GetPanel(), "", controls.enable));

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

    auto minimumSpacing = LabeledWidget(
        this->GetPanel(),
        "minimum spacing",
        new Field(
            this->GetPanel(),
            controls.minimumSpacing));

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

    auto maximumVertexDistance = LabeledWidget(
        this->GetPanel(),
        "Maximum Vertex Distance",
        new Field(
            this->GetPanel(),
            controls.maximumVertexDistance));

    auto sizer = LayoutLabeled(
        layoutOptions,
        enable,
        rows,
        columns,
        minimumSpacing,
        groupSeparationDegrees,
        minimumLinesPerGroup,
        maximumSpacing,
        ratioLimit,
        maximumVertexDistance);

    this->ConfigureTopSizer(std::move(sizer));
}


} // end namespace iris
