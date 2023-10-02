#include "demo_controls.h"
#include "demo_settings_view.h"


#include <wxpex/file_field.h>


DemoControls::DemoControls(
    wxWindow *parent,
    UserControl userControl,
    DemoControl control)
    :
    wxPanel(parent, wxID_ANY)
{
    auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);

    wxpex::FileDialogOptions options{};
    options.message = "Choose a PNG file";
    options.wildcard = "*.png";

    auto fileSelector = new wxpex::FileField(
        this,
        userControl.fileName,
        options);

    auto demoSettingsView = new DemoSettingsView(
        this,
        control);

    sizer->Add(fileSelector, 0, wxEXPAND | wxBOTTOM, 5);
    sizer->Add(demoSettingsView, 1, wxEXPAND);
    auto topSizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
    topSizer->Add(sizer.release(), 1, wxEXPAND | wxALL, 5);
    this->SetSizerAndFit(topSizer.release());
}
