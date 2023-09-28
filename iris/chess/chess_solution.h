#pragma once

#include <variant>
#include <tau/vector2d.h>
#include "iris/chess_settings.h"
#include "iris/chess/line_from_points.h"
#include "iris/chess/line_group.h"
#include "iris/chess/groups.h"
#include "iris/chess/named_vertex.h"


namespace iris
{


struct ChessFromVertices
{
    using Line = LineFromPoints;
    using Point = typename LineFromPoints::Point;
    using LineCollection = typename LineGroup<LineFromPoints>::LineCollection;

    ChessFromVertices() = default;

    ChessFromVertices(
        const LineCollection &vertexLines_,
        const ChessSettings &settings);

    LineCollection vertexLines;
    Groups<Line> groups;
    LineGroup<Line> horizontal;
    LineGroup<Line> vertical;
    NamedVertices vertices;
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
    NamedVertices vertices;
};


using ChessOutput = std::variant<ChessFromVertices, ChessFromLines>;


struct ChessSolution
{
public:
    using Lines = std::vector<tau::Line2d<double>>;

    ChessSolution();

    ChessSolution(const ChessOutput &chessOutput);

    std::vector<tau::Line2d<double>> lines;
    std::vector<tau::Line2d<double>> horizontal;
    std::vector<tau::Line2d<double>> vertical;
    NamedVertices vertices;
};


} // end namespace iris
