#include "iris/harris_settings.h"



template struct pex::Group
    <
        iris::HarrisSchema<float, iris::HarrisRanges>::template Schema,
        pex::PlainT<iris::HarrisSettings<float>>
    >;

template struct pex::Group
    <
        iris::HarrisSchema<double, iris::HarrisRanges>::template Schema,
        pex::PlainT<iris::HarrisSettings<double>>
    >;
