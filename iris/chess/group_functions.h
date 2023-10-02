#pragma once


#include "iris/chess/chess_log.h"
#include "iris/chess/groups.h"
#include "iris/chess/axis_groups.h"
#include "iris/chess/named_vertex.h"


namespace iris
{


void AddLineToGroups(
    Groups &groups,
    const tau::Line2d<double> &line,
    double groupSeparation_deg);


Groups FilterByAngle(const Groups &groups);


Groups SplitGroupsOnAngles(const Groups &groups);


// Split the groups if there are any gaps larger than spacing limit.
Groups SplitGroupsOnSpacing(
    const Groups &groups,
    double maximumSpacing,
    double minimumSpacing);


Groups SplitMissingLines(const Groups &groups, double ratioLimit);


void RemoveOutlierGroups(
    Groups &groups,
    size_t rowCount,
    size_t columnCount,
    size_t minimumLinesPerGroup);


void SortDescending(
    Groups &groups,
    double verticalAngle);


void CombineFirstAndLast(Groups &groups, double groupSeparation_deg);


AxisGroups SelectAxisGroups(
    Groups &groups,
    double minimumSpacing,
    bool allowSkippedLines);


} // end namespace iris
