#include "iris/lines_chain.h"
#include "iris/default.h"


namespace iris
{


LinesChainResults::LinesChainResults(ssize_t shapesId)
    :
    cannyChain{},
    hough{},
    shapesId_(shapesId)
{

}


std::shared_ptr<draw::Pixels> LinesChainResults::Display(
    draw::AsyncShapesControl shapesControl,
    const draw::LinesShapeSettings &linesShapeSettings,
    ThreadsafeColorMap<int32_t> &color,
    std::optional<HoughControl> houghControl) const
{
    if (!this->cannyChain)
    {
        // There's nothing to display if the first filter in the chain has no
        // result.
        return {};
    }

    auto cannyChainPixels = this->cannyChain->Display(color);

    if (this->hough)
    {
        auto houghResult = *this->hough;

        if (houghControl)
        {
            auto scale = static_cast<double>(color.GetSettings().maximum);
            ProcessMatrix space = houghResult.GetScaledSpace<int32_t>(scale);

            auto houghPixels =
                std::make_shared<draw::Pixels>(color.Filter(space));

            houghControl->Set(houghPixels);
        }

        if (houghResult.lines.empty())
        {
            return cannyChainPixels;
        }

        draw::Shapes shapes(this->shapesId_);

        shapes.EmplaceBack<draw::LinesShape>(
            linesShapeSettings,
            houghResult.lines);

        shapesControl.Set(shapes);
    }

    return cannyChainPixels;
}


template class LinesChain<DefaultLevelAdjustNode>;


} // end namespace iris
