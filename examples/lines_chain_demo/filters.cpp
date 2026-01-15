#include "filters.h"


Filters::Filters(
    const iris::CancelControl &cancelControl,
    const DemoControl &controls)
    :
    source(),

    mask(
        "Mask",
        this->source,
        controls.mask,
        cancelControl),

    level(
        this->mask,
        controls.level,
        cancelControl),

    linesChain(
        this->level,
        controls.linesChain,
        cancelControl),

    color(controls.color)
{

}
