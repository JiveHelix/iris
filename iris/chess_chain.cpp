#include "iris/chess_chain.h"


namespace iris
{


ChessChainResults::ChessChainResults(ssize_t chessShapesId)
    :
    mask{},
    level{},
    vertices{},
    lines{},
    chess{},
    chessShapesId_(chessShapesId)
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

    auto leveledPixels =
        std::make_shared<draw::Pixels>(color.Filter(*this->level));

    if (this->chess)
    {
        draw::Shapes shapes(this->chessShapesId_);

        shapes.EmplaceBack<iris::ChessShape>(
            chessShapeSettings,
            *this->chess);

        shapesControl.Set(shapes);

        return leveledPixels;
    }

    if (this->lines)
    {
        return this->lines->Display(
            shapesControl,
            linesShapeSettings,
            color,
            houghControl);
    }

    if (this->vertices)
    {
        return this->vertices->Display(
            shapesControl,
            pointsShapeSettings,
            color);
    }

    return leveledPixels;
}


ChessChainNodes::ChessChainNodes(
    SourceNode &source,
    ChessChainControl control,
    CancelControl cancel)
    :
    mask("mask", source, control.mask, cancel),
    level(this->mask, control.level, cancel),
    lines(this->level, control.lines, cancel),
    vertices(this->level, control.vertices, cancel),
    mux(this->vertices, this->lines, control.chess.useVertices, cancel),
    chess("chess", this->mux, control.chess, cancel)
{

}


ChessChain::ChessChain(
    SourceNode &sourceNode,
    ChessChainControl control,
    CancelControl cancel)
    :
    Base("ChessChain", sourceNode, control, cancel),
    chessShapesId_(),
    nodes_(sourceNode, control, cancel),
    autoDetectEndpoint_(
        this,
        control.autoDetectSettings,
        &ChessChain::AutoDetectSettings)
{

}

std::optional<ChessChain::Result> ChessChain::DoGetResult()
{
    if (!this->settings_.enable)
    {
        return {};
    }

    return this->nodes_.chess.GetResult();
}

std::optional<ChessChain::ChainResults> ChessChain::GetChainResults()
{
    if (!this->settings_.enable)
    {
        return {};
    }

    {
        std::lock_guard lock(this->mutex_);
        this->settingsChanged_ = false;
    }

    ChainResults result(this->chessShapesId_.Get());

    result.chess = this->nodes_.chess.GetResult();

    if (this->settings_.chess.useVertices)
    {
        result.vertices = this->nodes_.vertices.GetChainResults();
    }
    else
    {
        result.lines = this->nodes_.lines.GetChainResults();
    }

    result.level = this->nodes_.level.GetResult();
    result.mask = this->nodes_.mask.GetResult();

    std::lock_guard lock(this->mutex_);

    if (this->settingsChanged_)
    {
        return {};
    }

    return result;
}


void ChessChain::AutoDetectSettings()
{
    this->nodes_.level.AutoDetectSettings();
    this->nodes_.vertices.AutoDetectSettings();
    this->nodes_.lines.AutoDetectSettings();
}


} // end namespace iris
