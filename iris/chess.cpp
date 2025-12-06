#include "iris/chess.h"


namespace iris
{


Chess::Chess(const ChessSettings &settings)
    :
    settings_(settings)
{

}


bool Chess::Filter(const ChessInput &input, ChessSolution &result)
{
    if (!this->settings_.enable)
    {
        return false;
    }

    if (input.vertices.empty())
    {
        return false;
    }

    if (input.hough.lines.empty())
    {
        return false;
    }

    result = ChessOutput(input.hough.lines, input.vertices, this->settings_);

    return true;
}


} // end namespace iris
