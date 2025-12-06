#pragma once


#include <cstdint>
#include <draw/views/pixel_view_settings.h>
#include <draw/points_shape.h>
#include "iris/vertex_chain_settings.h"
#include "iris/level_adjust.h"
#include "iris/gaussian.h"
#include "iris/gradient.h"
#include "iris/harris.h"
#include "iris/vertex.h"
#include "iris/node.h"
#include "iris/color_map.h"


namespace iris
{


template<typename Filter>
using FilterResult =
    std::shared_ptr<const typename Filter::Result>;


struct VertexChainFilters
{
    using GaussianFilter = Gaussian<int32_t, 0>;
    using GradientFilter = Gradient<int32_t>;
    using HarrisFilter = Harris<double>;
    using VertexFilter = VertexFinder;

    using GaussianResult = FilterResult<GaussianFilter>;
    using GradientResult = FilterResult<GradientFilter>;
    using HarrisResult = FilterResult<HarrisFilter>;
    using VertexResult = FilterResult<VertexFilter>;
};



struct VertexChainResults
{
    using Filters = VertexChainFilters;

    typename Filters::GaussianResult gaussian;
    typename Filters::GradientResult gradient;
    typename Filters::HarrisResult harris;
    typename Filters::VertexResult vertex;

    VertexChainResults(int64_t shapesId);

    std::shared_ptr<draw::Pixels> Display(
        const tau::Margins &margins,
        draw::AsyncShapesControl shapesControl,
        const draw::PointsShapeSettings &pointsShapeSettings,
        ThreadsafeColorMap<int32_t> &color) const;

private:
    int64_t shapesId_;
};


template<typename SourceNode>
struct VertexChainNodes
{
    using Filters = VertexChainFilters;
    using GaussianFilter = typename Filters::GaussianFilter;
    using GradientFilter = typename Filters::GradientFilter;
    using HarrisFilter = typename Filters::HarrisFilter;
    using VertexFilter = typename Filters::VertexFilter;

    using Result = typename VertexFilter::Result;
    using ResultPtr = std::shared_ptr<const Result>;
    using ChainResults = VertexChainResults;

    using GaussianNode =
        iris::Node<SourceNode, GaussianFilter, GaussianControl<int32_t>>;

    using GradientNode_ = GradientNode<GaussianNode>;

    using HarrisNode =
        iris::Node<GradientNode_, HarrisFilter, HarrisControl<double>>;

    using VertexNode =
        iris::Node<HarrisNode, VertexFilter, VertexControl>;

    SourceNode & source;
    GaussianNode gaussian;
    GradientNode_ gradient;
    HarrisNode harris;
    VertexNode vertex;

    VertexChainNodes(
        SourceNode &source_,
        VertexChainControl controls,
        CancelControl cancel)
        :
        source(source_),
        gaussian("Gaussian", this->source, controls.gaussian, cancel),
        gradient(this->gaussian, controls.gradient, cancel),
        harris("Harris", this->gradient, controls.harris, cancel),
        vertex("Vertex", this->harris, controls.vertex, cancel)
    {

    }

};


template<typename SourceNode>
class VertexChain
    :
    public NodeBase
        <
            SourceNode,
            VertexChainControl,
            typename VertexChainNodes<SourceNode>::Result,
            VertexChain<SourceNode>
        >
{
public:

    using Base = NodeBase
        <
            SourceNode,
            VertexChainControl,
            typename VertexChainNodes<SourceNode>::Result,
            VertexChain<SourceNode>
        >;

    using Result = typename Base::Result;
    using ResultPtr = typename Base::ResultPtr;
    using ChainResults = VertexChainResults;

    VertexChain(
        SourceNode &sourceNode,
        VertexChainControl controls,
        CancelControl cancel)
        :
        Base("VertexChain", sourceNode, controls, cancel),
        shapesId_(),
        nodes_(sourceNode, controls, cancel)
    {

    }

    ResultPtr DoGetResult()
    {
        if (!this->settings_.enable)
        {
            return {};
        }

        return this->nodes_.vertex.GetResult();
    }

    std::shared_ptr<ChainResults> GetChainResults()
    {
        if (!this->settings_.enable)
        {
            return {};
        }

        {
            std::lock_guard lock(this->mutex_);
            this->settingsChanged_ = false;
        }

        auto result = std::make_shared<ChainResults>(this->shapesId_.Get());
        result->vertex = this->nodes_.vertex.GetResult();
        result->harris = this->nodes_.harris.GetResult();
        result->gradient = this->nodes_.gradient.GetResult();
        result->gaussian = this->nodes_.gaussian.GetResult();

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

    int64_t GetShapesId() const
    {
        return this->shapesId_.Get();
    }

private:
    draw::ShapesId shapesId_;
    VertexChainNodes<SourceNode> nodes_;
};


extern template class VertexChain<DefaultLevelAdjustNode>;
using DefaultVertexChain = VertexChain<DefaultLevelAdjustNode>;


} // end namespace iris
