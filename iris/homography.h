#pragma once


#include <tau/size.h>
#include <tau/vector2d.h>
#include "iris/chess.h"
#include "iris/homography_settings.h"


namespace iris
{


class Normalize
{
public:
    Normalize(const tau::Size<double> &sensorSize)
        :
        sensorSize_(sensorSize)
    {

    }

    tau::Point2d<double> operator()(const tau::Point2d<double> &pixel) const
    {
        return this->ToNormalized(pixel);
    }

    tau::Point2d<double> ToNormalized(const tau::Point2d<double> &pixel) const
    {
#if 0
        return pixel;
#else
        // Scale from 0 to 2.
        tau::Point2d<double> result = pixel * 2 / this->sensorSize_;

        // Shift center point to 0.
        return result - 1;
#endif
    }

    tau::Point2d<double> ToPixel(const tau::Point2d<double> &normalized) const
    {
        tau::Point2d<double> unshifted = normalized;

        // Shift center point back to 1, 1
        unshifted += 1;

        // Scale back to sensor size.
        return unshifted * this->sensorSize_ / 2.0;
    }

    double ToPixel(double normalized, bool isX) const
    {
        double unshifted = normalized;

        // Shift center point back to 1, 1
        unshifted += 1;

        // Scale back to sensor size.
        if (isX)
        {
            return unshifted * this->sensorSize_.width / 2.0;
        }
        else
        {
            return unshifted * this->sensorSize_.height / 2.0;
        }
    }

    double Unscale(double normalized, bool isX) const
    {
        if (isX)
        {
            return normalized * this->sensorSize_.width / 2.0;
        }
        else
        {
            return normalized * this->sensorSize_.height / 2.0;
        }
    }

private:
    tau::Size<double> sensorSize_;
};


using HomographyMatrix = Eigen::Matrix<double, 3, 3>;
using ConstrainedElements = Eigen::RowVector<double, 6>;
using ConstrainedFactors = Eigen::Matrix<double, 2, 6>;


ConstrainedElements GetConstrainedElements(
    const HomographyMatrix &homography,
    Eigen::Index i,
    Eigen::Index j);


ConstrainedFactors GetConstrainedFactors(const HomographyMatrix &homography);


class World
{
public:
    static constexpr double metersPerMillimeter = 1e-3;

    World(double chessSquareSize_mm)
        :
        chessSquareSize_m_(chessSquareSize_mm * metersPerMillimeter)
    {

    }

    tau::Point2d<double> operator()(const tau::Point2d<size_t> &logical) const
    {
        return logical.template Convert<double>() * this->chessSquareSize_m_;
    }

private:
    double chessSquareSize_m_;
};


class Homography
{
public:
    using Intrinsics = Eigen::Matrix<double, 3, 3>;

    Homography(const HomographySettings &settings);

    Eigen::Matrix<double, 2, 9>
    GetHomographyFactors(const Intersection & intersection);

    using Factors = Eigen::Matrix<double, Eigen::Dynamic, 9>;

    Factors CombineHomographyFactors(
        const std::vector<Intersection> &intersections);

    HomographyMatrix GetHomographyMatrix(
        const std::vector<Intersection> &intersections);

    Intrinsics ComputeIntrinsics(
        const std::vector<ChessSolution> &chessSolutions);

private:
    World world_;
    Normalize normalize_;
};


} // end namespace iris
