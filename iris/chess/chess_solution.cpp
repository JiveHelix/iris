#include "chess_solution.h"
#include "group_functions.h"
#include "chess_log.h"
#include "find_vertex.h"


namespace iris
{


#ifdef ENABLE_CHESS_LOG
#define LOG_GROUPS                                                  \
    CHESS_LOG(                                                      \
        "\nLOG_GROUPS\n  groups size: ",                            \
        this->groups.size());                                       \
                                                                    \
    for (auto &group: this->groups)                                 \
    {                                                               \
        std::cout << "    group.lines.size(): "                     \
            << group.lines.size() << "\n";                          \
    }                                                               \
                                                                    \
    std::cout << std::endl;
#else
#define LOG_GROUPS
#endif


ChessOutput::ChessOutput(
    const LineCollection &lines_,
    const Vertices &vertices_,
    const ChessSettings &settings)
    :
    lines(lines_),
    groups(),
    horizontal(),
    vertical(),
    vertices()
{
    CHESS_LOG("lines: ", lines_.size());

    // Begin the first group with the first line.
    this->groups.emplace_back(this->lines.at(0));

    double groupSeparationDegrees = settings.groupSeparationDegrees;
    size_t index = 1;

    while (index < this->lines.size())
    {
        AddLineToGroups(
            this->groups,
            this->lines[index++],
            groupSeparationDegrees);
    }

    LOG_GROUPS

    CombineFirstAndLast(this->groups, groupSeparationDegrees);

    LOG_GROUPS

    this->groups = SplitGroupsOnSpacing(
        this->groups,
        settings.maximumSpacing,
        settings.minimumSpacing);

    LOG_GROUPS

    this->groups = FilterByAngle(this->groups);

    LOG_GROUPS

    this->groups = SplitMissingLines(this->groups, settings.ratioLimit);

    LOG_GROUPS

    RemoveOutlierGroups(
        this->groups,
        settings.rowCount,
        settings.columnCount,
        settings.minimumLinesPerGroup);

    LOG_GROUPS

    if (this->groups.empty())
    {
        return;
    }

    // There is at least one group, and
    // all remaining groups have the minimum number of lines.
    CHESS_LOG("groups size: ", this->groups.size());

    auto axisGroups = SelectAxisGroups(
        this->groups,
        settings.minimumSpacing,
        false);

    this->vertical = axisGroups.vertical;
    this->horizontal = axisGroups.horizontal;

    this->vertices = FormVertices(
        axisGroups,
        vertices_,
        settings.maximumVertexDistance);
}


NamedVertices FormVertices(
    const AxisGroups &axisGroups,
    const Vertices &vertices,
    double maximumVertexDistance)
{
    using Line = typename ChessOutput::Line;

    if (!axisGroups.CanFormVertices())
    {
        return {};
    }

    auto result = NamedVertices{};

    size_t verticalCount = axisGroups.vertical.lines.size();
    size_t horizontalCount = axisGroups.horizontal.lines.size();

    // Iterate over the vertices of horizontal and vertical lines.
    // If an intersection has a vertex within the maximumVertexDistance
    // threshold, consider that point a vertex on the chess board.
    for (auto j: jive::Range<size_t>(0, horizontalCount))
    {
        const Line &horizontalLine = axisGroups.horizontal.lines[j];
        auto logicalRow = axisGroups.horizontal.GetLogicalIndex(j);

        for (auto i: jive::Range<size_t>(0, verticalCount))
        {
            const Line &verticalLine = axisGroups.vertical.lines[i];
            auto vertex = verticalLine.Intersect(horizontalLine);
            auto logicalColumn = axisGroups.vertical.GetLogicalIndex(i);

            auto vertexPoint = FindVertex(
                vertex,
                vertices,
                maximumVertexDistance);

            if (vertexPoint)
            {
                result.push_back(
                    {{logicalColumn, logicalRow}, vertexPoint->point});
            }
        }
    }

    return result;
}


ChessSolution::ChessSolution()
    :
    vertices()
{

}


ChessSolution::ChessSolution(const ChessOutput &chessOutput)
{
    this->vertices = chessOutput.vertices;
    this->lines = chessOutput.lines;
    this->horizontal = chessOutput.horizontal.lines;
    this->vertical = chessOutput.vertical.lines;
}


} // end namespace iris
