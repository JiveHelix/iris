#include "line_collector.h"


namespace iris
{


LineCollector::LineCollector(const ChessSettings &settings)
    :
    cornerLines{},
    maximumPointError(settings.pointsChess.maximumPointError),
    angleToleranceDegrees(settings.pointsChess.angleToleranceDegrees),
    minimumSpacing(settings.minimumSpacing),
    minimumPointsPerLine(settings.pointsChess.minimumPointsPerLine),
    angleFilterLow(settings.angleFilter.low),
    angleFilterHigh(settings.angleFilter.high)
{

}


void LineCollector::AddToLines(
    const tau::Point2d<double> &firstPoint,
    const tau::Point2d<double> &secondPoint)
{
    // Add points to any existing lines that are below the threshold.
    bool exists = false;

    LineFromPoints candidateLine(
        this->angleToleranceDegrees,
        this->minimumSpacing,
        firstPoint,
        secondPoint);

    for (auto &line: this->cornerLines)
    {
        if (line.GetError(firstPoint) <= this->maximumPointError)
        {
            line.AddPoint(firstPoint);
        }

        if (line.GetError(secondPoint) <= this->maximumPointError)
        {
            line.AddPoint(secondPoint);
        }

        if (line == candidateLine)
        {
            exists = true;
        }
    }

    if (!exists)
    {
        // The candidate line is not colinear with any of the other lines.
        // Add it to the collection.
        this->cornerLines.push_back(candidateLine);
    }
};


// As lines are constructed, only the error from the newest point is
// considered. The updated average line can be pulled away from earlier
// points, leaving them as outliers that have too much point error.
void LineCollector::RemoveOutliers()
{
    for (auto &line: this->cornerLines)
    {
        line.RemoveOutliers(this->maximumPointError);
    }
}


// Apply angle and minimum points filters, and remove duplicates.
void LineCollector::Filter()
{
    auto linesEnd = std::remove_if(
        begin(this->cornerLines),
        end(this->cornerLines),
        [this] (const auto &line) -> bool
        {
            return (line.GetPointCount() < this->minimumPointsPerLine)
                || (line.GetAngleDegrees() < this->angleFilterLow)
                || (line.GetAngleDegrees() > this->angleFilterHigh);
        });

    if (linesEnd != end(this->cornerLines))
    {
        this->cornerLines.erase(linesEnd, end(this->cornerLines));
    }

    // As points are added to lines, the lines shift slightly to fit the
    // new points. This leads to duplicate lines.
    std::sort(begin(this->cornerLines), end(this->cornerLines));

    // Remove duplicate lines.
    LineCollection filtered{};

    auto line = begin(this->cornerLines);

    while (line != end(this->cornerLines))
    {
        auto adjacent = std::adjacent_find(line, end(this->cornerLines));
        filtered.insert(end(filtered), line, adjacent);

        if (adjacent == end(this->cornerLines))
        {
            // There were no duplicates.
            break;
        }

        auto duplicate = adjacent;

        while (++duplicate != end(this->cornerLines))
        {
            // Search up to the end for duplicates
            if (*adjacent != *duplicate)
            {
                // duplicate points to the next line greater than adjacent.
                break;
            }

            // duplicate is actually a duplicate.
            // Combine the points into one line.
            adjacent->Combine(*duplicate);
        }

        assert(adjacent != end(this->cornerLines));
        filtered.push_back(*adjacent);
        line = duplicate;
    }

    this->cornerLines = filtered;
}


LineCollector::LineCollection
LineCollector::FormLines(const iris::CornerPoints &corners)
{
    assert(!corners.empty());

    for (size_t i = 0; i < corners.size() - 1; ++i)
    {
        const auto &firstCorner = corners[i];

        for (size_t j = i + 1; j < corners.size(); ++j)
        {
            const auto &secondCorner = corners[j];
            this->AddToLines(firstCorner.point, secondCorner.point);
        }
    }

    this->RemoveOutliers();
    this->Filter();

    return this->cornerLines;
}


} // end namespace iris
