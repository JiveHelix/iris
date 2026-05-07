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


class Homography
{
public:
    using Intrinsics = Eigen::Matrix<double, 3, 3>;

    Homography(const HomographySettings &settings);

    Eigen::Matrix<double, 2, 9>
    GetHomographyFactors(const NamedVertex &vertext);

    using Factors = Eigen::Matrix<double, Eigen::Dynamic, 9>;

    Factors CombineHomographyFactors(const NamedVertices &vertices);

    HomographyMatrix GetHomographyMatrix(const NamedVertices &vertices);

    Intrinsics ComputeIntrinsics(
        const std::vector<ChessSolution> &chessSolutions);

    Distortion<double> ComputeDistortion(
        const Intrinsics &intrinsics,
        const std::vector<ChessSolution> &chessSolutions);

private:
    World world_;
    tau::NormalizePixel normalize_;
};


} // end namespace iris
