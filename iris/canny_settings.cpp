#include "iris/canny_settings.h"


namespace iris
{


template struct CannySettings<float>;
template struct CannySettings<double>;


} // end namespace iris


template struct pex::LinkedRanges
    <
        float,
        iris::CannyLowerBound,
        iris::CannyLow,
        iris::CannyUpperBound,
        iris::CannyHigh
    >;


template struct pex::LinkedRanges
    <
        double,
        iris::CannyLowerBound,
        iris::CannyLow,
        iris::CannyUpperBound,
        iris::CannyHigh
    >;


template struct pex::Group
    <
        iris::CannyFields,
        iris::CannyTemplate<float>::template Template,
        pex::PlainT<iris::CannySettings<float>>
    >;


template struct pex::Group
    <
        iris::CannyFields,
        iris::CannyTemplate<double>::template Template,
        pex::PlainT<iris::CannySettings<double>>
    >;
