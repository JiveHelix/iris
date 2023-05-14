#pragma once


#include <mutex>
#include <pex/value.h>
#include <wxpex/wxshim.h>
#include <wx/scrolwin.h>

#include <tau/view.h>

#include "iris/views/data_view.h"
#include "iris/views/shape_view_settings.h"
#include "iris/pixels.h"


namespace iris
{


class ShapeCanvas: public DataCanvas<Shapes, ShapeCanvas>
{
public:
    ShapeCanvas(
        wxWindow *parent,
        ShapesViewControl controls);

    bool PrepareDrawing();

private:
    template<typename Context>
    bool Draw_(Context &&context)
    {
        auto gc = wxpex::GraphicsContext(context);
        auto scale = this->scaleTerminus_.Get();
        gc->Scale(scale.horizontal, scale.vertical);

        auto viewPosition =
            this->viewPositionTerminus_.Get().template Convert<double>();

        gc->Translate(-viewPosition.x, -viewPosition.y);

#if 0
        auto viewPosition = this->viewPositionTerminus_.Get();
        auto viewSize = this->viewSizeTerminus_.Get();

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
#endif

        for (auto &shape: this->shapes_)
        {
            shape->Draw(gc);
        }

        return true;
    }

    void OnShapes_(const Shapes &shapes);

    void OnPaint_(wxPaintEvent &);

private:
    wxBitmap bitmap_;
    ShapesTerminus<ShapeCanvas> shapesTerminus_;
    Shapes shapes_;
};


using ShapesView = DataView<Shapes, ShapeCanvas>;


} // end namespace iris
