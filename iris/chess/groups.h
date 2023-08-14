#pragma once


#include <list>
#include "iris/chess/line_group.h"


namespace iris
{


template<typename Line>
using Groups = std::list<LineGroup<Line>>;


} // end namespace iris
