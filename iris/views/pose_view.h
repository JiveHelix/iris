#pragma once


#include <wxpex/wxshim.h>
#include <wxpex/labeled_widget.h>
#include <wxpex/file_field.h>
#include <wxpex/slider.h>
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
                control.settings.angles.yaw,
                control.settings.angles.yaw.value));

        auto pitch = LabeledWidget(
            this,
            "Pitch (y) (deg)",
            new FieldSlider(
                this,
                control.settings.angles.pitch,
                control.settings.angles.pitch.value));

        auto roll = LabeledWidget(
            this,
            "Roll (x) (deg)",
            new FieldSlider(
                this,
                control.settings.angles.roll,
                control.settings.angles.roll.value));

        auto axisOrder = LabeledWidget(
            this,
            "Axis Order",
            MakeComboBox<AxisOrderConverter>(
                this,
                control.settings.angles.axisOrder));

        auto x_m = LabeledWidget(
            this,
            "x (m)",
            new Field(
                this,
                control.settings.x_m));

        auto y_m = LabeledWidget(
            this,
            "y (m)",
            new Field(
                this,
                control.settings.y_m));

        auto z_m = LabeledWidget(
            this,
            "z (m)",
            new Field(
                this,
                control.settings.z_m));

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
