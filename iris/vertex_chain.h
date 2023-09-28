#pragma once


#include <optional>
#include <draw/views/pixel_view_settings.h>
#include <draw/points_shape.h>
#include "iris/vertex_chain_settings.h"
#include "iris/level_adjust.h"
#include "iris/gaussian.h"
#include "iris/gradient.h"
#include "iris/harris.h"
#include "iris/vertex.h"
#include "iris/node.h"
#include "iris/image.h"
#include "iris/color.h"


namespace iris
{


struct VertexChainFilters
{
    using GaussianFilter = Gaussian<int32_t, 0>;
    using GradientFilter = Gradient<int32_t>;
    using HarrisFilter = Harris<double>;
    using VertexFilter = VertexFinder;
};


struct VertexChainResults
{
    using Filters = VertexChainFilters;
    std::optional<typename Filters::GaussianFilter::Result> gaussian;
    std::optional<typename Filters::GradientFilter::Result> gradient;
    std::optional<typename Filters::HarrisFilter::Result> harris;
    std::optional<typename Filters::VertexFilter::Result> vertex;

    using ShapesControl =
        typename draw::PixelViewControl::AsyncShapesControl;

    VertexChainResults(ssize_t shapesId);

    std::shared_ptr<draw::Pixels> Display(
        draw::ShapesControl shapesControl,
        const draw::PointsShapeSettings &pointsShapeSettings,
        ThreadsafeColor<int32_t> &color) const;

private:
    ssize_t shapesId_;
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
    using Result = typename VertexChainNodes<SourceNode>::Result;
    using ChainResults = VertexChainResults;

    using Base = NodeBase
        <
            SourceNode,
            VertexChainControl,
            Result,
            VertexChain<SourceNode>
        >;

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

    std::optional<Result> DoGetResult()
    {
        if (!this->settings_.enable)
        {
            return {};
        }

        return this->nodes_.vertex.GetResult();
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

        ChainResults result(this->shapesId_.Get());
        result.vertex = this->nodes_.vertex.GetResult();
        result.harris = this->nodes_.harris.GetResult();
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

    ssize_t GetShapesId() const
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
