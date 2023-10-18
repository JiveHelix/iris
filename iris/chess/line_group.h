#pragma once


#include <list>
#include <vector>
#include <iostream>
#include <tau/vector2d.h>
#include <tau/angular.h>
#include <tau/line2d.h>
#include <tau/percentile.h>

#include "iris/chess/chess_log.h"
#include "iris/chess/chess_error.h"


namespace iris
{


struct LineGroup
{
    using Line = tau::Line2d<double>;
    using LineCollection = std::vector<Line>;

    double angle;
    LineCollection lines;
    std::vector<size_t> logicalIndices;

    LineGroup();

    LineGroup(const Line &line);

    LineGroup & Combine(const LineGroup &other);

    void AddLine(const Line &line);

    size_t GetLogicalIndex(size_t lineIndex) const;

    Line GetPerpendicular(bool isHorizontal) const;

    void RemoveOutlierLines();

    LineGroup FilterOnSorting();

    std::vector<LineGroup> SplitOnSpacing(
        double maximumSpacing,
        double minimumSpacing);

    void SortByAngles();

    std::ostream & ToStream(std::ostream &outputStream) const;

    std::vector<LineGroup> SplitOnAngles() const;

    std::vector<LineGroup> SplitOnMissingLines(double ratioLimit) const;

    Eigen::VectorX<double> GetAngles() const;

    double GetPerspective() const;

    std::vector<double> MakeSpacings(bool isHorizontal) const;

    void ComputeIndices(
        bool isHorizontal,
        double minimumSpacing,
        double perspective,
        bool allowSkippedLines);

    void SortByPosition(bool isHorizontal);
};


std::ostream & operator<<(
    std::ostream &outputStream,
    const LineGroup &group);


std::ostream & operator<<(
    std::ostream &outputStream,
    const std::list<LineGroup> &groups);


std::ostream & operator<<(
    std::ostream &outputStream,
    const std::vector<LineGroup> groups);


} // end namespace iris
