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
    auto pointAsInt = point.template Cast<int>();
    this->x_->SetLabelText(std::to_string(pointAsInt.x));
    this->y_->SetLabelText(std::to_string(pointAsInt.y));
}


PixelInfoView::PixelInfoView(
    wxWindow *parent,
    draw::CanvasControl control,
    const std::string &label)
    :
    wxPanel(parent, wxID_ANY),
    pixelInfo_(new PixelInfo(this, control.logicalPosition))

{
    auto pixelInfo = wxpex::LabeledWidget(this, label, this->pixelInfo_);
    auto sizer = wxpex::LayoutLabeled(LayoutOptions{}, pixelInfo);
    this->SetSizerAndFit(sizer.release());
}


} // end namespace iris
