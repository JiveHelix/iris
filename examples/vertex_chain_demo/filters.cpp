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

    vertexChain(
        this->level,
        controls.vertexChain,
        cancelControl),

    color(controls.color)
{

}
