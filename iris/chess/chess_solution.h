#pragma once

#include <variant>
#include <tau/vector2d.h>
#include "iris/chess_settings.h"
#include "iris/chess/line_from_points.h"
#include "iris/chess/line_group.h"
#include "iris/chess/groups.h"
#include "iris/chess/intersection.h"


namespace iris
{


struct ChessFromPoints
{
    using Line = LineFromPoints;
    using Point = typename LineFromPoints::Point;
    using LineCollection = typename LineGroup<LineFromPoints>::LineCollection;

    ChessFromPoints() = default;

    ChessFromPoints(
        const LineCollection &cornerLines_,
        const ChessSettings &settings);

    LineCollection cornerLines;
    Groups<Line> groups;
    LineGroup<Line> horizontal;
    LineGroup<Line> vertical;
    Intersections intersections;
};


struct ChessFromLines
{
    using Line = tau::Line2d<double>;
    using Point = tau::Point2d<double>;
    using LineCollection = typename LineGroup<Line>::LineCollection;

    ChessFromLines() = default;

    ChessFromLines(
        const LineCollection &lines_,
        const ChessSettings &settings);

    LineCollection lines;
    Groups<Line> groups;
    LineGroup<Line> horizontal;
    LineGroup<Line> vertical;
    Intersections intersections;
};


using ChessOutput = std::variant<ChessFromPoints, ChessFromLines>;


struct ChessSolution
{
public:
    using Lines = std::vector<tau::Line2d<double>>;

    ChessSolution();

    ChessSolution(const ChessOutput &chessOutput);

    std::vector<tau::Line2d<double>> lines;
    std::vector<tau::Line2d<double>> horizontal;
    std::vector<tau::Line2d<double>> vertical;
    Intersections intersections;
};


} // end namespace iris
