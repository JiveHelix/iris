#pragma once


#include <wxpex/wxshim.h>
#include <wxpex/point.h>
#include <wxpex/size.h>


namespace iris
{


template<typename Line>
void DrawLines(
    wxDC &dc,
    const std::vector<Line> &lines)
{
    auto size = wxpex::ToSize<double>(dc.GetSize());
    tau::Scale<double> scale;
    dc.GetUserScale(&scale.horizontal, &scale.vertical);
    size *= scale;

    using Point = tau::Point2d<double>;
    auto region = tau::Region<double>{{Point(0, 0), size}};

    for (auto &line: lines)
    {
        auto endPoints = line.Intersect(region);

        if (!endPoints)
        {
            continue;
        }

        dc.DrawLine(
            wxpex::ToWxPoint(endPoints->first),
            wxpex::ToWxPoint(endPoints->second));
    }
}


} // end namespace iris
