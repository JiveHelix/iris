#pragma once


#include <cstdint>
#include <draw/views/pixel_view_settings.h>
#include "iris/lines_chain_settings.h"
#include "iris/canny_chain.h"
#include "iris/hough.h"
#include "iris/node.h"
#include "iris/color_map.h"


namespace iris
{


struct LinesChainFilters
{
    using HoughFilter = Hough<double>;
};


struct LinesChainResults
{
    using Filters = LinesChainFilters;
    std::shared_ptr<const CannyChainResults> cannyChain;
    std::shared_ptr<const typename Filters::HoughFilter::Result> hough;

    using HoughPixelsControl =
        typename draw::PixelViewControl::AsyncPixelsControl;

    LinesChainResults(int64_t shapesId);

    std::shared_ptr<draw::Pixels> Display(
        const tau::Margins &margins,
        const draw::AsyncShapesControl &shapesControl,
        const draw::LinesShapeSettings &linesShapeSettings,
        ThreadsafeColorMap<int32_t> &color,
        HoughPixelsControl *houghControl) const;

private:
    int64_t shapesId_;
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
    using ResultPtr = std::shared_ptr<const Result>;
    using ChainResults = LinesChainResults;
    using ChainResultsPtr = std::shared_ptr<const ChainResults>;

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

    ResultPtr DoGetResult()
    {
        if (!this->settings_.enable)
        {
            return {};
        }

        return this->nodes_.hough.GetResult();
    }

    ChainResultsPtr GetChainResults()
    {
        if (!this->settings_.enable)
        {
            return {};
        }

        {
            std::lock_guard lock(this->mutex_);
            this->settingsChanged_ = false;
        }

        auto result =
            std::make_shared<ChainResults>(this->shapesId_.Get());

        result->hough = this->nodes_.hough.GetResult();
        result->cannyChain = this->nodes_.cannyChain.GetChainResults();

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

    int64_t GetShapesId() const
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
