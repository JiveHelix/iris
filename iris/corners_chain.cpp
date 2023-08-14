#include "iris/corners_chain.h"


namespace iris
{


CornersChainResults::CornersChainResults(ssize_t shapesId)
    :
    gaussian{},
    gradient{},
    harris{},
    corner{},
    shapesId_(shapesId)
{

}


std::shared_ptr<draw::Pixels> CornersChainResults::Display(
    draw::ShapesControl shapesControl,
    const draw::PointsShapeSettings &pointsShapeSettings,
    ThreadsafeColor<int32_t> &color) const
{
    if (!this->gaussian)
    {
        // There's nothing to display if the first filter in the chain has no
        // result.
        return {};
    }

    auto gaussianPixels =
        std::make_shared<draw::Pixels>(color.Filter(*this->gaussian));

    if (this->corner)
    {
        draw::Shapes shapes(this->shapesId_);

        shapes.EmplaceBack<draw::PointsShape>(
            pointsShapeSettings,
            CornerPointsToPoints(*this->corner));

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


template class CornersChain<DefaultLevelAdjustNode>;


} // end namespace iris
