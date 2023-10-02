#include "iris/chess.h"


namespace iris
{


Chess::Chess(const ChessSettings &settings)
    :
    settings_(settings)
{

}


std::optional<ChessSolution> Chess::Filter(const ChessInput &input)
{
    if (!this->settings_.enable)
    {
        return {};
    }

    if (input.vertices.empty())
    {
        return {};
    }

    if (input.hough.lines.empty())
    {
        return {};
    }

    return ChessSolution(
        ChessOutput(input.hough.lines, input.vertices, this->settings_));
}


} // end namespace iris
