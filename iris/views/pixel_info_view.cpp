#include "iris/views/pixel_info_view.h"

#include <fmt/format.h>
#include <wxpex/labeled_widget.h>
#include <wxpex/layout_top_level.h>
#include <wxpex/slider.h>


namespace iris
{


PixelInfo::PixelInfo(
    wxWindow *parent,
    draw::PointControl pointControl)
    :
    wxPanel(parent, wxID_ANY),
    pointControl_(pointControl),
    point_(this, pointControl, &PixelInfo::OnPoint_),
    x_(new wxStaticText(this, wxID_ANY, "    ")),
    y_(new wxStaticText(this, wxID_ANY, "    "))
{
    this->SetWindowStyle(wxBORDER_SIMPLE);

    auto x = wxpex::LabeledWidget(this, "x:", this->x_);
    auto y = wxpex::LabeledWidget(this, "y:", this->y_);

    wxpex::LayoutOptions options{};
    options.labelFlags = wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL;

    this->sizer_ = wxpex::LayoutLabeled(options, x, y).release();

    auto borderSizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
    borderSizer->Add(this->sizer_, 0, wxALL, 3);

    this->SetSizerAndFit(borderSizer.release());
}


void PixelInfo::OnPoint_(const draw::Point &point)
{
    this->x_->SetLabelText(std::to_string(point.x));
    this->y_->SetLabelText(std::to_string(point.y));
}


PixelInfoView::PixelInfoView(
    wxWindow *parent,
    draw::PixelViewControl control,
    const std::string &title)
    :
    wxFrame(parent, wxID_ANY, title),
    pixelInfo_(new PixelInfo(this, control.logicalPosition))

{
    auto pixelInfo = wxpex::LabeledWidget(this, "Pixel Info", this->pixelInfo_);
    LayoutOptions layoutOptions{};
    auto sizer = wxpex::LayoutLabeled(layoutOptions, pixelInfo);
    this->SetSizerAndFit(sizer.release());
}


} // end namespace iris
