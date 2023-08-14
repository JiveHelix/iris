#pragma once


#include <optional>
#include "iris/canny_chain_settings.h"
#include "iris/level_adjust.h"
#include "iris/gaussian.h"
#include "iris/gradient.h"
#include "iris/canny.h"
#include "iris/node.h"
#include "iris/image.h"
#include "iris/color.h"


namespace iris
{


struct CannyChainFilters
{
    using GaussianFilter = Gaussian<int32_t, 0>;
    using GradientFilter = Gradient<int32_t>;
    using CannyFilter = Canny<double>;
};


struct CannyChainResults
{
    using Filters = CannyChainFilters;
    std::optional<typename Filters::GaussianFilter::Result> gaussian;
    std::optional<typename Filters::GradientFilter::Result> gradient;
    std::optional<typename Filters::CannyFilter::Result> canny;

    std::shared_ptr<draw::Pixels>
        Display(ThreadsafeColor<int32_t> &color) const;
};



template<typename SourceNode>
struct CannyChainNodes
{
    using Filters = CannyChainFilters;
    using GaussianFilter = typename Filters::GaussianFilter;
    using GradientFilter = typename Filters::GradientFilter;
    using CannyFilter = typename Filters::CannyFilter;

    using Result = typename CannyFilter::Result;

    using GaussianNode =
        iris::Node<SourceNode, GaussianFilter, GaussianControl<int32_t>>;

    using GradientNode_ = GradientNode<GaussianNode>;

    using CannyNode =
        iris::Node<GradientNode_, CannyFilter, CannyControl<double>>;

    SourceNode & source;
    GaussianNode gaussian;
    GradientNode_ gradient;
    CannyNode canny;

    CannyChainNodes(
        SourceNode &source_,
        CannyChainControl controls,
        CancelControl cancel)
        :
        source(source_),
        gaussian("Gaussian", this->source, controls.gaussian, cancel),
        gradient(this->gaussian, controls.gradient, cancel),
        canny("Canny", this->gradient, controls.canny, cancel)
    {

    }
};


template<typename SourceNode>
class CannyChain
    :
    public NodeBase
        <
            SourceNode,
            CannyChainControl,
            typename CannyChainNodes<SourceNode>::Result,
            CannyChain<SourceNode>
        >
{
public:
    using Result = typename CannyChainNodes<SourceNode>::Result;
    using ChainResults = CannyChainResults;

    using Base = NodeBase
        <
            SourceNode,
            CannyChainControl,
            Result,
            CannyChain<SourceNode>
        >;

    CannyChain(
        SourceNode &sourceNode,
        CannyChainControl controls,
        CancelControl cancel)
        :
        Base("CannyChain", sourceNode, controls, cancel),
        nodes_(sourceNode, controls, cancel)
    {

    }

    std::optional<Result> DoGetResult()
    {
        if (!this->settings_.enable)
        {
            return {};
        }

        return this->nodes_.canny.GetResult();
    }

    std::optional<ChainResults> GetChainResults()
    {
        if (!this->settings_.enable)
        {
            return {};
        }

        {
            std::lock_guard lock(this->mutex_);
            this->settingsChanged_ = false;
        }

        ChainResults result;
        result.canny = this->nodes_.canny.GetResult();
        result.gradient = this->nodes_.gradient.GetResult();
        result.gaussian = this->nodes_.gaussian.GetResult();

        std::lock_guard lock(this->mutex_);

        if (this->settingsChanged_)
        {
            return {};
        }

        return result;
    }

    void AutoDetectSettings()
    {
        this->nodes_.gradient.AutoDetectSettings();
    }

private:
    CannyChainNodes<SourceNode> nodes_;
};


extern template class CannyChain<DefaultLevelAdjustNode>;
using DefaultCannyChain = CannyChain<DefaultLevelAdjustNode>;


} // end namespace iris
