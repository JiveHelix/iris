#pragma once


#include "iris/chess/line_group.h"


namespace iris
{


struct AxisGroups
{
    LineGroup vertical;
    LineGroup horizontal;

    void ComputeIndices(double minimumSpacing, bool allowSkippedLines);
    bool CanFormVertices() const;
};


} // end namespace iris
