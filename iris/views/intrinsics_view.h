#pragma once


#include <wxpex/wxshim.h>
#include <wxpex/labeled_widget.h>
#include "iris/projection_settings.h"


namespace iris
{


template<typename T>
class IntrinsicsView: public wxControl
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    IntrinsicsView(
        wxWindow *parent,
        file::IntrinsicsControl<T> controls,
        const LayoutOptions &layoutOptions = LayoutOptions{})
        :
        wxControl(parent, wxID_ANY)
    {
        using namespace wxpex;

        auto pixelSize_um = LabeledWidget(
            this,
            "Pixel Size (um)",
            new Field(
                this,
                control.settings.pixelSize_um));

        auto focalLengthX_mm = LabeledWidget(
            this,
            "Focal Length X (mm)",
            new Field(
                this,
                control.settings.focalLengthX_mm));

        auto focalLengthY_mm = LabeledWidget(
            this,
            "Focal Length Y (mm)",
            new Field(
                this,
                control.settings.focalLengthY_mm));

        auto principalX_pixels = LabeledWidget(
            this,
            "Principal Point X (pixels)",
            new Field(
                this,
                control.settings.principalX_pixels));

        auto principalY_pixels = LabeledWidget(
            this,
            "Principal Point Y (pixels)",
            new Field(
                this,
                control.settings.principalY_pixels));

        auto skew = LabeledWidget(
            this,
            "Skew",
            new Field(
                this,
                control.settings.skew));

        FileDialogOptions options{};
        options.style = wxFD_OPEN;
        options.wildcard = std::string("*") + file::intrinsicsExtension;

        auto fileSelector = new wxpex::FileField(
            this,
            control.fileName,
            options);

        auto controlsSizer = LayoutLabeled(
            layoutOptions,
            pixelSize_um,
            focalLengthX_mm,
            focalLengthY_mm,
            principalX_pixels,
            principalY_pixels,
            skew);

        auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
        sizer->Add(controlsSizer.release(), 0, wxEXPAND | wxBOTTOM, 3);
        sizer->Add(fileSelector, 0, wxEXPAND | wxBOTTOM, 3);

        sizer->Add(
            MakeReadWriteButtons(this, control).release(),
            0,
            wxALIGN_CENTER);

        this->SetSizerAndFit(sizer.release());
    }
};


} // end namespace iris
