#include "iris/views/chess_shape_view.h"

#include <wxpex/check_box.h>
#include <draw/views/points_shape_view.h>
#include <draw/views/lines_shape_view.h>
#include <draw/views/font_look_view.h>


namespace iris
{


ChessShapeView::ChessShapeView(
    wxWindow *parent,
    ChessShapeControl controls,
    const LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, "Chess Shape")
{
    auto displayIntersections =
        new wxpex::CheckBox(
            this->GetPanel(),
            "Display Intersections",
            controls.displayIntersections);

    auto labelIntersections =
        new wxpex::CheckBox(
            this->GetPanel(),
            "Label Intersections",
            controls.labelIntersections);

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

    auto intersectionsShape =
        new draw::PointsShapeView(
            this->GetPanel(),
            "Intersections Shape",
            controls.intersectionsShape,
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
        displayIntersections,
        labelIntersections,
        displayHorizontals,
        displayVerticals,
        intersectionsShape,
        labelsLook,
        horizontalsShape,
        verticalsShape);

    this->ConfigureTopSizer(std::move(sizer));
}


} // end namespace iris
