#pragma once

#include <tau/color_map.h>
#include <tau/color_maps/turbo.h>
#include <tau/color_maps/gray.h>

#include "iris/pixels.h"
#include "iris/color_settings.h"
#include "iris/threadsafe_filter.h"


namespace iris
{


template<typename T, typename Value>
auto MakeColorMap(const ColorSettings<Value> &colorSettings)
{
    auto low = colorSettings.level.low;
    auto high = colorSettings.level.high;

    static constexpr auto maximum =
        static_cast<decltype(low)>(std::numeric_limits<T>::max());

    high = std::min(maximum, high);
    low = std::min(high - 1, low);

    assert(low < high);

    size_t count = static_cast<size_t>(1 + high - low);

    if (colorSettings.turbo)
    {
        return tau::LimitedColorMap<PixelMatrix, T>(
            tau::turbo::MakeRgb8(count),
            static_cast<T>(low),
            static_cast<T>(high));
    }
    else
    {
        return tau::LimitedColorMap<PixelMatrix, T>(
            tau::gray::MakeRgb8(count),
            static_cast<T>(low),
            static_cast<T>(high));
    }
}


template<typename Value>
class Color: public ColorSettings<Value>
{
public:
    Color(const ColorSettings<Value> &colorSettings)
        :
        colorMap_(MakeColorMap<Value>(colorSettings))
    {

    }

    template<typename Data>
    Pixels Filter(const Eigen::MatrixBase<Data> &data) const
    {
        Pixels result{{}, {data.cols(), data.rows()}};
        this->colorMap_(data, &result.data);

        return result;
    }

protected:
    tau::LimitedColorMap<PixelMatrix, Value> colorMap_;
};


template<typename Value>
using ThreadsafeColor = ThreadsafeFilter<ColorGroup<Value>, Color<Value>>;


} // end namespace iris
