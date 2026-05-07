#pragma once


#include <tau/normalize_pixel.h>
#include "iris/chess.h"
#include "iris/homography_settings.h"


namespace iris
{


template<typename T>
struct DistortionFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::k1, "k1"),
        fields::Field(&T::k2, "k2"),
        fields::Field(&T::p1, "p1"),
        fields::Field(&T::p2, "p2"),
        fields::Field(&T::k3, "k3"));
};


template<typename Float>
struct DistortionTemplate
{
    template<template<typename> typename T>
    struct Template
    {
        T<Float> k1;
        T<Float> k2;
        T<Float> p1;
        T<Float> p2;
        T<Float> k3;

        static constexpr auto fields =
            DistortionFields<Template>::fields;

        static constexpr auto fieldsTypeName = "Distortion";
    };
};


template<typename T>
using DistortionGroup =
    pex::Group
    <
        DistortionFields,
        DistortionTemplate<T>::template Template
    >;

template<typename T>
using Distortion = typename DistortionGroup<T>::Plain;

template<typename T>
using DistortionModel = typename DistortionGroup<T>::Model;

template<typename T>
using DistortionControl = typename DistortionGroup<T>::DefaultControl;


DECLARE_OUTPUT_STREAM_OPERATOR(Distortion<float>)
DECLARE_OUTPUT_STREAM_OPERATOR(Distortion<double>)
DECLARE_EQUALITY_OPERATORS(Distortion<float>)
DECLARE_EQUALITY_OPERATORS(Distortion<double>)


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
        return logical.template Cast<double>() * this->chessSquareSize_m_;
    }

private:
    double chessSquareSize_m_;
};


template<typename T>
tau::Point2d<double> DistortPoint(
    const T &parameters,
    const Eigen::Vector3<double> &point)
{
    double x = point(0);
    double y = point(1);

    double xPow2 = x * x;
    double yPow2 = y * y;

    double radiusPow2 = xPow2 + yPow2;
    double radiusPow4 = radiusPow2 * radiusPow2;
    double radiusPow6 = radiusPow4 * radiusPow2;

    double radialDistortion =
        1.0
        + parameters.k1 * radiusPow2
        + parameters.k2 * radiusPow4
        + parameters.k3 * radiusPow6;

    double xy = x * y;

    tau::Point2d<double> result;

    result.x =
        x * radialDistortion
        + 2.0 * parameters.p1 * xy
        + parameters.p2 * (radiusPow2 + 2.0 * xPow2);

    result.y =
        y * radialDistortion
        + parameters.p1 * (radiusPow2 + 2.0 * yPow2)
        + 2.0 * parameters.p2 * xy;

    return result;
}


using IntrinsicsMatrix = Eigen::Matrix<double, 3, 3>;


struct ReprojectionErrorMinimum
{
    IntrinsicsMatrix intrinsics;
    Distortion<double> distortion;
    Eigen::Vector<double, Eigen::Dynamic> residuals_pixels;
    double rmsResidual_pixels;
};


class Homography
{
public:
    Homography(const HomographySettings &settings);

    Eigen::Matrix<double, 2, 9>
    GetHomographyFactors(const NamedVertex &vertext);

    using Factors = Eigen::Matrix<double, Eigen::Dynamic, 9>;

    Factors CombineHomographyFactors(const NamedVertices &vertices);

    HomographyMatrix GetHomographyMatrix(const NamedVertices &vertices);

    IntrinsicsMatrix ComputeIntrinsics(
        const std::vector<ChessSolution> &chessSolutions);

    ReprojectionErrorMinimum MinimizeReprojectionError(
        const IntrinsicsMatrix &intrinsics,
        const std::vector<ChessSolution> &chessSolutions);

private:
    World world_;
    tau::Size<double> sensorSize_;
    tau::NormalizePixel normalize_;
};


} // end namespace iris
