#include "iris/gradient_settings.h"


template struct pex::Group
    <
        iris::GradientFields,
        iris::GradientTemplate<int32_t>::template Template,
        iris::GradientCustom<int32_t>
    >;
