#pragma once


#include <pex/endpoint.h>
#include "iris/chess_chain_results.h"
#include "iris/chess_chain_settings.h"
#include "iris/chess/chess_solution.h"


namespace iris
{


template<typename VertexSource, typename LinesSource>
struct ChessNodes
{
    using MixNode = Mix<VertexSource, LinesSource, ChessInput>;
    using FilterNode = Node<MixNode, Chess, ChessControl>;
};


struct ChessChainNodes
{
    using SourceNode = Source<ProcessMatrix>;

    using Filters = ChessChainFilters;

    // Common
    using MaskFilter = typename Filters::MaskFilter;
    using GaussianFilter = typename Filters::GaussianFilter;
    using GradientFilter = typename Filters::GradientFilter;

    // Lines
    using CannyFilter = typename Filters::CannyFilter;
    using HoughFilter = typename Filters::HoughFilter;

    // Vertices
    using HarrisFilter = typename Filters::HarrisFilter;
    using VertexFilter = typename Filters::VertexFilter;


    using MaskNode = Node<SourceNode, MaskFilter, MaskControl>;
    using LevelNode = LevelAdjustNode<MaskNode, InProcess, double>;

    using GaussianNode =
        iris::Node<LevelNode, GaussianFilter, GaussianControl<int32_t>>;

    using GradientNode_ = GradientNode<GaussianNode>;


    using CannyNode =
        iris::Node<GradientNode_, CannyFilter, CannyControl<double>>;

    using HoughNode =
        iris::Node<CannyNode, HoughFilter, HoughControl<double>>;

    using HarrisNode =
        iris::Node<GradientNode_, HarrisFilter, HarrisControl<double>>;

    using VertexNode =
        iris::Node<HarrisNode, VertexFilter, VertexControl>;

    using Result = ChessSolution;

    using MixNode =
        typename ChessNodes<VertexNode, HoughNode>::MixNode;

    using ChessNode =
        typename ChessNodes<VertexNode, HoughNode>::FilterNode;

    MaskNode mask;
    LevelNode level;
    GaussianNode gaussian;
    GradientNode_ gradientForCanny;
    GradientNode_ gradientForHarris;

    CannyNode canny;
    HoughNode hough;

    HarrisNode harris;
    VertexNode vertices;

    MixNode mix;
    ChessNode chess;

    ChessChainNodes(
        SourceNode &source,
        ChessChainControl control,
        CancelControl cancel);
};


class ChessChain
    :
    public NodeBase
        <
            typename ChessChainNodes::SourceNode,
            ChessChainControl,
            typename ChessChainNodes::Result,
            ChessChain
        >
{
public:
    using SourceNode = typename ChessChainNodes::SourceNode;
    using Result = typename ChessChainNodes::Result;
    using ChainResults = ChessChainResults;

    using Base = NodeBase
        <
            SourceNode,
            ChessChainControl,
            Result,
            ChessChain
        >;

    ChessChain(
        SourceNode &sourceNode,
        ChessChainControl control,
        CancelControl cancel);

    void AutoDetectSettings();

    std::optional<Result> DoGetResult();

    std::optional<ChainResults> GetChainResults();

    ssize_t GetShapesId() const
    {
        return this->chessShapesId_.Get();
    }

private:
    draw::ShapesId linesShapesId_;
    draw::ShapesId verticesShapesId_;
    draw::ShapesId chessShapesId_;
    ChessChainNodes nodes_;
    pex::Endpoint<ChessChain, pex::control::DefaultSignal> autoDetectEndpoint_;
};


} // end namespace iris
