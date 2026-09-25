#include "iris/gradient_settings.h"


template struct pex::Group
    <
        iris::GradientSchema<int32_t>::template Schema,
        iris::GradientFinisher<int32_t>
    >;
