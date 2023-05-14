#pragma once


#include <wxpex/point.h>


namespace iris
{


using PointGroup = tau::Point2dGroup<int>;
using Point = typename PointGroup::Plain;
using PointControl = typename PointGroup::Control<void>;

template<typename Observer>
using PointTerminus = typename PointGroup::Terminus<Observer>;


} // end namespace iris
