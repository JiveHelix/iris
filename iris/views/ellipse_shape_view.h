#pragma once


#include <wxpex/collapsible.h>
#include "iris/views/ellipse_view.h"
#include "iris/views/look_view.h"
#include "iris/views/ellipse_shape.h"


namespace iris
{


class EllipseShapeView: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    EllipseShapeView(
        wxWindow *parent,
        EllipseShapeControl control,
        const LayoutOptions &layoutOptions)
        :
        wxpex::Collapsible(parent, "Ellipse")
    {
        using namespace wxpex;

        auto ellipse = wxpex::LabeledWidget(
            this->GetPane(),
            "Ellipse",
            new EllipseView<double>(
                this->GetPane(),
                control.ellipse,
                layoutOptions));

        auto look = wxpex::LabeledWidget(
            this->GetPane(),
            "Look",
            new LookView(this->GetPane(), control.look, layoutOptions));

        auto sizer = wxpex::LayoutLabeled(
            layoutOptions,
            ellipse,
            look);

        this->ConfigureTopSizer(sizer.release());
    }
};


} // end namespace iris
