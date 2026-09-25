#include "iris/level_settings.h"


namespace iris
{


template struct iris::LevelSettings<int32_t>;


} // end namespace iris


template struct pex::Group
    <
        iris::LevelSchema<int32_t>::template Schema,
        iris::LevelFinisher<int32_t>
    >;
