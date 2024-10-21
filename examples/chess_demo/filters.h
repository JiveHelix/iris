#pragma once


#include <iris/node.h>
#include <iris/mask.h>
#include <iris/level_adjust.h>
#include <iris/chess_chain.h>
#include <iris/color_map.h>

#include "demo_settings.h"


class Filters
{
public:
    using SourceNode = typename iris::ChessChain::SourceNode;
    using Color = iris::ThreadsafeColorMap<int32_t>;
    using Chess = iris::ChessChain;

    iris::Cancel cancel;
    SourceNode source;
    Chess chess;
    Color color;

    Filters(DemoControl controls);

    Filters(const Filters &) = delete;
};


