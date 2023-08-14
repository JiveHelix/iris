#pragma once


#include <vector>
#include <tau/vector2d.h>
#include "iris/chess_settings.h"
#include "iris/corner.h"
#include "iris/chess/line_from_points.h"


namespace iris
{


class LineCollector
{
public:
    using LineCollection = std::vector<LineFromPoints>;

    LineCollection cornerLines;
    double maximumPointError;
    double angleToleranceDegrees;
    double minimumSpacing;

    size_t minimumPointsPerLine;
    double angleFilterLow;
    double angleFilterHigh;

    LineCollector(const ChessSettings &settings);

    void AddToLines(
        const tau::Point2d<double> &firstPoint,
        const tau::Point2d<double> &secondPoint);

    // As lines are constructed, only the error from the newest point is
    // considered. The updated average line can be pulled away from earlier
    // points, leaving them as outliers that have too much point error.
    void RemoveOutliers();

    // Apply angle and minimum points filters.
    void Filter();

    LineCollection FormLines(const iris::CornerPoints &corners);
};


} // end namespace iris
