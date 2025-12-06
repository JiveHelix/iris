#pragma once

#include <cstdint>
#include <memory>
#include <draw/pixels.h>
#include <draw/node_settings.h>
#include <draw/views/pixel_view.h>

#include "iris/color_map.h"
#include "iris/node.h"
#include "iris/mask.h"
#include "iris/level_adjust.h"
#include "iris/gaussian.h"
#include "iris/gradient.h"
#include "iris/canny.h"
#include "iris/harris.h"
#include "iris/hough.h"
#include "iris/vertex.h"
#include "iris/chess.h"
#include "iris/chess_chain_node_settings.h"

#include "iris/views/chess_shape.h"


namespace iris
{


struct ChessChainFilters
{
    using MaskFilter = Mask<InProcess>;
    using LevelFilter = LevelAdjust<InProcess, double>;

    using GaussianFilter = Gaussian<int32_t, 0>;
    using GradientFilter = Gradient<int32_t>;
    using CannyFilter = Canny<double>;
    using HoughFilter = Hough<double>;

    using HarrisFilter = Harris<double>;
    using VertexFilter = VertexFinder;
};


struct ChessChainResults
{
    using Filters = ChessChainFilters;
    std::shared_ptr<const typename Filters::MaskFilter::Result> mask;
    std::shared_ptr<const typename Filters::LevelFilter::Result> level;

    std::shared_ptr<const typename Filters::GaussianFilter::Result> gaussian;
    std::shared_ptr<const typename Filters::GradientFilter::Result> gradient;
    std::shared_ptr<const typename Filters::CannyFilter::Result> canny;
    std::shared_ptr<const typename Filters::HoughFilter::Result> hough;

    std::shared_ptr<const typename Filters::HarrisFilter::Result> harris;
    std::shared_ptr<const typename Filters::VertexFilter::Result> vertices;

    std::shared_ptr<const typename Chess::Result> chess;

    using HoughPixelsControl =
        typename draw::PixelViewControl::AsyncPixelsControl;

    ChessChainResults(
        int64_t chessShapesId,
        int64_t linesShapesId,
        int64_t verticesShapesId);

    std::shared_ptr<draw::Pixels> Display(
        const tau::Margins &margins,
        const draw::AsyncShapesControl &shapesControl,
        const draw::LinesShapeSettings &linesShapeSettings,
        const draw::PointsShapeSettings &pointsShapeSettings,
        const ChessShapeSettings &chessShapeSettings,
        ThreadsafeColorMap<int32_t> &color,
        HoughPixelsControl *houghControl,
        ChessChainNodeSettings *nodeSettings) const;

    std::shared_ptr<draw::Pixels> DisplayNode(
        const tau::Margins &margins,
        const ChessChainNodeSettings &nodeSettings,
        const draw::AsyncShapesControl &shapesControl,
        const draw::LinesShapeSettings &linesShapeSettings,
        const draw::PointsShapeSettings &pointsShapeSettings,
        const ChessShapeSettings &chessShapeSettings,
        ThreadsafeColorMap<int32_t> &color,
        HoughPixelsControl *houghControl) const;

private:
    void ClearShapes_(draw::AsyncShapesControl) const;

    std::shared_ptr<draw::Pixels> GetPreprocessedPixels_(
        const tau::Margins &margins,
        ThreadsafeColorMap<int32_t> &color) const;

    std::shared_ptr<draw::Pixels> GetNodePixels_(
        const tau::Margins &margins,
        const ChessChainNodeSettings &nodeSettings,
        ThreadsafeColorMap<int32_t> &color) const;

    void DrawHoughResults_(
        const draw::AsyncShapesControl &shapesControl,
        const draw::LinesShapeSettings &linesShapeSettings,
        ThreadsafeColorMap<int32_t> &color,
        HoughPixelsControl *houghControl) const;

    void DrawVerticesResults_(
        const draw::AsyncShapesControl &shapesControl,
        const draw::PointsShapeSettings &pointsShapeSettings,
        ThreadsafeColorMap<int32_t> &color) const;

private:
    int64_t chessShapesId_;
    int64_t linesShapesId_;
    int64_t verticesShapesId_;
};


} // end namespace iris
