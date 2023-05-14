#pragma once

#include <pex/group.h>
#include <wxpex/async.h>

#include "iris/views/data_view_settings.h"
#include "iris/views/shapes.h"


namespace iris
{


using ShapesViewGroup = DataViewGroup<Shapes>;
using ShapesViewModel = DataViewModel<Shapes>;
using ShapesViewControl = DataViewControl<Shapes>;

template<typename Observer>
using ShapesViewTerminus = DataViewTerminus<Shapes, Observer>;

using ShapesViewGroupMaker = DataViewGroupMaker<Shapes>;


} // end namespace iris
