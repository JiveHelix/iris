#include "iris/chess.h"
#include "iris/chess/line_collector.h"


namespace iris
{


Chess::Chess(const ChessSettings &settings)
    :
    settings_(settings)
{

}


std::optional<ChessOutput> Chess::FilterPoints(const CornerPoints &points)
{
    if (points.empty())
    {
        return {};
    }

    auto lines = LineCollector(this->settings_).FormLines(points);

    if (lines.empty())
    {
        return {};
    }

    if (this->settings_.enableGroup)
    {
        return ChessFromPoints(lines, this->settings_);
    }

    ChessFromPoints solution;
    solution.cornerLines = lines;

    return solution;
}


std::optional<ChessOutput> Chess::FilterLines(
    const typename ChessInput::Lines &lines)
{
    if (!this->settings_.enable)
    {
        return {};
    }

    if (lines.empty())
    {
        return {};
    }

    if (this->settings_.enableGroup)
    {
        return ChessFromLines(lines, this->settings_);
    }

    ChessFromLines solution;
    solution.lines = lines;

    return solution;
}


std::optional<ChessSolution> Chess::Filter(const ChessInput &input)
{
    if (!this->settings_.enable)
    {
        return {};
    }

    if (this->settings_.usePoints)
    {
        if (!input.points)
        {
            return {};
        }

        auto output = this->FilterPoints(*input.points);

        if (!output)
        {
            return {};
        }

        return ChessSolution(*output);
    }

    if (!input.hough)
    {
        return {};
    }

    auto output = this->FilterLines(input.hough->lines);

    if (!output)
    {
        return {};
    }

    return ChessSolution(*output);
}


} // end namespace iris
