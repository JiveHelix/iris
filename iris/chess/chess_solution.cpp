#include "chess_solution.h"
#include "group_functions.h"
#include "chess_log.h"


namespace iris
{


ChessFromPoints::ChessFromPoints(
    const LineCollection &cornerLines_,
    const ChessSettings &settings)
    :
    cornerLines(cornerLines_),
    groups(),
    horizontal(),
    vertical(),
    intersections()
{
    // Begin the first group with the first line.
    this->groups.emplace_back(this->cornerLines.at(0));

    double groupSeparationDegrees = settings.groupSeparationDegrees;
    size_t index = 1;

    while (index < this->cornerLines.size())
    {
        AddLineToGroups(
            this->groups,
            this->cornerLines[index++],
            groupSeparationDegrees);
    }

    CombineFirstAndLast(this->groups, groupSeparationDegrees);

    this->groups = FilterByAngle(
        SplitGroups(
            this->groups,
            settings.maximumSpacing,
            settings.minimumSpacing));

    RemoveOutlierGroups(
        this->groups,
        settings.rowCount,
        settings.columnCount,
        settings.minimumLinesPerGroup);

    if (this->groups.empty())
    {
        return;
    }

    // There is at least one group, and
    // all remaining groups have the minimum number of lines.

    auto axisGroups = SelectAxisGroups(
        this->groups,
        settings.minimumSpacing,
        true);

    this->vertical = axisGroups.vertical;
    this->horizontal = axisGroups.horizontal;

    this->intersections = FormIntersections(
        axisGroups,
        settings.pointsChess.maximumPointError);
}


#ifdef ENABLE_CHESS_LOG
#define LOG_GROUPS                                                  \
    CHESS_LOG("groups size: ", this->groups.size());                \
                                                                    \
    for (auto &group: this->groups)                                 \
    {                                                               \
        std::cout << "  group.lines.size(): "                       \
            << group.lines.size() << "\n";                          \
    }                                                               \
                                                                    \
    std::cout << std::endl;
#else
#define LOG_GROUPS
#endif


ChessFromLines::ChessFromLines(
    const LineCollection &lines_,
    const ChessSettings &settings)
    :
    lines(lines_),
    groups(),
    horizontal(),
    vertical(),
    intersections()
{
    CHESS_LOG("lines: ", lines_.size());

    // Begin the first group with the first line.
    this->groups.emplace_back(this->lines.at(0));

    double groupSeparationDegrees = settings.groupSeparationDegrees;
    size_t index = 1;

    while (index < this->lines.size())
    {
        CHESS_LOG("Adding line: ", index);

        AddLineToGroups(
            this->groups,
            this->lines[index++],
            groupSeparationDegrees);

        LOG_GROUPS
    }

    CHESS_LOG("groups.size(): ", this->groups.size());

    CombineFirstAndLast(this->groups, groupSeparationDegrees);

    LOG_GROUPS

    this->groups = FilterByAngle(
        SplitGroups(
            this->groups,
            settings.maximumSpacing,
            settings.minimumSpacing));

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
        std::cout << "ChessFromLines groups is empty" << std::endl;
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

    this->intersections = FormIntersections(
        axisGroups,
        settings.pointsChess.maximumPointError);
}


ChessSolution::ChessSolution()
    :
    intersections()
{

}

ChessSolution::ChessSolution(const ChessOutput &chessOutput)
{
    if (std::holds_alternative<ChessFromPoints>(chessOutput))
    {
        this->intersections =
            std::get<ChessFromPoints>(chessOutput).intersections;

        auto &cornerLines =
            std::get<ChessFromPoints>(chessOutput).cornerLines;

        auto &horizontal =
            std::get<ChessFromPoints>(chessOutput).horizontal.lines;

        auto &vertical =
            std::get<ChessFromPoints>(chessOutput).vertical.lines;

        this->lines = Lines(
            std::begin(cornerLines),
            std::end(cornerLines));

        this->horizontal = Lines(
            std::begin(horizontal),
            std::end(horizontal));

        this->vertical = Lines(
            std::begin(vertical),
            std::end(vertical));
    }
    else
    {
        this->intersections =
            std::get<ChessFromLines>(chessOutput).intersections;

        this->lines =
            std::get<ChessFromLines>(chessOutput).lines;

        this->horizontal =
            std::get<ChessFromLines>(chessOutput).horizontal.lines;

        this->vertical =
            std::get<ChessFromLines>(chessOutput).vertical.lines;
    }
}


} // end namespace iris
