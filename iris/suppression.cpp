#include "iris/suppression.h"
#include "iris/detail/suppression_detail.h"


namespace iris
{


Suppression::Suppression(const SuppressionSettings &settings)
    :
    settings_(settings),
    windowSize_(settings.window)
{
    assert(settings.count < settings.window * settings.window);
    assert(settings.count > 0);
}


tau::ImageMatrixFloat Suppression::Filter(const tau::ImageMatrixFloat &input)
{
    using Eigen::Index;

    tau::ImageMatrixFloat result = input;

    Index rowCount = input.rows();
    Index columnCount = input.cols();

    Index limitRow = rowCount - this->windowSize_ + 1;
    Index limitColumn = columnCount - this->windowSize_ + 1;

    if (this->settings_.count == 1)
    {
        detail::SelectOne(limitRow, limitColumn, this->windowSize_, result);
        return result;
    }

    using Float = typename tau::ImageMatrixFloat::Scalar;
    std::vector<detail::Detection<Float>> detections;

    detections.reserve(
        static_cast<size_t>(this->windowSize_ * this->windowSize_));

    if constexpr (tau::MatrixTraits<tau::ImageMatrixFloat>::isRowMajor)
    {
        for (Index row = 0; row < limitRow; ++row)
        {
            for (Index column = 0; column < limitColumn; ++column)
            {
                detail::FilterWindowedDetections(
                    result,
                    detections,
                    this->windowSize_,
                    this->settings_.count,
                    row,
                    column);
            }
        }
    }
    else
    {
        // Iterate in column-major order
        for (Index column = 0; column < limitColumn; ++column)
        {
            for (Index row = 0; row < limitRow; ++row)
            {
                detail::FilterWindowedDetections(
                    result,
                    detections,
                    this->windowSize_,
                    this->settings_.count,
                    row,
                    column);
            }
        }
    }

    return result;
}


} // end namespace iris
