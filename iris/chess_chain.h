#pragma once


#include <pex/endpoint.h>

#include "iris/node.h"
#include "iris/mask.h"
#include "iris/level_adjust.h"
#include "iris/lines_chain.h"
#include "iris/corners_chain.h"
#include "iris/chess.h"
#include "iris/views/chess_shape.h"
#include "iris/chess_chain_settings.h"


namespace iris
{


template<typename CornersSource, typename LinesSource>
struct ChessNodes
{
    using MuxNode = Mux<CornersSource, LinesSource, ChessInput>;
    using FilterNode = Node<MuxNode, Chess, ChessControl>;
};


struct ChessChainFilters
{
    using MaskFilter = Mask<InProcess>;
    using LevelFilter = LevelAdjust<InProcess, double>;
};


struct ChessChainResults
{
    using Filters = ChessChainFilters;
    std::optional<typename Filters::MaskFilter::Result> mask;
    std::optional<typename Filters::LevelFilter::Result> level;
    std::optional<CornersChainResults> corners;
    std::optional<LinesChainResults> lines;
    std::optional<typename Chess::Result> chess;

    using ShapesControl =
        typename draw::PixelViewControl::AsyncShapesControl;

    using HoughControl =
        typename draw::PixelViewControl::AsyncPixelsControl;

    ChessChainResults(ssize_t chessShapesId);

    std::shared_ptr<draw::Pixels> Display(
        draw::ShapesControl shapesControl,
        const draw::LinesShapeSettings &linesShapeSettings,
        const draw::PointsShapeSettings &pointsShapeSettings,
        const ChessShapeSettings &chessShapeSettings,
        ThreadsafeColor<InProcess> &color,
        std::optional<HoughControl> houghControl) const;

private:
    ssize_t chessShapesId_;
};


struct ChessChainNodes
{
    using SourceNode = Source<ProcessMatrix>;

    using Filters = ChessChainFilters;
    using MaskFilter = typename Filters::MaskFilter;

    using MaskNode = Node<SourceNode, MaskFilter, MaskControl>;
    using LevelNode = LevelAdjustNode<MaskNode, InProcess, double>;
    using LinesChainNode = LinesChain<LevelNode>;
    using CornersChainNode = CornersChain<LevelNode>;

    using Result = typename Chess::Result;

    using MuxNode =
        typename ChessNodes<CornersChainNode, LinesChainNode>::MuxNode;

    using ChessNode =
        typename ChessNodes<CornersChainNode, LinesChainNode>::FilterNode;

    MaskNode mask;
    LevelNode level;
    LinesChainNode lines;
    CornersChainNode corners;
    MuxNode mux;
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
    draw::ShapesId chessShapesId_;
    ChessChainNodes nodes_;
    pex::Endpoint<ChessChain, pex::control::Signal<>> autoDetectEndpoint_;
};


} // end namespace iris
