#include "iris/views/chess_shape.h"
#include <draw/lines_shape.h>

namespace iris
{


ChessShapeSettings ChessShapeSettings::Default()
{
    ChessShapeSettings result{{
        true,
        true,
        true,
        true,
        draw::PointsShapeSettings::Default(),
        draw::FontLook::Default(),
        draw::LinesShapeSettings::Default(),
        draw::LinesShapeSettings::Default()}};

    result.intersectionsShape.look.fillEnable = false;
    result.intersectionsShape.look.strokeEnable = true;
    result.intersectionsShape.look.strokeColor = {{0.0, 1.0, 1.0}};
    result.intersectionsShape.radius = 3.0;

    result.labelsLook.color = {{300.0, 1.0, 1.0}};
    result.horizontalsShape.look.strokeColor = {{120.0, 1.0, 1.0}};
    result.verticalsShape.look.strokeColor = {{240.0, 1.0, 1.0}};

    return result;
}


ChessShape::ChessShape(
    const ChessShapeSettings &settings,
    const ChessSolution &chessSolution)
    :
    settings_(settings),
    chessSolution_(chessSolution)
{

}


void ChessShape::Draw(wxpex::GraphicsContext &context)
{
    if (!this->chessSolution_.intersections.empty())
    {
        if (this->settings_.displayIntersections)
        {
            auto pixels =
                IntersectionsToPixels(this->chessSolution_.intersections);

            auto cornerPointsShape = draw::PointsShape(
                this->settings_.intersectionsShape,
                pixels);

            cornerPointsShape.Draw(context);
        }

        if (this->settings_.labelIntersections)
        {
            draw::ConfigureFontLook(context, this->settings_.labelsLook);

            for (const auto &intersection: this->chessSolution_.intersections)
            {
                context->DrawText(
                    intersection.logical.GetAsString(),
                    intersection.pixel.x,
                    intersection.pixel.y);
            }
        }
    }

    if (this->settings_.displayHorizontals
            && !this->chessSolution_.horizontal.empty())
    {
        auto linesShape = draw::LinesShape(
            this->settings_.horizontalsShape,
            this->chessSolution_.horizontal);

        linesShape.Draw(context);
    }

    if (this->settings_.displayVerticals
            && !this->chessSolution_.vertical.empty())
    {
        auto linesShape = draw::LinesShape(
            this->settings_.verticalsShape,
            this->chessSolution_.vertical);

        linesShape.Draw(context);
    }
}


} // end namespace iris


template struct pex::Group
<
    iris::ChessShapeFields,
    iris::ChessShapeTemplate,
    iris::ChessShapeSettings
>;
