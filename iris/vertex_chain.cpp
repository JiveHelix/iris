#include "iris/vertex_chain.h"


namespace iris
{


VertexChainResults::VertexChainResults(int64_t shapesId)
    :
    gaussian{},
    gradient{},
    harris{},
    vertex{},
    shapesId_(shapesId)
{

}


std::shared_ptr<draw::Pixels> VertexChainResults::Display(
    const tau::Margins &margins,
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

    auto gaussianPixels = color.Filter(*this->gaussian);

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
        return iris::ColorizeHarris(margins, *this->harris);
    }

    // Harris didn't return a result.
    if (this->gradient)
    {
        return this->gradient->Colorize(margins);
    }

    // Gradient has no result.
    return gaussianPixels;
}


template class VertexChain<DefaultLevelAdjustNode>;


} // end namespace iris
