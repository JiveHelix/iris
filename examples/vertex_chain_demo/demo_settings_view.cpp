#include "demo_settings_view.h"

#include <iris/views/level_settings_view.h>
#include <iris/views/mask_settings_view.h>
#include <iris/views/vertex_chain_settings_view.h>
#include <iris/views/color_settings_view.h>


DemoSettingsView::DemoSettingsView(
    wxWindow *parent,
    DemoControl control)
    :
    wxScrolled<wxPanel>(parent, wxID_ANY)
{
    wxpex::LayoutOptions layoutOptions{};
    layoutOptions.labelFlags = wxALIGN_RIGHT;

    auto mask = new iris::MaskSettingsView(
        this,
        control.mask,
        layoutOptions);

    mask->Expand();

    auto levels = new iris::LevelSettingsView<int32_t>(
        this,
        "Level",
        control.level,
        layoutOptions);

    auto vertexSettings = new iris::VertexChainSettingsView(
        this,
        control.vertexChain,
        layoutOptions);

    vertexSettings->Expand();

    auto color = new iris::ColorSettingsView<int32_t>(
        this,
        control.color,
        layoutOptions);

    auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);

    sizer->Add(mask, 0, wxEXPAND | wxBOTTOM, 5);
    sizer->Add(levels, 0, wxEXPAND | wxBOTTOM, 5);
    sizer->Add(vertexSettings, 0, wxEXPAND | wxBOTTOM, 5);
    sizer->Add(color, 0, wxEXPAND | wxBOTTOM, 5);

    auto borderSizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
    borderSizer->Add(sizer.release(), 1, wxEXPAND | wxALL, 5);
    auto topSizer = borderSizer.release();
    this->SetSizer(topSizer);
    topSizer->FitInside(this);
    this->SetScrollRate(0, 10);
    this->SetMinSize(wxSize(-1, 400));
}
