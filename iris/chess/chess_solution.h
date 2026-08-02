#pragma once

#include <tau/vector2d.h>
#include <ray/named_vertex.h>
#include <iris/filter_result.h>
#include <iris/vertex.h>
#include <iris/chess_settings.h>
#include <iris/chess/line_group.h>
#include <iris/chess/groups.h>
#include <iris/chess/axis_groups.h>


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
    ray::PlanarVertices vertices;
};


ray::PlanarVertices FormVertices(
    const AxisGroups &axisGroups,
    const Vertices &vertices,
    double maximumVertexDistance);


struct ChessSolution: public FilterResult
{
public:
    using Lines = std::vector<tau::Line2d<double>>;

    ChessSolution();

    ChessSolution(const ChessOutput &chessOutput);

    std::vector<tau::Line2d<double>> lines;
    std::vector<tau::Line2d<double>> horizontal;
    std::vector<tau::Line2d<double>> vertical;
    ray::PlanarVertices vertices;

    tau::Size<Eigen::Index> GetSize() const
    {
        return {0, 0};
    }

    void Resize(const tau::Size<Eigen::Index> &)
    {

    }
};


} // end namespace iris
