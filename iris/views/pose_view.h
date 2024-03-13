#pragma once


#include <wxpex/wxshim.h>
#include <wxpex/labeled_widget.h>
#include <wxpex/file_field.h>
#include <wxpex/slider.h>
#include <wxpex/combo_box.h>
#include <tau/pose.h>


namespace iris
{


template<typename T>
class PoseView: public wxControl
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    PoseView(
        wxWindow *parent,
        tau::PoseControl<T> control,
        const LayoutOptions &layoutOptions = LayoutOptions{})
        :
        wxControl(parent, wxID_ANY)
    {
        using namespace wxpex;

        auto yaw = LabeledWidget(
            this,
            "Yaw (z) (deg)",
            new FieldSlider(
                this,
                control.rotation.yaw,
                control.rotation.yaw.value));

        auto pitch = LabeledWidget(
            this,
            "Pitch (y) (deg)",
            new FieldSlider(
                this,
                control.rotation.pitch,
                control.rotation.pitch.value));

        auto roll = LabeledWidget(
            this,
            "Roll (x) (deg)",
            new FieldSlider(
                this,
                control.rotation.roll,
                control.rotation.roll.value));

        auto axisOrder = LabeledWidget(
            this,
            "Axis Order",
            MakeComboBox<tau::AxisOrderConverter>(
                this,
                control.rotation.axisOrder));

        auto x_m = LabeledWidget(
            this,
            "x (m)",
            new Field(
                this,
                control.x_m));

        auto y_m = LabeledWidget(
            this,
            "y (m)",
            new Field(
                this,
                control.y_m));

        auto z_m = LabeledWidget(
            this,
            "z (m)",
            new Field(
                this,
                control.z_m));

        auto sizer = LayoutLabeled(
            layoutOptions,
            yaw,
            pitch,
            roll,
            axisOrder,
            x_m,
            y_m,
            z_m);

        this->SetSizerAndFit(sizer.release());
    }
};


} // end namespace iris
