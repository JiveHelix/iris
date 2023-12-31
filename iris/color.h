#pragma once

#include <tau/color_map.h>
#include <tau/color_maps/turbo.h>
#include <tau/color_maps/gray.h>

#include "iris/image.h"
#include "draw/pixels.h"
#include "iris/color_settings.h"
#include "iris/threadsafe_filter.h"


namespace iris
{


template<typename T, typename Value>
auto MakeColorMap(const ColorSettings<Value> &colorSettings)
{
    auto low = colorSettings.range.low;
    auto high = colorSettings.range.high;

    static constexpr auto maximum =
        static_cast<decltype(low)>(std::numeric_limits<T>::max());

    high = std::min(maximum, high);
    low = std::min(high - 1, low);

    assert(low < high);

    size_t count = static_cast<size_t>(1 + high - low);

    if (colorSettings.turbo)
    {
        return tau::LimitedColorMap<draw::PixelMatrix, T>(
            tau::turbo::MakeRgb8(count),
            static_cast<T>(low),
            static_cast<T>(high));
    }
    else
    {
        return tau::LimitedColorMap<draw::PixelMatrix, T>(
            tau::gray::MakeRgb8(count),
            static_cast<T>(low),
            static_cast<T>(high));
    }
}


template<typename Value>
class Color
{
public:
    using Matrix = ImageMatrix<Value>;

    Color(const ColorSettings<Value> &colorSettings)
        :
        colorMap_(MakeColorMap<Value>(colorSettings))
    {

    }

    draw::Pixels Filter(const Matrix &data) const
    {
        draw::Pixels result{{}, {data.cols(), data.rows()}};
        this->colorMap_(data, &result.data);

        return result;
    }

protected:
    tau::LimitedColorMap<draw::PixelMatrix, Value> colorMap_;
};


template<typename Value>
using ThreadsafeColor =
    ThreadsafeFilter<ColorGroup<Value>, Color<Value>>;


extern template class Color<int32_t>;
extern template class ThreadsafeFilter<ColorGroup<int32_t>, Color<int32_t>>;


} // end namespace iris
