#include "iris/gaussian_settings.h"


namespace iris
{


template struct GaussianSettings<int32_t>;


} // end namespace iris


template struct pex::Group
<
    iris::GaussianFields,
    iris::GaussianTemplate<int32_t>::template Template,
    iris::GaussianSettings<int32_t>
>;
