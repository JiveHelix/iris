#include "demo_settings_view.h"

#include <iris/views/level_settings_view.h>
#include <iris/views/mask_settings_view.h>
#include <iris/views/lines_chain_settings_view.h>
#include <iris/views/color_settings_view.h>


DemoSettingsView::DemoSettingsView(
    wxWindow *parent,
    DemoControl control)
    :
    wxpex::Scrolled(parent)
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

    auto lines = new iris::LinesChainSettingsView(
        this,
        control.lines,
        layoutOptions);

    lines->Expand();

    auto color = new iris::ColorSettingsView<int32_t>(
        this,
        control.color,
        layoutOptions);

    auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);

    sizer->Add(mask, 0, wxEXPAND | wxBOTTOM, 5);
    sizer->Add(levels, 0, wxEXPAND | wxBOTTOM, 5);
    sizer->Add(lines, 0, wxEXPAND | wxBOTTOM, 5);
    sizer->Add(color, 0, wxEXPAND | wxBOTTOM, 5);

    auto borderSizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
    borderSizer->Add(sizer.release(), 1, wxEXPAND | wxALL, 5);

    this->ConfigureTopSizer(wxpex::verticalScrolled, std::move(borderSizer));
};
