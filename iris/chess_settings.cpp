#include "iris/chess_settings.h"


template struct pex::Group
    <
        iris::VertexChessFields,
        iris::VertexChessTemplate,
        iris::VertexChessSettings
    >;


template struct pex::Group
    <
        iris::ChessFields,
        iris::ChessTemplate::template Template,
        iris::ChessSettings
    >;
