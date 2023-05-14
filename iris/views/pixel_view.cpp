#include "pixel_view.h"
#include <wx/dcbuffer.h>


namespace iris
{


PixelCanvas::PixelCanvas(
    wxWindow *parent,
    PixelViewControl controls)
    :
    DataCanvas<Pixels, PixelCanvas>(parent, controls),
    bitmap_(),
    pixelsTerminus_(this, controls.data),
    pixelData_()
{
    this->Bind(wxEVT_PAINT, &PixelCanvas::OnPaint_, this);
    this->pixelsTerminus_.Connect(&PixelCanvas::OnPixels_);
}


bool PixelCanvas::PrepareDrawing()
{
    wxSize size = this->GetClientSize();
    this->bitmap_ = wxBitmap(size.GetWidth(), size.GetHeight());
    wxMemoryDC memoryDc(this->bitmap_);
    return this->Draw_(memoryDc);
}


void PixelCanvas::OnPixels_(const Pixels &pixels)
{
    this->pixelData_ = pixels;
    auto imageSize = wxpex::ToSize<Pixels::Index>(this->image_.GetSize());

    if (imageSize != this->pixelData_.size)
    {
        this->image_ = wxImage(imageSize.width, imageSize.height, false);
    }

    this->image_.SetData(this->pixelData_.data.data(), true);

    if (this->control_.buffered.Get())
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


void PixelCanvas::OnPaint_(wxPaintEvent &)
{
    if (this->control_.buffered.Get())
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
