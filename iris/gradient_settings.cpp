#include "iris/gradient_settings.h"


namespace iris
{


template struct GradientSettings<int32_t>;
template struct GradientModel<int32_t>;


} // end namespace iris


template struct pex::Group
    <
        iris::GradientFields,
        iris::GradientTemplate<int32_t>::template Template,
        iris::GradientSettings<int32_t>
    >;


template struct pex::MakeGroup
    <
        iris::GradientGroup<int32_t>,
        iris::GradientModel<int32_t>
    >;
