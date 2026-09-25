#include "iris/hough_settings.h"


template struct pex::Group
    <
        iris::HoughSchema<float>::template Schema,
        iris::HoughFinisher<float>
    >;


template struct pex::Group
    <
        iris::HoughSchema<double>::template Schema,
        iris::HoughFinisher<double>
    >;



