#pragma once


#include <optional>
#include <draw/views/pixel_view_settings.h>
#include "iris/lines_chain_settings.h"
#include "iris/canny_chain.h"
#include "iris/hough.h"
#include "iris/node.h"
#include "iris/image.h"
#include "iris/color.h"


namespace iris
{


struct LinesChainFilters
{
    using HoughFilter = Hough<double>;
};


struct LinesChainResults
{
    using Filters = LinesChainFilters;
    std::optional<CannyChainResults> cannyChain;
    std::optional<typename Filters::HoughFilter::Result> hough;

    using ShapesControl =
        typename draw::PixelViewControl::AsyncShapesControl;

    using HoughControl =
        typename draw::PixelViewControl::AsyncPixelsControl;

    LinesChainResults(ssize_t shapesId);

    std::shared_ptr<draw::Pixels> Display(
        ShapesControl shapesControl,
        const draw::LinesShapeSettings &linesShapeSettings,
        ThreadsafeColor<int32_t> &color,
        std::optional<HoughControl> houghControl) const;

private:
    ssize_t shapesId_;
};



template<typename SourceNode>
struct LinesChainNodes
{
    using Filters = LinesChainFilters;
    using HoughFilter = typename Filters::HoughFilter;

    using Result = typename HoughFilter::Result;

    using CannyChainNode = CannyChain<SourceNode>;

    using HoughNode =
        iris::Node<CannyChainNode, HoughFilter, HoughControl<double>>;

    CannyChainNode cannyChain;
    HoughNode hough;

    LinesChainNodes(
        SourceNode &source,
        LinesChainControl controls,
        CancelControl cancel)
        :
        cannyChain(source, controls.cannyChain, cancel),
        hough("Hough", this->cannyChain, controls.hough, cancel)
    {

    }
};


template<typename SourceNode>
class LinesChain
    :
    public NodeBase
        <
            SourceNode,
            LinesChainControl,
            typename LinesChainNodes<SourceNode>::Result,
            LinesChain<SourceNode>
        >
{
public:
    using Result = typename LinesChainNodes<SourceNode>::Result;
    using ChainResults = LinesChainResults;

    using Base = NodeBase
        <
            SourceNode,
            LinesChainControl,
            Result,
            LinesChain<SourceNode>
        >;

    LinesChain(
        SourceNode &sourceNode,
        LinesChainControl controls,
        CancelControl cancel)
        :
        Base("LinesChain", sourceNode, controls, cancel),
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

        return this->nodes_.hough.GetResult();
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
        result.hough = this->nodes_.hough.GetResult();
        result.cannyChain = this->nodes_.cannyChain.GetChainResults();

        std::lock_guard lock(this->mutex_);

        if (this->settingsChanged_)
        {
            return {};
        }

        return result;
    }

    void AutoDetectSettings()
    {
        this->nodes_.cannyChain.AutoDetectSettings();
    }

    ssize_t GetShapesId() const
    {
        return this->shapesId_.Get();
    }

private:
    draw::ShapesId shapesId_;
    LinesChainNodes<SourceNode> nodes_;
};


extern template class LinesChain<DefaultLevelAdjustNode>;
using DefaultLinesChain = LinesChain<DefaultLevelAdjustNode>;

} // end namespace iris
