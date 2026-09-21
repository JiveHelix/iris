#include "iris/harris_settings.h"



template struct pex::Group
    <
        iris::HarrisTemplate<float, iris::HarrisRanges>::template Template,
        pex::PlainT<iris::HarrisSettings<float>>
    >;

template struct pex::Group
    <
        iris::HarrisTemplate<double, iris::HarrisRanges>::template Template,
        pex::PlainT<iris::HarrisSettings<double>>
    >;
