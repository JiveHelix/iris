#pragma once


#include <mutex>
#include <pex/value.h>
#include <wxpex/wxshim.h>
#include <wx/scrolwin.h>

#include <tau/view.h>

#include "iris/views/data_view.h"
#include "iris/views/pixel_view_settings.h"
#include "iris/pixels.h"


namespace iris
{


class PixelCanvas: public DataCanvas<Pixels, PixelCanvas>
{
public:
    PixelCanvas(
        wxWindow *parent,
        PixelViewControl controls);

    bool PrepareDrawing();

private:
    template<typename Context>
    bool Draw_(Context &&context)
    {
        auto bitmap = wxBitmap(this->image_);
        auto source = wxMemoryDC(bitmap);

        auto viewRegion = tau::Region<int>{{
            this->viewPositionTerminus_.Get(),
            this->viewSizeTerminus_.Get()}};

        auto view = tau::View<int>(
            viewRegion,
            this->imageSizeTerminus_.Get(),
            this->scaleTerminus_.Get());

        if (!view.HasArea())
        {
            return false;
        }

        context.StretchBlit(
            view.target.topLeft.x,
            view.target.topLeft.y,
            view.target.size.width,
            view.target.size.height,
            &source,
            view.source.topLeft.x,
            view.source.topLeft.y,
            view.source.size.width,
            view.source.size.height);

        return true;
    }

    void OnPixels_(const Pixels &pixels);

    void OnPaint_(wxPaintEvent &);

private:
    wxBitmap bitmap_;
    PixelsTerminus<PixelCanvas> pixelsTerminus_;
    Pixels pixelData_;
};


using PixelView = DataView<Pixels, PixelCanvas>;


} // end namespace iris
