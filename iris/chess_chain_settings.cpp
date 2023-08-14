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
    this->lines.SetImageSizeControl(sizeControl);

    this->imageSizeEndpoint_.ConnectUpstream(
        sizeControl,
        &ChessChainModel::OnImageSize_);
}

void ChessChainModel::SetMaximumControl(const MaximumControl &maximumControl)
{
    this->corners.SetMaximumControl(maximumControl);
    this->lines.SetMaximumControl(maximumControl);

    this->maximumEndpoint_.ConnectUpstream(
        maximumControl,
        &ChessChainModel::OnMaximum_);
}

void ChessChainModel::OnMaximum_(InProcess maximum)
{
    this->level.maximum.Set(maximum);
}


void ChessChainModel::OnImageSize_(const draw::Size &size)
{
    this->mask.imageSize.Set(size);
}


} // end namespace iris


template struct pex::Group
    <
        iris::ChessChainFields,
        iris::ChessChainTemplate,
        iris::ChessChainSettings
    >;
