#include "filters.h"


Filters::Filters(const DemoControl &controls)
    :
    cancel(false),
    source(),
    chess(
        this->source,
        controls.chess,
        iris::CancelControl(this->cancel)),
    color(controls.color)
{

}
