#include "iris/hough_settings.h"


template struct pex::Group
    <
        iris::HoughFields,
        iris::HoughTemplate<float>::template Template,
        iris::HoughCustom<float>
    >;


template struct pex::Group
    <
        iris::HoughFields,
        iris::HoughTemplate<double>::template Template,
        iris::HoughCustom<double>
    >;



