#pragma once


#include <iris/node.h>
#include <iris/mask.h>
#include <iris/color.h>
#include <iris/lines_chain.h>
#include <iris/level_adjust.h>

#include "demo_settings.h"


class Filters
{
public:
    using Color = iris::ThreadsafeColor<int32_t>;

    iris::Cancel cancel;
    iris::DefaultSource source;
    iris::DefaultMaskNode mask;
    iris::DefaultLevelAdjustNode level;
    iris::DefaultLinesChain lines;
    Color color;

    Filters(DemoControl controls);

    Filters(const Filters &) = delete;
};


