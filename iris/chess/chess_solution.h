#pragma once

#include <tau/vector2d.h>
#include "iris/vertex.h"
#include "iris/chess_settings.h"
#include "iris/chess/line_from_points.h"
#include "iris/chess/line_group.h"
#include "iris/chess/groups.h"
#include "iris/chess/named_vertex.h"
#include "iris/chess/axis_groups.h"


namespace iris
{


struct ChessOutput
{
    using Line = tau::Line2d<double>;
    using Point = tau::Point2d<double>;
    using LineCollection = typename LineGroup::LineCollection;

    ChessOutput() = default;

    ChessOutput(
        const LineCollection &lines_,
        const Vertices &vertices_,
        const ChessSettings &settings);

    LineCollection lines;
    Groups groups;
    LineGroup horizontal;
    LineGroup vertical;
    NamedVertices vertices;
};


NamedVertices FormVertices(
    const AxisGroups &axisGroups,
    const Vertices &vertices,
    double maximumVertexDistance);


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
