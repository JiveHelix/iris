#include "iris/views/draw_points.h"


namespace iris
{


void DrawPoints(const CornerPoints &points, wxDC &dc, int radius)
{
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(wxBrush(*wxWHITE));

    for (auto &point: points)
    {
        auto rounded = point.point.Convert<int>();
        dc.DrawCircle(rounded.x, rounded.y, radius);
    }
}


} // end namespace iris
