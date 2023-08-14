#pragma once


#include <optional>
#include <draw/views/pixel_view_settings.h>
#include <draw/points_shape.h>
#include "iris/corners_chain_settings.h"
#include "iris/level_adjust.h"
#include "iris/gaussian.h"
#include "iris/gradient.h"
#include "iris/harris.h"
#include "iris/corner.h"
#include "iris/node.h"
#include "iris/image.h"
#include "iris/color.h"


namespace iris
{


struct CornersChainFilters
{
    using GaussianFilter = Gaussian<int32_t, 0>;
    using GradientFilter = Gradient<int32_t>;
    using HarrisFilter = Harris<double>;
    using CornerFilter = Corner<double>;
};


struct CornersChainResults
{
    using Filters = CornersChainFilters;
    std::optional<typename Filters::GaussianFilter::Result> gaussian;
    std::optional<typename Filters::GradientFilter::Result> gradient;
    std::optional<typename Filters::HarrisFilter::Result> harris;
    std::optional<typename Filters::CornerFilter::Result> corner;

    using ShapesControl =
        typename draw::PixelViewControl::AsyncShapesControl;

    CornersChainResults(ssize_t shapesId);

    std::shared_ptr<draw::Pixels> Display(
        draw::ShapesControl shapesControl,
        const draw::PointsShapeSettings &pointsShapeSettings,
        ThreadsafeColor<int32_t> &color) const;

private:
    ssize_t shapesId_;
};


template<typename SourceNode>
struct CornersChainNodes
{
    using Filters = CornersChainFilters;
    using GaussianFilter = typename Filters::GaussianFilter;
    using GradientFilter = typename Filters::GradientFilter;
    using HarrisFilter = typename Filters::HarrisFilter;
    using CornerFilter = typename Filters::CornerFilter;

    using Result = typename CornerFilter::Result;
    using ChainResults = CornersChainResults;

    using GaussianNode =
        iris::Node<SourceNode, GaussianFilter, GaussianControl<int32_t>>;

    using GradientNode_ = GradientNode<GaussianNode>;

    using HarrisNode =
        iris::Node<GradientNode_, HarrisFilter, HarrisControl<double>>;

    using CornerNode =
        iris::Node<HarrisNode, CornerFilter, CornerControl>;

    SourceNode & source;
    GaussianNode gaussian;
    GradientNode_ gradient;
    HarrisNode harris;
    CornerNode corner;

    CornersChainNodes(
        SourceNode &source_,
        CornersChainControl controls,
        CancelControl cancel)
        :
        source(source_),
        gaussian("Gaussian", this->source, controls.gaussian, cancel),
        gradient(this->gaussian, controls.gradient, cancel),
        harris("Harris", this->gradient, controls.harris, cancel),
        corner("Corner", this->harris, controls.corner, cancel)
    {

    }

};


template<typename SourceNode>
class CornersChain
    :
    public NodeBase
        <
            SourceNode,
            CornersChainControl,
            typename CornersChainNodes<SourceNode>::Result,
            CornersChain<SourceNode>
        >
{
public:
    using Result = typename CornersChainNodes<SourceNode>::Result;
    using ChainResults = CornersChainResults;

    using Base = NodeBase
        <
            SourceNode,
            CornersChainControl,
            Result,
            CornersChain<SourceNode>
        >;

    CornersChain(
        SourceNode &sourceNode,
        CornersChainControl controls,
        CancelControl cancel)
        :
        Base("CornersChain", sourceNode, controls, cancel),
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

        return this->nodes_.corner.GetResult();
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
        result.corner = this->nodes_.corner.GetResult();
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
    CornersChainNodes<SourceNode> nodes_;
};


extern template class CornersChain<DefaultLevelAdjustNode>;
using DefaultCornersChain = CornersChain<DefaultLevelAdjustNode>;


} // end namespace iris
