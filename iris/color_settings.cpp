#include "iris/color_settings.h"


template struct pex::Group
    <
        iris::ColorFields,
        iris::ColorTemplate<int32_t>::template Template,
        iris::ColorCustom<int32_t>
    >;
