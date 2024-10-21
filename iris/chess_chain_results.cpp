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


void ChessChainResults::ClearShapes_(
    draw::AsyncShapesControl shapesControl) const
{
    draw::Shapes chessShapes(this->chessShapesId_);
    draw::Shapes linesShapes(this->linesShapesId_);
    draw::Shapes verticesShapes(this->verticesShapesId_);

    shapesControl.Set(chessShapes);
    shapesControl.Set(linesShapes);
    shapesControl.Set(verticesShapes);
}


std::shared_ptr<draw::Pixels> ChessChainResults::DisplayNode(
    ChessChainNodeSettings nodeSettings,
    draw::AsyncShapesControl shapesControl,
    const draw::LinesShapeSettings &linesShapeSettings,
    const draw::PointsShapeSettings &pointsShapeSettings,
    const ChessShapeSettings &chessShapeSettings,
    ThreadsafeColorMap<int32_t> &color,
    std::optional<HoughControl> houghControl) const
{
    this->ClearShapes_(shapesControl);

    auto pixels = this->GetNodePixels_(nodeSettings, color);

    if (!pixels)
    {
        pixels = this->GetPreprocessedPixels_(color);
    }

    if (nodeSettings.vertices.isSelected && this->vertices)
    {
        this->DrawVerticesResults_(
            shapesControl,
            pointsShapeSettings,
            color);

        return pixels;
    }

    if (nodeSettings.hough.isSelected && this->hough)
    {
        this->DrawHoughResults_(
            shapesControl,
            linesShapeSettings,
            color,
            houghControl);

        return pixels;
    }

    if (nodeSettings.chess.isSelected && this->chess)
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
    draw::AsyncShapesControl shapesControl,
    const draw::LinesShapeSettings &linesShapeSettings,
    const draw::PointsShapeSettings &pointsShapeSettings,
    const ChessShapeSettings &chessShapeSettings,
    ThreadsafeColorMap<int32_t> &color,
    std::optional<HoughControl> houghControl,
    std::optional<ChessChainNodeSettings> nodeSettings) const
{
    if (nodeSettings)
    {
        if (HasSelectedNode(*nodeSettings))
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

    this->ClearShapes_(shapesControl);

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
    ThreadsafeColorMap<int32_t> &color) const
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
    ThreadsafeColorMap<int32_t> &color) const
{
    if (nodeSettings.mask.isSelected)
    {
        if (!this->mask)
        {
            std::cout << "Cannot select a disabled node" << std::endl;
            return {};
        }

        std::cout << "Creating mask pixels" << std::endl;
        return std::make_shared<draw::Pixels>(color.Filter(*this->mask));
    }

    if (nodeSettings.level.isSelected)
    {
        if (!this->level)
        {
            std::cout << "Cannot select a disabled node" << std::endl;
            return {};
        }

        return std::make_shared<draw::Pixels>(color.Filter(*this->level));
    }

    if (nodeSettings.gaussian.isSelected)
    {
        if (!this->gaussian)
        {
            std::cout << "Cannot select a disabled node" << std::endl;
            return {};
        }

        return std::make_shared<draw::Pixels>(color.Filter(*this->gaussian));
    }

    if (nodeSettings.gradient.isSelected)
    {
        if (!this->gradient)
        {
            std::cout << "Cannot select a disabled node" << std::endl;
            return {};
        }

        return std::make_shared<draw::Pixels>(this->gradient->Colorize());
    }

    if (nodeSettings.harris.isSelected)
    {
        if (!this->harris)
        {
            std::cout << "Cannot select a disabled node" << std::endl;
            return {};
        }

        return std::make_shared<draw::Pixels>(
            iris::ColorizeHarris(*this->harris));
    }

    if (nodeSettings.vertices.isSelected)
    {
        if (!this->vertices || !this->gaussian)
        {
            std::cout << "Cannot select a disabled node" << std::endl;
            return {};
        }

        // Display the gaussian output behind the vertices.
        return std::make_shared<draw::Pixels>(color.Filter(*this->gaussian));
    }

    if (nodeSettings.canny.isSelected)
    {
        if (!this->canny)
        {
            std::cout << "Cannot select a disabled node" << std::endl;
            return {};
        }

        return std::make_shared<draw::Pixels>(this->canny->Colorize());
    }

    return {};
}


void ChessChainResults::DrawHoughResults_(
    draw::AsyncShapesControl shapesControl,
    const draw::LinesShapeSettings &linesShapeSettings,
    ThreadsafeColorMap<int32_t> &color,
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
    draw::AsyncShapesControl shapesControl,
    const draw::PointsShapeSettings &pointsShapeSettings,
    ThreadsafeColorMap<int32_t> &color) const
{
    assert(this->vertices);
    draw::Shapes shapes(this->verticesShapesId_);

    shapes.EmplaceBack<draw::PointsShape>(
        pointsShapeSettings,
        VerticesToPoints(*this->vertices));

    auto valuePointsShapeSettings = pointsShapeSettings;
    valuePointsShapeSettings.look.stroke.enable = true;
    valuePointsShapeSettings.look.stroke.color.hue = 120.0;
    valuePointsShapeSettings.look.stroke.color.saturation = 1.0;
    valuePointsShapeSettings.look.fill.enable = true;
    valuePointsShapeSettings.look.fill.color.hue = 120.0;
    valuePointsShapeSettings.look.fill.color.saturation = 1.0;

    shapes.EmplaceBack<draw::ValuePointsShape>(
        valuePointsShapeSettings,
        VerticesToValuePoints(*this->vertices));

    shapesControl.Set(shapes);
}


} // end namespace iris
