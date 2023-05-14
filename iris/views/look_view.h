#pragma once


#include <wxpex/labeled_widget.h>
#include <wxpex/slider.h>
#include <wxpex/field.h>
#include <wxpex/check_box.h>
#include <wxpex/color_picker.h>
#include "iris/views/look.h"



namespace iris
{


class LookView: public wxControl
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    LookView(
        wxWindow *parent,
        LookControl control,
        const LayoutOptions &layoutOptions)
        :
        wxControl(parent, wxID_ANY)
    {
        using namespace wxpex;

        auto strokeEnable = wxpex::LabeledWidget(
            this,
            "Stroke Enable",
            new CheckBox(this, "", control.strokeEnable));

        auto strokeWeight = wxpex::LabeledWidget(
            this,
            "Stroke Weight",
            new FieldSlider(
                this,
                control.strokeWeight,
                control.strokeWeight.value));

        auto strokeColor = wxpex::LabeledWidget(
            this,
            "Stroke Color",
            new HsvPicker(
                this,
                control.strokeColor));

        auto fillEnable = wxpex::LabeledWidget(
            this,
            "Fill Enable",
            new CheckBox(this, "", control.fillEnable));

        auto fillColor = wxpex::LabeledWidget(
            this,
            "Fill Color",
            new HsvPicker(
                this,
                control.fillColor));

        auto antialias = wxpex::LabeledWidget(
            this,
            "Anti-alias",
            new CheckBox(this, "", control.antialias));

        auto sizer = wxpex::LayoutLabeled(
            layoutOptions,
            strokeEnable,
            strokeWeight,
            strokeColor,
            fillEnable,
            fillColor,
            antialias);

        this->SetSizerAndFit(sizer.release());
    }
};


} // end namespace iris
