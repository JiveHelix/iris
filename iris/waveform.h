#pragma once


#include <tau/eigen.h>
#include "iris/waveform_settings.h"


namespace iris
{


using Waveform =
    Eigen::Matrix<uint16_t, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;


using ColoredWaveform = Eigen::Matrix<uint8_t, Eigen::Dynamic, 3>;


template<typename T>
using Data = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>;


Waveform Resize(
    const Waveform &source,
    const Size &displaySize,
    double verticalScale);


template<typename Matrix>
Waveform DoGenerateWaveform(
    const Eigen::MatrixBase<Matrix> &data,
    Eigen::Index maximumValue,
    Eigen::Index levelCount,
    Eigen::Index columnCount)
{
    auto columnDivisor = static_cast<float>(data.cols())
        / static_cast<float>(columnCount);

    auto valueDivisor = static_cast<float>(maximumValue)
        / static_cast<float>(levelCount);

    auto maximum = levelCount - 1;

    Waveform result = Waveform::Zero(levelCount, columnCount);

    using Type = typename tau::MatrixTraits<Matrix>::type;

    Matrix scaled =
        (data.template cast<float>().array() / valueDivisor)
            .template cast<Type>();

    for (Eigen::Index row = 0; row < data.rows(); ++row)
    {
        for (Eigen::Index column = 0; column < data.cols(); ++column)
        {
            // (0, 0) is the top left
            // Value 0 must go in the last row, and maximum in row zero.
            auto value = tau::Index(scaled(row, column));

            result(
                maximum - value,
                tau::Index(float(column) / columnDivisor)) += 1;
        }
    }

    return result;
}


} // end namespace iris
