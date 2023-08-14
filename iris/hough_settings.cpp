#include "iris/hough_settings.h"


namespace iris
{


template struct HoughSettings<float>;
template struct HoughSettings<double>;

template struct HoughModel<float>;
template struct HoughModel<double>;


} // end namespace iris


template struct pex::Group
    <
        iris::HoughFields,
        iris::HoughTemplate<float>::template Template,
        iris::HoughSettings<float>
    >;


template struct pex::Group
    <
        iris::HoughFields,
        iris::HoughTemplate<double>::template Template,
        iris::HoughSettings<double>
    >;



