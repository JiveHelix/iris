#pragma once

#include <tau/color_map.h>

#include "level_settings.h"
#include "iris/threadsafe_filter.h"


namespace iris
{


template<typename Value>
class LevelAdjust
{
public:
    static tau::FloatRescale<Value, float> MakeRescale(
        Value maximum,
        Value low,
        Value high)
    {
        low = std::min(high - 1, low);
        assert(low < high);

        return tau::FloatRescale<Value, float>(maximum + 1, low, high);
    }

    LevelAdjust(const LevelSettings<Value> &settings)
        :
        enable_(settings.enable),
        rescale_(
            MakeRescale(
                settings.maximum,
                settings.range.low,
                settings.range.high))
    {

    }

    template<typename Derived>
    std::optional<Derived> Filter(const Eigen::MatrixBase<Derived> &data)
    {
        if (!this->enable_)
        {
            return {};
        }

        return this->rescale_(data);
    }

protected:
    bool enable_;
    tau::FloatRescale<Value, float> rescale_;
};


template<typename Value>
using ThreadsafeLevelAdjust =
    ThreadsafeFilter<LevelGroup<Value>, LevelAdjust<Value>>;


} // end namespace iris
