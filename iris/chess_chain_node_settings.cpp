#include "iris/chess_chain_node_settings.h"


namespace iris
{


bool HasSelectedNode(const ChessChainNodeSettings &settings)
{
    return (
        settings.mask.isSelected
        || settings.level.isSelected
        || settings.gaussian.isSelected
        || settings.gradient.isSelected
        || settings.canny.isSelected
        || settings.hough.isSelected
        || settings.harris.isSelected
        || settings.vertices.isSelected
        || settings.chess.isSelected);
}


} // end namespace iris
