#pragma once

#include <tau/color_map.h>
#include <tau/percentile.h>
#include <draw/mono_image.h>

#include "iris/level_settings.h"
#include "iris/node.h"
#include "iris/mask.h"


namespace iris
{


template<typename Value, typename Float>
class LevelAdjust
{
public:
    using Matrix = draw::MonoImage<Value>;
    using Result = Matrix;

    static tau::FloatRescale<Value, Float> MakeRescale(
        Value maximum,
        Value low,
        Value high)
    {
        low = std::min(high - 1, low);
        assert(low < high);

        return tau::FloatRescale<Value, Float>(maximum + 1, low, high);
    }

    LevelAdjust()
        :
        enable_(false),
        rescale_(MakeRescale(255, 0, 255))
    {

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

    std::optional<Result> Filter(const Matrix &data)
    {
        if (!this->enable_)
        {
            return data;
        }

        return this->rescale_(data);
    }

protected:
    bool enable_;
    tau::FloatRescale<Value, Float> rescale_;
};


template<typename SourceNode, typename Value, typename Float>
class LevelAdjustNode
    :
    public Node<SourceNode, LevelAdjust<Value, Float>, LevelControl<Value>>
{
public:

    using Control = LevelControl<Value>;
    using Filter = LevelAdjust<Value, Float>;
    using Base = Node<SourceNode, Filter, Control>;

    LevelAdjustNode(
        SourceNode &source,
        Control control,
        CancelControl cancel)
        :
        Base("LevelAdjust", source, control, cancel),
        control_(control),
        detectEndpoint_(
            PEX_THIS("LevelAdjustNode"),
            control.autoDetectSettings,
            &LevelAdjustNode::AutoDetectSettings)
    {

    }

    void AutoDetectSettings()
    {
        auto input = this->input_.GetResult();

        if (!input)
        {
            std::cerr << "Unable to detect levels without input." << std::endl;
            return;
        }

        auto filtered = tau::RemoveZeros(*input);

        if (filtered.size() < 2)
        {
            auto defer = pex::MakeDefer(this->control_);
            defer.range.low.Set(0);
            defer.range.high.Set(255);

            return;
        }

        // The input to this node may be a mask that sets masked values to zero.
        // Estimate the low and high percentile without considering the zeros.
        auto values = tau::Percentile(
            filtered,
            Eigen::Vector2d(
                this->settings_.detectMargin,
                1.0 - this->settings_.detectMargin));

        auto defer = pex::MakeDefer(this->control_);
        defer.range.low.Set(values(0));
        defer.range.high.Set(values(1));
    }

    using DetectEndpoint =
        pex::Endpoint<LevelAdjustNode, pex::control::Signal<>>;

    Control control_;
    DetectEndpoint detectEndpoint_;
};


extern template class LevelAdjust<int32_t, float>;
extern template class LevelAdjustNode<DefaultMaskNode, int32_t, float>;
using DefaultLevelAdjustNode = LevelAdjustNode<DefaultMaskNode, int32_t, float>;


} // end namespace iris
