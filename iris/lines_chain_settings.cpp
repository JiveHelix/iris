#include "iris/lines_chain_settings.h"


template struct pex::Group
    <
        iris::LinesChainFields,
        iris::LinesChainTemplate,
        iris::LinesChainSettings
    >;


template struct pex::MakeGroup
    <
        iris::LinesChainGroup,
        iris::LinesChainModel
    >;
