#include "line_collector.h"


namespace iris
{


LineCollector::LineCollector(const ChessSettings &settings)
    :
    vertexLines{},
    maximumPointError(settings.vertexChess.maximumPointError),
    angleToleranceDegrees(settings.vertexChess.angleToleranceDegrees),
    minimumSpacing(settings.minimumSpacing),
    minimumPointsPerLine(settings.vertexChess.minimumPointsPerLine),
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

    for (auto &line: this->vertexLines)
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
        this->vertexLines.push_back(candidateLine);
    }
};


// As lines are constructed, only the error from the newest point is
// considered. The updated average line can be pulled away from earlier
// points, leaving them as outliers that have too much point error.
void LineCollector::RemoveOutliers()
{
    for (auto &line: this->vertexLines)
    {
        line.RemoveOutliers(this->maximumPointError);
    }
}


// Apply angle and minimum points filters, and remove duplicates.
void LineCollector::Filter()
{
    auto linesEnd = std::remove_if(
        begin(this->vertexLines),
        end(this->vertexLines),
        [this] (const auto &line) -> bool
        {
            return (line.GetPointCount() < this->minimumPointsPerLine)
                || (line.GetAngleDegrees() < this->angleFilterLow)
                || (line.GetAngleDegrees() > this->angleFilterHigh);
        });

    if (linesEnd != end(this->vertexLines))
    {
        this->vertexLines.erase(linesEnd, end(this->vertexLines));
    }

    // As points are added to lines, the lines shift slightly to fit the
    // new points. This leads to duplicate lines.
    std::sort(begin(this->vertexLines), end(this->vertexLines));

    // Remove duplicate lines.
    LineCollection filtered{};

    auto line = begin(this->vertexLines);

    while (line != end(this->vertexLines))
    {
        auto adjacent = std::adjacent_find(line, end(this->vertexLines));
        filtered.insert(end(filtered), line, adjacent);

        if (adjacent == end(this->vertexLines))
        {
            // There were no duplicates.
            break;
        }

        auto duplicate = adjacent;

        while (++duplicate != end(this->vertexLines))
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

        assert(adjacent != end(this->vertexLines));
        filtered.push_back(*adjacent);
        line = duplicate;
    }

    this->vertexLines = filtered;
}


LineCollector::LineCollection
LineCollector::FormLines(const iris::Vertices &vertices)
{
    assert(!vertices.empty());

    for (size_t i = 0; i < vertices.size() - 1; ++i)
    {
        const auto &firstVertex = vertices[i];

        for (size_t j = i + 1; j < vertices.size(); ++j)
        {
            const auto &secondVertex = vertices[j];
            this->AddToLines(firstVertex.point, secondVertex.point);
        }
    }

    this->RemoveOutliers();
    this->Filter();

    return this->vertexLines;
}


} // end namespace iris
