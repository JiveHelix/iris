#include "iris/vertex_chain_settings.h"


template struct pex::Group
    <
        iris::VertexChainFields,
        iris::VertexChainTemplate,
        iris::VertexChainSettings
    >;


template struct pex::MakeGroup
    <
        iris::VertexChainGroup,
        iris::VertexChainModel
    >;
