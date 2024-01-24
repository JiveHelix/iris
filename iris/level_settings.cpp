#include "iris/level_settings.h"


namespace iris
{


template struct iris::LevelSettings<int32_t>;


} // end namespace iris


template struct pex::Group
    <
        iris::LevelFields,
        iris::LevelTemplate<int32_t>::template Template,
        iris::LevelCustom<int32_t>
    >;
