#include "iris/chess/group_functions.h"


namespace iris
{


void AddLineToGroups(
    Groups &groups,
    const tau::Line2d<double> &line,
    double groupSeparation_deg)
{
    double angle = line.GetAngleDegrees();

    CHESS_LOG(" line: ", angle);

    auto group = std::upper_bound(
        begin(groups),
        end(groups),
        angle,
        [](double angle_, const auto &group_) -> bool
        {
            return angle_ < group_.angle;
        });

    // The group iterator points to the first group for which
    // angle < group.angle is false.
    // It could also point to the end.

    // Check the group found by upper_bound
    if (group != end(groups))
    {
        // It is not the end, so it is safe to dereference.
        if (tau::CompareLineAngles(angle, group->angle, groupSeparation_deg))
        {
            group->AddLine(line);
            return;
        }
    }

    // Check the preceding group
    if (group != begin(groups))
    {
        // Check the previous group for a match
        --group;

        if (tau::CompareLineAngles(angle, group->angle, groupSeparation_deg))
        {
            group->AddLine(line);
            return;
        }

        // Restore group to the upper_bound for insertion of a new group.
        ++group;
    }

    // Insert a new group.
    groups.insert(group, LineGroup(line));
}


Groups FilterByAngle(const Groups &groups)
{
    Groups result;

    for (auto &group: groups)
    {
        auto localGroup = group;
        localGroup.RemoveOutlierLines();

        if (localGroup.lines.empty())
        {
            continue;
        }

        auto angles = localGroup.GetAngles();
        auto quartiles = tau::GetAngularQuartiles(angles);

        CHESS_LOG("Angular quartiles: ", quartiles);

        auto quartileRange = quartiles.Range();

        if (quartileRange > 3.0)
        {
            // The spread is sufficient to require that the lines are also
            // sorted by angle.

            bool isAscending = std::is_sorted(
                std::begin(angles),
                std::end(angles),
                [](double first, double second) -> bool
                {
                    return tau::LineAngleDifference(first, second) <= 0.0;
                });

            bool isDescending = std::is_sorted(
                std::begin(angles),
                std::end(angles),
                [](double first, double second) -> bool
                {
                    return tau::LineAngleDifference(first, second) >= 0.0;
                });

            if (isAscending || isDescending)
            {
                result.push_back(localGroup);
            }
            else
            {
                result.push_back(localGroup.FilterOnSorting());
            }
        }
        else
        {
            // Interquartile range is <= 3
            // Lines are nearly parallel.
            result.push_back(localGroup);
        }
    }

    return result;
}


Groups SplitGroupsOnAngles(const Groups &groups)
{
    Groups splitOnAngles;

    for (auto &group: groups)
    {
        auto split = group.SplitOnAngles();
        splitOnAngles.insert(begin(splitOnAngles), begin(split), end(split));
    }

    return splitOnAngles;
}


// Split the groups if there are any gaps larger than spacing limit.
Groups SplitGroupsOnSpacing(
    const Groups &groups,
    double maximumSpacing,
    double minimumSpacing)
{
    Groups splitOnSpacing;
    auto localGroups = groups;

    for (auto &group: localGroups)
    {
        // For this pass, we are not concerned with the orientation of the
        // sort, only that they are consistently sorted.
        group.SortByPosition(false);
        auto split = group.SplitOnSpacing(maximumSpacing, minimumSpacing);
        splitOnSpacing.insert(begin(splitOnSpacing), begin(split), end(split));
    }

    return splitOnSpacing;
}


Groups SplitMissingLines(const Groups &groups, double ratioLimit)
{
    Groups splitOnMissingLines;

    for (auto &group: groups)
    {
        auto split = group.SplitOnMissingLines(ratioLimit);

        splitOnMissingLines.insert(
            begin(splitOnMissingLines),
            begin(split),
            end(split));
    }

    return splitOnMissingLines;
}


void RemoveOutlierGroups(
    Groups &groups,
    size_t rowCount,
    size_t columnCount,
    size_t minimumLinesPerGroup)
{
    auto upperLimit = std::max(rowCount, columnCount);
    CHESS_LOG("upperLimit: ", upperLimit);
    CHESS_LOG("minimumLinesPerGroup: ", minimumLinesPerGroup);

    // Remove groups that are outside the allowable range.
    auto filteredEnd = std::remove_if(
        begin(groups),
        end(groups),
        [&] (const auto &group) -> bool
        {
            CHESS_LOG("considering group.lines.size(): ", group.lines.size());

            return (group.lines.size() < minimumLinesPerGroup)
                || (group.lines.size() > upperLimit);
        });

    if (filteredEnd != end(groups))
    {
        groups.erase(filteredEnd, end(groups));
    }
}


void SortDescending(
    Groups &groups,
    double verticalAngle)
{
    // Sort descending by number of lines and by how close the angle is to
    // 90 from the vertical
    groups.sort(
        [&](const LineGroup &first, const LineGroup &second) -> bool
        {
            auto sizeDifference = std::abs(
                static_cast<ssize_t>(first.lines.size())
                    - static_cast<ssize_t>(second.lines.size()));

            double firstAngleDifference = std::abs(
                tau::LineAngleDifference(first.angle, verticalAngle));

            double secondAngleDifference = std::abs(
                tau::LineAngleDifference(second.angle, verticalAngle));

            double firstAngle = std::abs(
                tau::LineAngleDifference(double{90}, firstAngleDifference));

            double secondAngle = std::abs(
                tau::LineAngleDifference(double{90}, secondAngleDifference));

            if (sizeDifference < 2)
            {
                // The difference in size is minimal.
                // Prefer the line closest to 90 degrees with the vertical.
                return tau::LineAngleDifference(firstAngle, secondAngle) < 0;
            }

            return first.lines.size() > second.lines.size();
        });
}


void CombineFirstAndLast(Groups &groups, double groupSeparation_deg)
{
    // Combine the first and last group if they are within the group
    // separation angular limit.
    if (groups.size() > 1)
    {
        // There are at least two groups.
        auto &front = groups.front();
        auto &back = groups.back();

        if (tau::CompareLineAngles(
                front.angle,
                back.angle,
                groupSeparation_deg))
        {
            // Combine the groups.
            for (auto &line: back.lines)
            {
                front.lines.push_back(line);
            }

            front.angle =
                tau::ToDegrees(
                    tau::GetAverageAngleRadians<double>(front.lines));

            // Remove back.
            groups.pop_back();
        }
    }
}


AxisGroups SelectAxisGroups(
    Groups &groups,
    double minimumSpacing,
    bool allowSkippedLines)
{
    if (groups.empty())
    {
        return {};
    }

    AxisGroups result;

    // Sort descending by number of lines.
    groups.sort(
        [](const LineGroup &first, const LineGroup &second) -> bool
        {
            return first.lines.size() > second.lines.size();
        });

    CHESS_LOG("sorted groups: ");

    for (auto &group: groups)
    {
        CHESS_LOG("  ", group.lines.size(), ", ", group.angle, " degrees");
    }

    // Choose the largest group to be "vertical"...
    auto group = begin(groups);
    result.vertical = *group;
    double verticalAngle = result.vertical.angle;
    ++group;

    if (group == end(groups))
    {
        return result;
    }

    Groups theRest(group, end(groups));
    SortDescending(theRest, verticalAngle);

    group = begin(theRest);

    while (group != end(theRest))
    {
        double difference = std::abs(
            tau::LineAngleDifference(group->angle, verticalAngle));

        if (difference > 50)
        {
            result.horizontal = *group;
            result.ComputeIndices(minimumSpacing, allowSkippedLines);
            return result;
        }

        ++group;
    }

    return result;
}


} // end namespace iris
