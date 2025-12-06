#pragma once

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

    Vertices vertices;
    HoughResult<double> hough;
};


class Chess
{
public:
    using Result = ChessSolution;

    Chess() = default;

    Chess(const ChessSettings &settings);

    bool Filter(const ChessInput &input, ChessSolution &result);

private:
    ChessSettings settings_;
};


} // end namespace iris
