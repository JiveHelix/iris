#include "iris/chess/axis_groups.h"


namespace iris
{


void AxisGroups::ComputeIndices(double minimumSpacing, bool allowSkippedLines)
{
    if (
        this->vertical.lines.size() < 2
        || this->horizontal.lines.size() < 2)
    {
        return;
    }

    this->vertical.SortByPosition(false);
    this->horizontal.SortByPosition(true);

    this->vertical.ComputeIndices(
        false,
        minimumSpacing,
        this->horizontal.GetPerspective(),
        allowSkippedLines);

    this->horizontal.ComputeIndices(
        true,
        minimumSpacing,
        this->vertical.GetPerspective(),
        allowSkippedLines);
}

bool AxisGroups::CanFormVertices() const
{
    if (
        this->vertical.lines.empty()
        || this->horizontal.lines.empty())
    {
        return false;
    }

    // Previous stages may not have been able to guess the logical
    // indices.
    return
        !this->vertical.logicalIndices.empty()
        && !this->horizontal.logicalIndices.empty();
}


} // end namespace iris
