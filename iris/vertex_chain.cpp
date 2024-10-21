#include "iris/vertex_chain.h"


namespace iris
{


VertexChainResults::VertexChainResults(ssize_t shapesId)
    :
    gaussian{},
    gradient{},
    harris{},
    vertex{},
    shapesId_(shapesId)
{

}


std::shared_ptr<draw::Pixels> VertexChainResults::Display(
    draw::AsyncShapesControl shapesControl,
    const draw::PointsShapeSettings &pointsShapeSettings,
    ThreadsafeColorMap<int32_t> &color) const
{
    if (!this->gaussian)
    {
        // There's nothing to display if the first filter in the chain has no
        // result.
        return {};
    }

    auto gaussianPixels =
        std::make_shared<draw::Pixels>(color.Filter(*this->gaussian));

    if (this->vertex)
    {
        draw::Shapes shapes(this->shapesId_);

        shapes.EmplaceBack<draw::PointsShape>(
            pointsShapeSettings,
            VerticesToPoints(*this->vertex));

        shapesControl.Set(shapes);

        return gaussianPixels;
    }

    if (this->harris)
    {
        return std::make_shared<draw::Pixels>(iris::ColorizeHarris(*this->harris));
    }

    // Harris didn't return a result.
    if (this->gradient)
    {
        return std::make_shared<draw::Pixels>(this->gradient->Colorize());
    }

    // Gradient has no result.
    return gaussianPixels;
}


template class VertexChain<DefaultLevelAdjustNode>;


} // end namespace iris
