#pragma once


#include <wxpex/wxshim.h>
#include <wxpex/labeled_widget.h>
#include <wxpex/file_field.h>
#include <wxpex/slider.h>
#include "iris/projection_settings.h"
#include "iris/views/make_read_write_buttons.h"


namespace iris
{


template<typename T>
class PoseView: public wxControl
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    PoseView(
        wxWindow *parent,
        file::PoseControl<T> controls,
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

        FileDialogOptions options{};
        options.style = wxFD_OPEN;
        options.wildcard = std::string("*") + file::poseExtension;

        auto fileSelector = new wxpex::FileField(
            this,
            control.fileName,
            options);

        auto controlsSizer = LayoutLabeled(
            layoutOptions,
            yaw,
            pitch,
            roll,
            axisOrder,
            x_m,
            y_m,
            z_m);

        auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
        sizer->Add(controlsSizer.release(), 1, wxEXPAND | wxBOTTOM, 3);
        sizer->Add(fileSelector, 0, wxEXPAND | wxBOTTOM, 3);

        sizer->Add(
            MakeReadWriteButtons(this, control).release(),
            0,
            wxALIGN_CENTER);

        this->SetSizerAndFit(sizer.release());
    }
};


} // end namespace iris
