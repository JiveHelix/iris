#include "shape_view.h"
#include <wx/dcbuffer.h>


namespace iris
{


ShapeCanvas::ShapeCanvas(
    wxWindow *parent,
    ShapesViewControl controls)
    :
    DataCanvas<Shapes, ShapeCanvas>(parent, controls),
    bitmap_(),
    shapesTerminus_(this, controls.data),
    shapes_()
{
    this->Bind(wxEVT_PAINT, &ShapeCanvas::OnPaint_, this);
    this->shapesTerminus_.Connect(&ShapeCanvas::OnShapes_);
}


bool ShapeCanvas::PrepareDrawing()
{
    wxSize size = this->GetClientSize();
    this->bitmap_ = wxBitmap(size.GetWidth(), size.GetHeight());
    wxMemoryDC memoryDc(this->bitmap_);
    return this->Draw_(memoryDc);
}


void ShapeCanvas::OnShapes_(const Shapes &shapes)
{
    this->shapes_ = shapes;

    if (this->control_.buffered)
    {
        if (!this->PrepareDrawing())
        {
            // There is nothing to update.
            return;
        }
    }

    this->Refresh(false);
    this->Update();
}


void ShapeCanvas::OnPaint_(wxPaintEvent &)
{
    if (this->control_.buffered)
    {
        wxBufferedPaintDC(this, this->bitmap_);
    }
    else
    {
        wxPaintDC dc(this);
        this->Draw_(dc);
    }
}


} // end namespace iris
