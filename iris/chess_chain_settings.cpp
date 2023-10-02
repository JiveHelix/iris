#include "iris/chess_chain_settings.h"


namespace iris
{


ChessChainModel::ChessChainModel()
    :
    ChessChainGroup::Model(),
    imageSizeEndpoint_(this),
    maximumEndpoint_(this)
{

}

void ChessChainModel::SetImageSizeControl(const draw::SizeControl &sizeControl)
{
    this->imageSizeEndpoint_.ConnectUpstream(
        sizeControl,
        &ChessChainModel::OnImageSize_);
}

void ChessChainModel::SetMaximumControl(const MaximumControl &maximumControl)
{
    this->maximumEndpoint_.ConnectUpstream(
        maximumControl,
        &ChessChainModel::OnMaximum_);
}

void ChessChainModel::OnMaximum_(InProcess maximum)
{
    auto deferLevel = pex::MakeDefer(this->level.maximum);
    auto deferGaussian = pex::MakeDefer(this->gaussian.maximum);
    auto deferGradient = pex::MakeDefer(this->gradient.maximum);

    deferLevel.Set(maximum);
    deferGaussian.Set(maximum);
    deferGradient.Set(maximum);
}


void ChessChainModel::OnImageSize_(const draw::Size &size)
{
    auto deferMask = pex::MakeDefer(this->mask.imageSize);
    auto deferHough = pex::MakeDefer(this->hough.imageSize);
    deferMask.Set(size);
    deferHough.Set(size);
}


} // end namespace iris


template struct pex::Group
    <
        iris::ChessChainFields,
        iris::ChessChainTemplate,
        iris::ChessChainSettings
    >;
