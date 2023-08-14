#include "iris/corners_chain_settings.h"


template struct pex::Group
    <
        iris::CornersChainFields,
        iris::CornersChainTemplate,
        iris::CornersChainSettings
    >;


template struct pex::MakeGroup
    <
        iris::CornersChainGroup,
        iris::CornersChainModel
    >;
