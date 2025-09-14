#pragma once


#include <wxpex/wxshim.h>
#include <wxpex/labeled_widget.h>
#include <wxpex/layout_items.h>
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

        auto yaw_deg = LabeledWidget(
            this,
            "Yaw (z) (deg)",
            wxpex::CreateFieldSlider<3>(
                this,
                control.rotation.yaw_deg));

        auto pitch_deg = LabeledWidget(
            this,
            "Pitch (y) (deg)",
            wxpex::CreateFieldSlider<3>(
                this,
                control.rotation.pitch_deg));

        auto roll_deg = LabeledWidget(
            this,
            "Roll (x) (deg)",
            wxpex::CreateFieldSlider<3>(
                this,
                control.rotation.roll_deg));

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
                control.point_m.x));

        auto y_m = LabeledWidget(
            this,
            "y (m)",
            new Field(
                this,
                control.point_m.y));

        auto z_m = LabeledWidget(
            this,
            "z (m)",
            new Field(
                this,
                control.point_m.z));

        auto sizer = LayoutLabeled(
            layoutOptions,
            yaw_deg,
            pitch_deg,
            roll_deg,
            axisOrder,
            x_m,
            y_m,
            z_m);

        this->SetSizerAndFit(sizer.release());
    }
};


} // end namespace iris
