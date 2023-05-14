#pragma once


#include <wxpex/graphics.h>
#include <vector>


namespace iris
{


class Shape
{
public:
    virtual ~Shape()
    {

    }

    virtual void Draw(wxpex::GraphicsContext &) = 0;
};


using Shapes = std::vector<std::shared_ptr<Shape>>;

using AsyncShapes = wxpex::MakeAsync<Shapes>;
using ShapesControl = typename AsyncShapes::Control<void>;

template<typename Observer>
using ShapesTerminus = typename AsyncShapes::Terminus<Observer>;


} // end namespace iris
