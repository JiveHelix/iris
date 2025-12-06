#pragma once


#include <wxpex/collapsible.h>
#include <wxpex/labeled_widget.h>
#include "iris/views/chess_shape.h"


namespace iris
{


class ChessShapeView: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    ChessShapeView(
        wxWindow *parent,
        const ChessShapeControl &controls,
        const LayoutOptions &layoutOptions = LayoutOptions{});
};


} // end namespace iris
