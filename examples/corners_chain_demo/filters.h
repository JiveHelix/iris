#pragma once


#include <iris/node.h>
#include <iris/mask.h>
#include <iris/color.h>
#include <iris/corners_chain.h>
#include <iris/level_adjust.h>

#include "demo_settings.h"


class Filters
{
public:
    using SourceNode = iris::Source<iris::ProcessMatrix>;
    using Mask = iris::Mask<int32_t>;
    using Color = iris::ThreadsafeColor<int32_t>;

    using MaskNode = iris::Node<SourceNode, Mask, iris::MaskControl>;
    using LevelNode = iris::LevelAdjustNode<MaskNode, int32_t, double>;
    using CornersChain = iris::CornersChain<LevelNode>;

    iris::Cancel cancel;
    SourceNode source;
    MaskNode mask;
    LevelNode level;
    CornersChain cornersChain;
    Color color;

    Filters(DemoControl controls);

    Filters(const Filters &) = delete;
};


