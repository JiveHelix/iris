#include "iris/chess_chain_node_settings.h"


namespace iris
{


ChessChainNodeSettingsModel::ChessChainNodeSettingsModel()
    :
    ChessChainNodeSettingsGroup::Model(),
    maskEndpoint_(
        this,
        this->mask.select,
        &ChessChainNodeSettingsModel::OnMask_),

    levelEndpoint_(
        this,
        this->level.select,
        &ChessChainNodeSettingsModel::OnLevel_),

    gaussianEndpoint_(
        this,
        this->gaussian.select,
        &ChessChainNodeSettingsModel::OnGaussian_),

    gradientEndpoint_(
        this,
        this->gradient.select,
        &ChessChainNodeSettingsModel::OnGradient_),

    cannyEndpoint_(
        this,
        this->canny.select,
        &ChessChainNodeSettingsModel::OnCanny_),

    houghEndpoint_(
        this,
        this->hough.select,
        &ChessChainNodeSettingsModel::OnHough_),

    harrisEndpoint_(
        this,
        this->harris.select,
        &ChessChainNodeSettingsModel::OnHarris_),

    verticesEndpoint_(
        this,
        this->vertices.select,
        &ChessChainNodeSettingsModel::OnVertices_),

    chessEndpoint_(
        this,
        this->chess.select,
        &ChessChainNodeSettingsModel::OnChess_),

    selected_(nullptr)
{

}


bool HasHighlight(const ChessChainNodeSettings &settings)
{
    return (
        settings.mask.highlight
        || settings.level.highlight
        || settings.gaussian.highlight
        || settings.gradient.highlight
        || settings.canny.highlight
        || settings.hough.highlight
        || settings.harris.highlight
        || settings.vertices.highlight
        || settings.chess.highlight);
}


void ChessChainNodeSettingsModel::OnMask_()
{
    this->Toggle(&this->mask);
}

void ChessChainNodeSettingsModel::OnLevel_()
{
    this->Toggle(&this->level);
}

void ChessChainNodeSettingsModel::OnGaussian_()
{
    this->Toggle(&this->gaussian);
}

void ChessChainNodeSettingsModel::OnGradient_()
{
    this->Toggle(&this->gradient);
}

void ChessChainNodeSettingsModel::OnCanny_()
{
    this->Toggle(&this->canny);
}

void ChessChainNodeSettingsModel::OnHough_()
{
    this->Toggle(&this->hough);
}

void ChessChainNodeSettingsModel::OnHarris_()
{
    this->Toggle(&this->harris);
}

void ChessChainNodeSettingsModel::OnVertices_()
{
    this->Toggle(&this->vertices);
}

void ChessChainNodeSettingsModel::OnChess_()
{
    this->Toggle(&this->chess);
}

void ChessChainNodeSettingsModel::Toggle(NodeSettingsModel *toSelect)
{
    if (this->selected_ == toSelect)
    {
        // Toggle off.
        // None are selected.
        this->selected_->highlight.Set(false);
        this->selected_ = nullptr;

        return;
    }

    if (this->selected_)
    {
        this->selected_->highlight.Set(false);
    }

    toSelect->highlight.Set(true);
    this->selected_ = toSelect;
}



} // end namespace iris
