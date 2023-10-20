#pragma once

#include <memory>
#include <draw/pixels.h>
#include <draw/views/pixel_view.h>

#include "iris/color.h"
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
    std::optional<typename Filters::MaskFilter::Result> mask;
    std::optional<typename Filters::LevelFilter::Result> level;

    std::optional<typename Filters::GaussianFilter::Result> gaussian;
    std::optional<typename Filters::GradientFilter::Result> gradient;
    std::optional<typename Filters::CannyFilter::Result> canny;
    std::optional<typename Filters::HoughFilter::Result> hough;

    std::optional<typename Filters::HarrisFilter::Result> harris;
    std::optional<typename Filters::VertexFilter::Result> vertices;

    std::optional<typename Chess::Result> chess;

    using ShapesControl =
        typename draw::PixelViewControl::AsyncShapesControl;

    using HoughControl =
        typename draw::PixelViewControl::AsyncPixelsControl;

    ChessChainResults(
        ssize_t chessShapesId,
        ssize_t linesShapesId,
        ssize_t verticesShapesId);

    std::shared_ptr<draw::Pixels> Display(
        draw::ShapesControl shapesControl,
        const draw::LinesShapeSettings &linesShapeSettings,
        const draw::PointsShapeSettings &pointsShapeSettings,
        const ChessShapeSettings &chessShapeSettings,
        ThreadsafeColor<int32_t> &color,
        std::optional<HoughControl> houghControl,
        std::optional<ChessChainNodeSettings> nodeSettings) const;

    std::shared_ptr<draw::Pixels> DisplayNode(
        ChessChainNodeSettings nodeSettings,
        draw::ShapesControl shapesControl,
        const draw::LinesShapeSettings &linesShapeSettings,
        const draw::PointsShapeSettings &pointsShapeSettings,
        const ChessShapeSettings &chessShapeSettings,
        ThreadsafeColor<int32_t> &color,
        std::optional<HoughControl> houghControl) const;

private:
    std::shared_ptr<draw::Pixels> GetPreprocessedPixels_(
        ThreadsafeColor<int32_t> &color) const;

    std::shared_ptr<draw::Pixels> GetNodePixels_(
        ChessChainNodeSettings nodeSettings,
        ThreadsafeColor<int32_t> &color) const;

    void DrawHoughResults_(
        draw::ShapesControl shapesControl,
        const draw::LinesShapeSettings &linesShapeSettings,
        ThreadsafeColor<int32_t> &color,
        std::optional<HoughControl> houghControl) const;

    void DrawVerticesResults_(
        draw::ShapesControl shapesControl,
        const draw::PointsShapeSettings &pointsShapeSettings,
        ThreadsafeColor<int32_t> &color) const;

private:
    ssize_t chessShapesId_;
    ssize_t linesShapesId_;
    ssize_t verticesShapesId_;
};


} // end namespace iris
