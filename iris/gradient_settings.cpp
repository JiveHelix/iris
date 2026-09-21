#include "iris/gradient_settings.h"


template struct pex::Group
    <
        iris::GradientTemplate<int32_t>::template Template,
        iris::GradientCustom<int32_t>
    >;
