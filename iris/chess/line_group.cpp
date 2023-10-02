#include "iris/chess/line_group.h"


namespace iris
{


using Line = typename LineGroup::Line;


LineGroup::LineGroup()
    :
    angle{},
    lines{}
{

}


LineGroup::LineGroup(const Line &line)
    :
    angle(line.GetAngleDegrees()),
    lines({line})
{

}


LineGroup & LineGroup::Combine(const LineGroup &other)
{
    this->lines.insert(end(this->lines), begin(other.lines), end(other.lines));

    this->angle = tau::ToDegrees(
        tau::GetAverageAngleRadians<double>(this->lines));

    return *this;
}


void LineGroup::AddLine(const Line &line)
{
    this->lines.push_back(line);

    this->angle = tau::ToDegrees(
        tau::GetAverageAngleRadians<double>(this->lines));
}


size_t LineGroup::GetLogicalIndex(size_t lineIndex) const
{
    return this->logicalIndices.at(lineIndex);
}


Line LineGroup::GetPerpendicular(bool isHorizontal) const
{
    // Get perpendicular line
    // The range of values for this->angle is 0 to 180.
    double perpendicularAngle = this->angle + 90;

    if (isHorizontal)
    {
        // We want the perpendicularAngle to be close to 90, not -90.
        if (perpendicularAngle > 180)
        {
            perpendicularAngle -= 180;
        }
    }
    else
    {
        // We want the perpendicularAngle to be close to 0, not
        // 180
        if (perpendicularAngle > 90)
        {
            perpendicularAngle -= 180;
        }
    }

    auto perpendicularAngle_rad = tau::ToRadians(perpendicularAngle);

    return Line(
        tau::Point2d<double>(960.0, 540.0),
        tau::Vector2d<double>(
            std::cos(perpendicularAngle_rad),
            std::sin(perpendicularAngle_rad)));
}


void LineGroup::RemoveOutlierLines()
{
    // All lines in a group are required to have angles within
    // groupSeparationDegrees of each other. If most of the lines in a group
    // cluster around a particular value, we should discard outliers.
    // The Interquartile Range will be large for angles that are spread out,
    // and small for groups of lines that cluster around a particular value.
    using Eigen::Index;

    if (this->lines.size() < 4)
    {
        return;
    }

    auto quartiles = tau::GetAngularQuartiles(this->GetAngles());

    CHESS_LOG(
        "\n  lower: ",
        quartiles.lower,
        "\n  median: ",
        quartiles.median,
        "\n  upper: ",
        quartiles.upper);

    double upperLimit;
    double lowerLimit;

    if (quartiles.Range() <= 1)
    {
        // There are no "outliers" within 1.5 degrees of the median.
        lowerLimit = quartiles.ComputeLowerLimit(1.5, 1.0);
        upperLimit = quartiles.ComputeUpperLimit(1.5, 1.0);
    }
    else
    {
        // Use the interquartile range to create limits.
        upperLimit = quartiles.UpperLimit();
        lowerLimit = quartiles.LowerLimit();
    }

    CHESS_LOG("lowerLimit: ", lowerLimit);
    CHESS_LOG("upperLimit: ", upperLimit);

    auto result = std::remove_if(
        begin(this->lines),
        end(this->lines),
        [&] (const auto &line) -> bool
        {
            auto lineAngle = line.GetAngleDegrees();

            auto lowerDifference = tau::LineAngleDifference(
                lineAngle,
                lowerLimit);

            auto upperDifference = tau::LineAngleDifference(
                lineAngle,
                upperLimit);

            CHESS_LOG(
                "\n  lineAngle: ",
                lineAngle,
                "\n  lowerDifference: ",
                lowerDifference,
                "\n  upperDifference: ",
                upperDifference);

            return (lowerDifference < 0 || upperDifference > 0);
        });

    this->lines.erase(result, this->lines.end());
}


std::vector<LineGroup> LineGroup::SplitOnSorting()
{
    // Apply a hysteresis to allow small deviations from the dominant
    // sorting.
    double hysteresis = 1.0;

    using Eigen::Index;
    std::vector<LineGroup> result;

    if (this->lines.size() < 3)
    {
        result.push_back(*this);
        return result;
    }

    result.emplace_back(this->lines[0]);
    LineGroup *current = &result.back();

    bool isDescending =
        tau::LineAngleDifference(
            this->lines[0].GetAngleDegrees(),
            this->lines[1].GetAngleDegrees()) >= 0.0;

    bool sortingChanged = false;

    for (size_t i = 1; i < this->lines.size() - 1; ++i)
    {
        auto &first = this->lines[i];
        auto &second = this->lines[i + 1];

        auto difference = tau::LineAngleDifference(
            first.GetAngleDegrees(),
            second.GetAngleDegrees());

        sortingChanged = false;

        if (isDescending)
        {
            // Require the change to be large enough in the other
            // direction.
            if (difference < -hysteresis)
            {
                sortingChanged = true;
            }
        }
        else
        {
            // Require the change to be large enough in the other
            // direction.
            if (difference > hysteresis)
            {
                sortingChanged = true;
            }
        }

        if (sortingChanged)
        {
            // The sort order has changed.
            // Create a new group.
            result.emplace_back(first);
            current = &result.back();
            isDescending = !isDescending;
        }
        else
        {
            // The current sort order matches the previous.
            // Add first to the current group.
            current->AddLine(first);
        }
    }

    auto &last = this->lines.back();
    auto &penultimate = this->lines[this->lines.size() - 2];

    auto difference = tau::LineAngleDifference(
        penultimate.GetAngleDegrees(),
        last.GetAngleDegrees());

    sortingChanged = false;

    if (isDescending)
    {
        // Require the change to be large enough in the other
        // direction.
        if (difference < -hysteresis)
        {
            sortingChanged = true;
        }
    }
    else
    {
        // Require the change to be large enough in the other
        // direction.
        if (difference > hysteresis)
        {
            sortingChanged = true;
        }
    }

    if (sortingChanged)
    {
        result.emplace_back(last);
    }
    else
    {
        current->AddLine(last);
    }

    return result;
}

std::vector<LineGroup> LineGroup::SplitOnSpacing(
    double maximumSpacing,
    double minimumSpacing)
{
    using Eigen::Index;

    if (this->lines.empty())
    {
        return {};
    }

    std::vector<LineGroup> result;

    result.emplace_back(this->lines.at(0));

    LineGroup *current = &result.back();

    auto perpendicular = this->GetPerpendicular(false);

    for (size_t i = 0; i < this->lines.size() - 1; ++i)
    {
        const Line &first = this->lines[i];
        const Line &second = this->lines[i + 1];

        auto spacing = std::abs(
            perpendicular.DistanceToIntersection(second)
            - perpendicular.DistanceToIntersection(first));

        if (spacing < minimumSpacing)
        {
            // Skip closely-spaced lines.
            // These are effectively removed from the result.
            continue;
        }

        if (spacing < maximumSpacing)
        {
            current->AddLine(second);
        }
        else
        {
            result.emplace_back(second);
            current = &result.back();
        }
    }

    return result;
}

void LineGroup::SortByAngles()
{
    std::sort(
        begin(this->lines),
        end(this->lines),
        [&](const Line &first, const Line &second) -> bool
        {
            return (tau::LineAngleDifference(
                first.GetAngleDegrees(),
                second.GetAngleDegrees()) < 0);
        });
}

std::ostream & LineGroup::ToStream(std::ostream &outputStream) const
{
    outputStream << "\nGroup "
        << ", lines: " << this->lines.size()
        << ", angle: " << this->angle << '\n';

    for (auto &line: this->lines)
    {
        outputStream << line << " " << line.GetAngleDegrees() << " deg\n";
    }

    return outputStream;
}

std::vector<LineGroup> LineGroup::SplitOnAngles() const
{
    using Eigen::Index;

    if (this->lines.empty())
    {
        return {};
    }

    LineGroup sorted = *this;
    sorted.SortByAngles();

    std::vector<LineGroup> tightGroups;
    tightGroups.emplace_back(sorted.lines.at(0));
    LineGroup *current = &tightGroups.back();

    static const double angleThreshold = 1.0;

    // Form groups of lines that fall within 1 degree of one another.
    for (size_t i = 1; i < sorted.lines.size(); ++i)
    {
        const Line &line = sorted.lines[i];

        bool anglesAreClose =
            tau::CompareLineAngles(
                line.GetAngleDegrees(),
                current->angle,
                angleThreshold);

        if (anglesAreClose)
        {
            current->AddLine(line);
        }
        else
        {
            tightGroups.emplace_back(line);
            current = &tightGroups.back();
        }
    }

    // Recombine tight groups that are not sufficiently large.
    std::vector<LineGroup> soloGroups;
    LineGroup theRest;

    static const size_t soloThreshold = 4;

    for (auto &group: tightGroups)
    {
        if (group.lines.size() >= soloThreshold)
        {
            soloGroups.push_back(group);
        }
        else
        {
            theRest.Combine(group);
        }
    }

    soloGroups.push_back(theRest);

    return soloGroups;
}


std::vector<LineGroup> LineGroup::SplitOnMissingLines(double ratioLimit) const
{
    using Eigen::Index;

    if (this->lines.size() < 4)
    {
        // Not enough spaces to guess where the missing lines are.
        return {*this};
    }

    LineGroup sorted = *this;

    // Orientation of the sort is not important.
    sorted.SortByPosition(false);
    auto spacings = sorted.MakeSpacings(false);

    CHESS_LOG("spacings:");

#ifdef ENABLE_CHESS_LOG
    for (auto spacing: spacings)
    {
        CHESS_LOG("\n  ", spacing);
    }
#endif

    Eigen::VectorX<double> ratios(Index(spacings.size() - 1));

    // CHESS_LOG("ratios:");

    for (size_t i = 0; i < spacings.size() - 1; ++i)
    {
        ratios(Index(i)) = spacings[i + 1] / spacings[i];
        // CHESS_LOG("\n  ", ratios(Index(i)));
    }

    double median = tau::Percentile(ratios, 0.5);

    // CHESS_LOG("median ratio: ", median);

    std::vector<LineGroup> result;

    result.emplace_back(this->lines.at(0));

    LineGroup *current = &result.back();

    for (size_t i = 1; i < this->lines.size() - 1; ++i)
    {
        double ratio = ratios(Index(i - 1));

        if (std::abs(ratio - median) > ratioLimit)
        {
            // The change in spacing is too large.
            result.emplace_back(this->lines[i]);
            current = &result.back();
            CHESS_LOG("Splitting on ratio: ", ratio);
        }
        else
        {
            current->AddLine(this->lines[i]);
        }
    }

    // The penultimate line was already considered. If the spacing ratio
    // between it and the last line was outside of the allowed error, it
    // was already split into its own group.
    // The last two lines automatically go in the same group.
    current->AddLine(this->lines.back());

    return result;
}

Eigen::VectorX<double> LineGroup::GetAngles() const
{
    using Eigen::Index;

    Eigen::VectorX<double> angles(this->lines.size());

    for (Index i = 0; i < static_cast<Index>(this->lines.size()); ++i)
    {
        angles(i) = this->lines[static_cast<size_t>(i)].GetAngleDegrees();
    }

    return angles;
}


double LineGroup::GetPerspective() const
{
    if (this->lines.size() < 2)
    {
        throw ChessError("At least two lines are required.");
    }

    auto angles = this->GetAngles();
    return tau::LineAngleDifference(angles(0), angles(angles.size() - 1));
}


std::vector<double> LineGroup::MakeSpacings(bool isHorizontal) const
{
    auto perpendicular = this->GetPerpendicular(isHorizontal);

    auto result = std::vector<double>(this->lines.size() - 1);

    for (size_t i = 0; i < this->lines.size() - 1; ++i)
    {
        const Line &first = this->lines[i];
        const Line &second = this->lines[i + 1];

        auto spacing =
            perpendicular.DistanceToIntersection(second)
            - perpendicular.DistanceToIntersection(first);

        result[i] = spacing;
    }

    return result;
}


void LineGroup::ComputeIndices(
    bool isHorizontal,
    double minimumSpacing,
    double perspective,
    bool allowSkippedLines)
{
    this->logicalIndices = std::vector<size_t>();

    if (this->lines.size() < 2)
    {
        // Not enough lines to sort.
        return;
    }

    if (!allowSkippedLines)
    {
        for (size_t i = 0; i < this->lines.size(); ++i)
        {
            this->logicalIndices.push_back(i);
        }

        return;
    }

    // About how much each square grows because of perspective projection.
    double perspectiveFactor =
        1 + std::abs(std::tan(tau::ToRadians(perspective / 2)));

    auto spacings = this->MakeSpacings(isHorizontal);

    Eigen::Map<Eigen::VectorX<double>> spacings_(
        spacings.data(),
        spacings.size());

    auto smallestSpacing = spacings_.minCoeff();
    this->logicalIndices = std::vector<size_t>();

    if (smallestSpacing < minimumSpacing)
    {
        // Unable to safely guess the logical indices.
        std::cout << "Info: minimum spacing ("
            << smallestSpacing
            << ") is less than line separation ("
            << minimumSpacing << ")" << std::endl;

        for (auto &spacing: spacings)
        {
            std::cout << spacing << std::endl;
        }

        throw ChessError("Closely-spaced lines should have been removed");
    }

    CHESS_LOG("minimumSpacing: ", minimumSpacing);

    auto MakeIndices =
        [this, perspectiveFactor, smallestSpacing](auto spacing, auto end)
    {
        size_t logicalIndex = 0;

        this->logicalIndices.push_back(logicalIndex);
        CHESS_LOG("perspectiveFactor: ", perspectiveFactor);

        while (spacing != end)
        {
            CHESS_LOG("logicalIndex: ", logicalIndex);

            auto expectedSpacing =
                smallestSpacing
                * std::pow(perspectiveFactor, double(logicalIndex));

            CHESS_LOG("expectedSpacing: ", expectedSpacing);

            CHESS_LOG("spacing: ", *spacing);

            auto step = *spacing / expectedSpacing;

            logicalIndex += static_cast<size_t>(
                std::round(*spacing / expectedSpacing));

            CHESS_LOG("logicalIndex: ", logicalIndex);

            this->logicalIndices.push_back(logicalIndex);

            ++spacing;
        }
    };

    if (isHorizontal)
    {
        if (perspective > 0)
        {
            // The small square is on the top, matching the sort order.
            MakeIndices(spacings.begin(), spacings.end());
        }
        else
        {
            // The spacings are sorted from top to bottom, but the small
            // square is on the bottom.
            // Reverse the order of the spacings so you can estimate
            // whether a row has been skipped.
            MakeIndices(spacings.rbegin(), spacings.rend());

            // The lines were processed in reverse order.
            std::reverse(
                this->logicalIndices.begin(),
                this->logicalIndices.end());
        }
    }
    else
    {
        // vertical
        if (perspective > 0)
        {
            // The spacings are sorted from left to right, but the small
            // square is on the right.
            // Reverse the order of the spacings so you can estimate
            // whether a column has been skipped.
            MakeIndices(spacings.rbegin(), spacings.rend());

            // The lines were processed in reverse order.
            std::reverse(
                this->logicalIndices.begin(),
                this->logicalIndices.end());
        }
        else
        {
            // The small square is on the left, matching the sort order.
            MakeIndices(spacings.begin(), spacings.end());
        }
    }
}


void LineGroup::SortByPosition(bool isHorizontal)
{
    if (this->lines.size() < 2)
    {
        // Not enough lines to sort.
        return;
    }

    auto perpendicular = this->GetPerpendicular(isHorizontal);

    // Sort the lines by their position along the intersecting line.
    std::sort(
        begin(this->lines),
        end(this->lines),
        [&](const Line &first, const Line &second) -> bool
        {
            return perpendicular.DistanceToIntersection(first)
                < perpendicular.DistanceToIntersection(second);
        });
}


std::ostream & operator<<(
    std::ostream &outputStream,
    const std::list<LineGroup> &groups)
{
    size_t count = 0;

    for (auto &group: groups)
    {
        group.ToStream(outputStream << ++count << ": ") << std::endl;
    }

    return outputStream;
}


std::ostream & operator<<(
    std::ostream &outputStream,
    const std::vector<LineGroup> groups)
{
    size_t count = 0;

    for (auto &group: groups)
    {
        group.ToStream(outputStream << ++count << ": ") << std::endl;
    }

    return outputStream;
}


} // end namespace iris
