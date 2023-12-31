#include "filters.h"


Filters::Filters(DemoControl controls)
    :
    cancel(false),
    source(),
    mask(
        "Mask",
        this->source,
        controls.mask,
        iris::CancelControl(this->cancel)),
    level(
        this->mask,
        controls.level,
        iris::CancelControl(this->cancel)),
    vertexChain(
        this->level,
        controls.vertexChain,
        iris::CancelControl(this->cancel)),
    color(controls.color)
{

}
