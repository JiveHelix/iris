#include "iris/chess_chain.h"


namespace iris
{


ChessChainNodes::ChessChainNodes(
    SourceNode &source,
    ChessChainControl control,
    CancelControl cancel)
    :
    mask("Mask", source, control.mask, cancel),
    level(this->mask, control.level, cancel),
    gaussian("Gaussian", this->level, control.gaussian, cancel),

    gradientForCanny(
        this->gaussian,
        control.gradient,
        cancel),

    gradientForHarris(
        this->gaussian,
        control.gradient,
        cancel),

    canny("Canny", this->gradientForCanny, control.canny, cancel),
    hough("Hough", this->canny, control.hough, cancel),
    harris("Harris", this->gradientForHarris, control.harris, cancel),
    vertices("Vertices", this->harris, control.vertices, cancel),
    mix(this->vertices, this->hough, cancel),
    chess("chess", this->mix, control.chess, cancel)
{
    REGISTER_PEX_NAME(this, "ChessChainNodes");
}


ChessChain::ChessChain(
    SourceNode &sourceNode,
    ChessChainControl control,
    CancelControl cancel)
    :
    Base("ChessChain", sourceNode, control, cancel),
    linesShapesId_(),
    verticesShapesId_(),
    chessShapesId_(),
    nodes_(sourceNode, control, cancel),
    autoDetectEndpoint_(
        USE_REGISTER_PEX_NAME(this, "ChessChain"),
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

    ChainResults result(
        this->chessShapesId_.Get(),
        this->linesShapesId_.Get(),
        this->verticesShapesId_.Get());

    result.chess = this->nodes_.chess.GetResult();
    result.vertices = this->nodes_.vertices.GetResult();
    result.harris = this->nodes_.harris.GetResult();
    result.hough = this->nodes_.hough.GetResult();
    result.canny = this->nodes_.canny.GetResult();
    result.gradient = this->nodes_.gradientForHarris.GetResult();
    result.gaussian = this->nodes_.gaussian.GetResult();
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
    this->nodes_.gradientForCanny.AutoDetectSettings();
    this->nodes_.gradientForHarris.AutoDetectSettings();
}


} // end namespace iris
