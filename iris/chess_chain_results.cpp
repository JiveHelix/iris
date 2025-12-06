#include "iris/chess_chain_results.h"


namespace iris
{


ChessChainResults::ChessChainResults(
    int64_t chessShapesId,
    int64_t linesShapesId,
    int64_t verticesShapesId)
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
    const tau::Margins &margins,
    const ChessChainNodeSettings &nodeSettings,
    const draw::AsyncShapesControl &shapesControl,
    const draw::LinesShapeSettings &linesShapeSettings,
    const draw::PointsShapeSettings &pointsShapeSettings,
    const ChessShapeSettings &chessShapeSettings,
    ThreadsafeColorMap<int32_t> &color,
    HoughPixelsControl *houghControl) const
{
    this->ClearShapes_(shapesControl);

    auto pixels = this->GetNodePixels_(margins, nodeSettings, color);

    if (!pixels)
    {
        pixels = this->GetPreprocessedPixels_(margins, color);
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
    const tau::Margins &margins,
    const draw::AsyncShapesControl &shapesControl,
    const draw::LinesShapeSettings &linesShapeSettings,
    const draw::PointsShapeSettings &pointsShapeSettings,
    const ChessShapeSettings &chessShapeSettings,
    ThreadsafeColorMap<int32_t> &color,
    HoughPixelsControl *houghControl,
    ChessChainNodeSettings *nodeSettings) const
{
    if (nodeSettings)
    {
        if (HasSelectedNode(*nodeSettings))
        {
            return this->DisplayNode(
                margins,
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

    auto pixels = this->GetPreprocessedPixels_(margins, color);

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
    const tau::Margins &margins,
    ThreadsafeColorMap<int32_t> &color) const
{
    if (this->chess && this->level)
    {
        return color.Filter(*this->level);
    }

    if (this->canny)
    {
        return this->canny->Colorize(margins);
    }

    if (this->vertices && this->gaussian)
    {
        // Display the gaussian output behind the vertices.
        return color.Filter(*this->gaussian);
    }

    if (this->harris)
    {
        return iris::ColorizeHarris(margins, *this->harris);
    }

    if (this->gradient)
    {
        return this->gradient->Colorize(margins);
    }

    if (this->gaussian)
    {
        return color.Filter(*this->gaussian);
    }

    if (this->level)
    {
        return color.Filter(*this->level);
    }

    if (this->mask)
    {
        return color.Filter(*this->mask);
    }

    return {};
}


std::shared_ptr<draw::Pixels> ChessChainResults::GetNodePixels_(
    const tau::Margins &margins,
    const ChessChainNodeSettings &nodeSettings,
    ThreadsafeColorMap<int32_t> &color) const
{
    if (nodeSettings.mask.isSelected)
    {
        if (!this->mask)
        {
            std::cout << "Cannot select a disabled node" << std::endl;
            return {};
        }

        return color.Filter(*this->mask);
    }

    if (nodeSettings.level.isSelected)
    {
        if (!this->level)
        {
            std::cout << "Cannot select a disabled node" << std::endl;
            return {};
        }

        return color.Filter(*this->level);
    }

    if (nodeSettings.gaussian.isSelected)
    {
        if (!this->gaussian)
        {
            std::cout << "Cannot select a disabled node" << std::endl;
            return {};
        }

        return color.Filter(*this->gaussian);
    }

    if (nodeSettings.gradient.isSelected)
    {
        if (!this->gradient)
        {
            std::cout << "Cannot select a disabled node" << std::endl;
            return {};
        }

        return this->gradient->Colorize(margins);
    }

    if (nodeSettings.harris.isSelected)
    {
        if (!this->harris)
        {
            std::cout << "Cannot select a disabled node" << std::endl;
            return {};
        }

        return iris::ColorizeHarris(margins, *this->harris);
    }

    if (nodeSettings.vertices.isSelected)
    {
        if (!this->vertices || !this->gaussian)
        {
            std::cout << "Cannot select a disabled node" << std::endl;
            return {};
        }

        // Display the gaussian output behind the vertices.
        return color.Filter(*this->gaussian);
    }

    if (nodeSettings.canny.isSelected)
    {
        if (!this->canny)
        {
            std::cout << "Cannot select a disabled node" << std::endl;
            return {};
        }

        return this->canny->Colorize(margins);
    }

    return {};
}


void ChessChainResults::DrawHoughResults_(
    const draw::AsyncShapesControl &shapesControl,
    const draw::LinesShapeSettings &linesShapeSettings,
    ThreadsafeColorMap<int32_t> &color,
    HoughPixelsControl *houghControl) const
{
    assert(this->hough);

    auto houghResult = *this->hough;

    if (houghControl)
    {
        auto scale = static_cast<double>(color.GetSettings().maximum);
        ProcessMatrix space = houghResult.GetScaledSpace<int32_t>(scale);

        houghControl->Set(color.Filter(space));
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
    const draw::AsyncShapesControl &shapesControl,
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
