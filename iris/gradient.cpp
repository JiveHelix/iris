#include "iris/gradient.h"


namespace iris
{


int32_t DetectGradientScale(
    const GradientResult<int32_t> &result,
    double percentile)
{
    auto magnitude = result.template GetPhasor<double>().magnitude;

    // Find the percentile value while ignoring the zeros.
    // Scale the percentile to 0.9.
    auto filtered = tau::RemoveZeros(magnitude);

    if (filtered.size() < 10)
    {
        std::cerr << "gradient has insufficient non-zero values." << std::endl;
        return 1;
    }

    auto percentileValue = tau::Percentile(filtered, percentile);
    auto factor = 0.9 / percentileValue;

    auto scale = static_cast<int32_t>(std::round(factor));

    std::sort(
        std::begin(filtered),
        std::end(filtered));

    return scale;
}


template struct iris::GradientResult<int32_t>;
template class iris::Gradient<int32_t>;
template class GradientNode<DefaultGaussianNode>;


} // end namespace iris
