#include "iris/chess_chain_results.h"


namespace iris
{


ChessChainResults::ChessChainResults(
    ssize_t chessShapesId,
    ssize_t linesShapesId,
    ssize_t verticesShapesId)
    :
    mask{},
    level{},
    gaussian{},
    gradient{},
    canny{},
    hough{},
    harris{},
    vertices{},
    chess{},
    chessShapesId_(chessShapesId),
    linesShapesId_(linesShapesId),
    verticesShapesId_(verticesShapesId)
{

}


std::shared_ptr<draw::Pixels> ChessChainResults::Display(
    draw::ShapesControl shapesControl,
    const draw::LinesShapeSettings &linesShapeSettings,
    const draw::PointsShapeSettings &pointsShapeSettings,
    const ChessShapeSettings &chessShapeSettings,
    ThreadsafeColor<int32_t> &color,
    std::optional<HoughControl> houghControl) const
{
    if (!this->mask)
    {
        // There's nothing to display if the first filter in the chain has no
        // result.
        return {};
    }

    if (!this->level)
    {
        return std::make_shared<draw::Pixels>(color.Filter(*this->mask));
    }

    if (this->chess)
    {
        draw::Shapes shapes(this->chessShapesId_);

        shapes.EmplaceBack<iris::ChessShape>(
            chessShapeSettings,
            *this->chess);

        shapesControl.Set(shapes);

        return std::make_shared<draw::Pixels>(color.Filter(*this->level));
    }

    auto preprocessed = this->GetPreprocessedPixels_(color);

    if (this->hough)
    {
        this->DrawHoughResults_(
            shapesControl,
            linesShapeSettings,
            color,
            houghControl);

        return preprocessed;
    }

    if (this->vertices)
    {
        this->DrawVerticesResults_(
            shapesControl,
            pointsShapeSettings,
            color);

        return preprocessed;
    }

    return preprocessed;
}


std::shared_ptr<draw::Pixels> ChessChainResults::GetPreprocessedPixels_(
    ThreadsafeColor<int32_t> &color) const
{
    if (this->canny)
    {
        return std::make_shared<draw::Pixels>(this->canny->Colorize());
    }

    if (this->vertices)
    {
        // Display the gaussian output behind the vertices.
        return std::make_shared<draw::Pixels>(color.Filter(*this->gaussian));
    }

    if (this->harris)
    {
        return std::make_shared<draw::Pixels>(iris::ColorizeHarris(
            *this->harris));
    }

    if (this->gradient)
    {
        return std::make_shared<draw::Pixels>(this->gradient->Colorize());
    }

    if (this->gaussian)
    {
        return std::make_shared<draw::Pixels>(color.Filter(*this->gaussian));
    }

    assert(this->level);
    return std::make_shared<draw::Pixels>(color.Filter(*this->level));
}


void ChessChainResults::DrawHoughResults_(
    draw::ShapesControl shapesControl,
    const draw::LinesShapeSettings &linesShapeSettings,
    ThreadsafeColor<int32_t> &color,
    std::optional<HoughControl> houghControl) const
{
    assert(this->hough);

    auto houghResult = *this->hough;

    if (houghControl)
    {
        auto scale = static_cast<double>(color.GetSettings().maximum);
        ProcessMatrix space = houghResult.GetScaledSpace<int32_t>(scale);

        houghControl->Set(
            std::make_shared<draw::Pixels>(color.Filter(space)));
    }

    if (houghResult.lines.empty())
    {
        return;
    }

    draw::Shapes shapes(this->linesShapesId_);

    shapes.EmplaceBack<draw::LinesShape>(
        linesShapeSettings,
        houghResult.lines);

    shapesControl.Set(shapes);
}


void ChessChainResults::DrawVerticesResults_(
    draw::ShapesControl shapesControl,
    const draw::PointsShapeSettings &pointsShapeSettings,
    ThreadsafeColor<int32_t> &color) const
{
    assert(this->vertices);
    draw::Shapes shapes(this->verticesShapesId_);

    shapes.EmplaceBack<draw::PointsShape>(
        pointsShapeSettings,
        VerticesToPoints(*this->vertices));

    shapesControl.Set(shapes);
}


} // end namespace iris
