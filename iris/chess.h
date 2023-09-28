#pragma once

#include <optional>
#include <tau/eigen.h>
#include <tau/vector2d.h>
#include <tau/line2d.h>
#include <tau/size.h>
#include <iris/vertex.h>
#include <iris/threadsafe_filter.h>

#include "iris/chess_settings.h"
#include "iris/chess/chess_error.h"
#include "iris/hough.h"
#include "iris/chess/chess_solution.h"


namespace iris
{


struct ChessInput
{
    using Lines = typename HoughResult<double>::Lines;

    std::optional<Vertices> vertices;
    std::optional<HoughResult<double>> hough;
};


class Chess
{
public:
    using Result = ChessSolution;

    Chess() = default;

    Chess(const ChessSettings &settings);

    std::optional<ChessSolution> Filter(const ChessInput &input);

    std::optional<ChessOutput> FilterPoints(const Vertices &vertices);

    std::optional<ChessOutput> FilterLines(
        const typename ChessInput::Lines &lines);

private:
    ChessSettings settings_;
};


} // end namespace iris
