#include "iris/views/chess_shape_view.h"

#include <wxpex/layout_items.h>
#include <wxpex/check_box.h>
#include <draw/views/points_shape_view.h>
#include <draw/views/lines_shape_view.h>
#include <draw/views/font_look_view.h>

#include "iris/views/defaults.h"


namespace iris
{


ChessShapeView::ChessShapeView(
    wxWindow *parent,
    ChessShapeControl controls,
    const LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, "Chess Shape", borderStyle)
{
    auto displayVertices =
        new wxpex::CheckBox(
            this->GetPanel(),
            "Display Vertices",
            controls.displayVertices);

    auto labelVertices =
        new wxpex::CheckBox(
            this->GetPanel(),
            "Label Vertices",
            controls.labelVertices);

    auto displayHorizontals =
        new wxpex::CheckBox(
            this->GetPanel(),
            "Display Horizontals",
            controls.displayHorizontals);

    auto displayVerticals =
        new wxpex::CheckBox(
            this->GetPanel(),
            "Display Verticals",
            controls.displayVerticals);

    auto verticesShape =
        new draw::PointsShapeView(
            this->GetPanel(),
            "Vertices Shape",
            controls.verticesShape,
            layoutOptions);

    auto labelsLook =
        new draw::FontLookView(
            this->GetPanel(),
            "Labels Look",
            controls.labelsLook,
            layoutOptions);

    auto horizontalsShape =
        new draw::LinesShapeView(
            this->GetPanel(),
            "Horizontals Shape",
            controls.horizontalsShape,
            layoutOptions);

    auto verticalsShape =
        new draw::LinesShapeView(
            this->GetPanel(),
            "Verticals Shape",
            controls.verticalsShape,
            layoutOptions);

    auto sizer = wxpex::LayoutItems(
        wxpex::verticalItems,
        displayVertices,
        labelVertices,
        displayHorizontals,
        displayVerticals,
        verticesShape,
        labelsLook,
        horizontalsShape,
        verticalsShape);

    this->ConfigureSizer(std::move(sizer));
}


} // end namespace iris
