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
    lines(
        this->level,
        controls.lines,
        iris::CancelControl(this->cancel)),
    color(controls.color)
{

}
