#pragma once


#include <wxpex/labeled_widget.h>
#include <wxpex/slider.h>
#include <wxpex/field.h>
#include <wxpex/knob.h>

#include "iris/views/point_view.h"
#include "iris/ellipse.h"



namespace iris
{


template<typename Value>
class EllipseView: public wxControl
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    EllipseView(
        wxWindow *parent,
        EllipseControl<Value> control,
        const LayoutOptions &layoutOptions)
        :
        wxControl(parent, wxID_ANY)
    {
        using namespace wxpex;

        auto center = wxpex::LabeledWidget(
            this,
            "center",
            new PointView<Value>(this, control.center));

        auto major = wxpex::LabeledWidget(
            this,
            "major",
            wxpex::CreateFieldSlider<1>(
                this,
                control.major,
                control.major.value));

        auto minor = wxpex::LabeledWidget(
            this,
            "minor",
            wxpex::CreateFieldSlider<1>(
                this,
                control.minor,
                control.minor.value));

        auto angle = wxpex::LabeledWidget(
            this,
            "angle",
            wxpex::CreateFieldKnob<1>(
                this,
                control.angle,
                control.angle.value,
                KnobSettings().StartAngle(0.0).Continuous(true)));

        auto scale = wxpex::LabeledWidget(
            this,
            "scale",
            new iris::ScaleSlider(
                this,
                control.scale,
                control.scale.value));

        auto sizer = wxpex::LayoutLabeled(
            layoutOptions,
            center,
            major,
            minor,
            angle,
            scale);

        this->SetSizerAndFit(sizer.release());
    }
};


} // end namespace iris
