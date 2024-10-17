#include "iris/views/chess_shape.h"
#include <draw/lines_shape.h>

namespace iris
{


ChessShapeSettings::ChessShapeSettings()
    :
    ChessShapeTemplate<pex::Identity>{
        true,
        true,
        true,
        true,
        draw::PointsShapeSettings{},
        draw::FontLook{},
        draw::LinesShapeSettings{},
        draw::LinesShapeSettings{}}
{
    this->verticesShape.look.fill.enable = false;
    this->verticesShape.look.stroke.enable = true;
    this->verticesShape.look.stroke.color = {{0.0, 1.0, 1.0, 1.0}};
    this->verticesShape.radius = 3.0;

    this->labelsLook.color = {{300.0, 1.0, 1.0}};
    this->horizontalsShape.look.stroke.color = {{120.0, 1.0, 1.0, 1.0}};
    this->verticalsShape.look.stroke.color = {{240.0, 1.0, 1.0, 1.0}};
}


ChessShape::ChessShape(
    const ChessShapeSettings &settings,
    const ChessSolution &chessSolution)
    :
    settings_(settings),
    chessSolution_(chessSolution)
{

}


void ChessShape::Draw(draw::DrawContext &context)
{
    if (!this->chessSolution_.vertices.empty())
    {
        if (this->settings_.displayVertices)
        {
            auto pixels =
                NamedVerticesToPixels(this->chessSolution_.vertices);

            auto verticesShape = draw::PointsShape(
                this->settings_.verticesShape,
                pixels);

            verticesShape.Draw(context);
        }

        if (this->settings_.labelVertices)
        {
            draw::ConfigureFontLook(context, this->settings_.labelsLook);

            for (const auto &vertex: this->chessSolution_.vertices)
            {
                context->DrawText(
                    vertex.logical.GetAsString(),
                    vertex.pixel.x,
                    vertex.pixel.y);
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
    pex::PlainT<iris::ChessShapeSettings>
>;
