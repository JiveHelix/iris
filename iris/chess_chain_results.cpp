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


std::shared_ptr<draw::Pixels> ChessChainResults::DisplayNode(
    ChessChainNodeSettings nodeSettings,
    draw::ShapesControl shapesControl,
    const draw::LinesShapeSettings &linesShapeSettings,
    const draw::PointsShapeSettings &pointsShapeSettings,
    const ChessShapeSettings &chessShapeSettings,
    ThreadsafeColor<int32_t> &color,
    std::optional<HoughControl> houghControl) const
{
    std::cout << "DisplayNode GetNodePixels_" << std::endl;
    auto pixels = this->GetNodePixels_(nodeSettings, color);

    if (!pixels)
    {
        std::cout << "DisplayNode fallback to GetPreprocessedPixels_"
            << std::endl;
        pixels = this->GetPreprocessedPixels_(color);
    }

    if (nodeSettings.vertices.highlight && this->vertices)
    {
        this->DrawVerticesResults_(
            shapesControl,
            pointsShapeSettings,
            color);

        return pixels;
    }

    if (nodeSettings.hough.highlight && this->hough)
    {
        this->DrawHoughResults_(
            shapesControl,
            linesShapeSettings,
            color,
            houghControl);

        return pixels;
    }

    if (nodeSettings.chess.highlight && this->chess)
    {
        draw::Shapes shapes(this->chessShapesId_);

        shapes.EmplaceBack<iris::ChessShape>(
            chessShapeSettings,
            *this->chess);

        shapesControl.Set(shapes);

        return pixels;
    }

    return pixels;
}


std::shared_ptr<draw::Pixels> ChessChainResults::Display(
    draw::ShapesControl shapesControl,
    const draw::LinesShapeSettings &linesShapeSettings,
    const draw::PointsShapeSettings &pointsShapeSettings,
    const ChessShapeSettings &chessShapeSettings,
    ThreadsafeColor<int32_t> &color,
    std::optional<HoughControl> houghControl,
    std::optional<ChessChainNodeSettings> nodeSettings) const
{
    if (nodeSettings)
    {
        if (HasHighlight(*nodeSettings))
        {
            return this->DisplayNode(
                *nodeSettings,
                shapesControl,
                linesShapeSettings,
                pointsShapeSettings,
                chessShapeSettings,
                color,
                houghControl);
        }
    }

    auto pixels = this->GetPreprocessedPixels_(color);

    if (!this->level)
    {
        return pixels;
    }

    if (this->chess)
    {
        draw::Shapes shapes(this->chessShapesId_);

        shapes.EmplaceBack<iris::ChessShape>(
            chessShapeSettings,
            *this->chess);

        shapesControl.Set(shapes);

        return pixels;
    }

    if (this->hough)
    {
        this->DrawHoughResults_(
            shapesControl,
            linesShapeSettings,
            color,
            houghControl);

        return pixels;
    }

    if (this->vertices)
    {
        this->DrawVerticesResults_(
            shapesControl,
            pointsShapeSettings,
            color);

        return pixels;
    }

    return pixels;
}


std::shared_ptr<draw::Pixels> ChessChainResults::GetPreprocessedPixels_(
    ThreadsafeColor<int32_t> &color) const
{
    if (this->chess && this->level)
    {
        return std::make_shared<draw::Pixels>(color.Filter(*this->level));
    }

    if (this->canny)
    {
        return std::make_shared<draw::Pixels>(this->canny->Colorize());
    }

    if (this->vertices && this->gaussian)
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

    if (this->level)
    {
        return std::make_shared<draw::Pixels>(color.Filter(*this->level));
    }

    if (this->mask)
    {
        return std::make_shared<draw::Pixels>(color.Filter(*this->mask));
    }

    return {};
}


std::shared_ptr<draw::Pixels> ChessChainResults::GetNodePixels_(
    ChessChainNodeSettings nodeSettings,
    ThreadsafeColor<int32_t> &color) const
{
    if (nodeSettings.mask.highlight)
    {
        if (!this->mask)
        {
            std::cout << "Cannot highlight a disabled node" << std::endl;
            return {};
        }

        std::cout << "Creating mask pixels" << std::endl;
        return std::make_shared<draw::Pixels>(color.Filter(*this->mask));
    }

    if (nodeSettings.level.highlight)
    {
        if (!this->level)
        {
            std::cout << "Cannot highlight a disabled node" << std::endl;
            return {};
        }

        return std::make_shared<draw::Pixels>(color.Filter(*this->level));
    }

    if (nodeSettings.gaussian.highlight)
    {
        if (!this->gaussian)
        {
            std::cout << "Cannot highlight a disabled node" << std::endl;
            return {};
        }

        return std::make_shared<draw::Pixels>(color.Filter(*this->gaussian));
    }

    if (nodeSettings.gradient.highlight)
    {
        if (!this->gradient)
        {
            std::cout << "Cannot highlight a disabled node" << std::endl;
            return {};
        }

        return std::make_shared<draw::Pixels>(this->gradient->Colorize());
    }

    if (nodeSettings.harris.highlight)
    {
        if (!this->harris)
        {
            std::cout << "Cannot highlight a disabled node" << std::endl;
            return {};
        }

        return std::make_shared<draw::Pixels>(
            iris::ColorizeHarris(*this->harris));
    }

    if (nodeSettings.vertices.highlight)
    {
        if (!this->vertices || !this->gaussian)
        {
            std::cout << "Cannot highlight a disabled node" << std::endl;
            return {};
        }

        // Display the gaussian output behind the vertices.
        return std::make_shared<draw::Pixels>(color.Filter(*this->gaussian));
    }

    if (nodeSettings.canny.highlight)
    {
        if (!this->canny)
        {
            std::cout << "Cannot highlight a disabled node" << std::endl;
            return {};
        }

        return std::make_shared<draw::Pixels>(this->canny->Colorize());
    }

    return {};
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

    auto valuePointsShapeSettings = pointsShapeSettings;
    valuePointsShapeSettings.look.strokeEnable = true;
    valuePointsShapeSettings.look.strokeColor.hue = 120.0;
    valuePointsShapeSettings.look.strokeColor.saturation = 1.0;
    valuePointsShapeSettings.look.fillEnable = true;
    valuePointsShapeSettings.look.fillColor.hue = 120.0;
    valuePointsShapeSettings.look.fillColor.saturation = 1.0;

    shapes.EmplaceBack<draw::ValuePointsShape>(
        valuePointsShapeSettings,
        VerticesToValuePoints(*this->vertices));

    shapesControl.Set(shapes);
}


} // end namespace iris
