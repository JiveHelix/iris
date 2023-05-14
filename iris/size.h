#pragma once


#include <tau/size.h>


namespace iris
{


using SizeGroup = tau::SizeGroup<int>;
using Size = typename SizeGroup::Plain;

using SizeModel = typename SizeGroup::Model;
using SizeControl = typename SizeGroup::Control<void>;

template<typename Observer>
using SizeTerminus = typename SizeGroup::Terminus<Observer>;


} // end namespace iris
