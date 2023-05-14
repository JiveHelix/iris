#pragma once


#include <wxpex/labeled_widget.h>
#include <pex/value.h>
#include <wx/listctrl.h>

#include "iris/views/pixel_view_settings.h"


namespace iris
{


template<typename ValuesControl>
class PixelValue: public wxStaticText
{
public:
    using Values = typename ValuesControl::Type;

    PixelValue(wxWindow *parent, ValuesControl values, PointControl point)
        :
        wxStaticText(parent, wxID_ANY, "    "),
        values_(this, values),
        point_(this, point)
    {
        this->values_.Connect(&PixelValue::OnValues_);
        this->point_.Connect(&PixelValue::OnPoint_);
    }

    void OnValues_(const Values &values)
    {
        this->Display(values, this->point_.Get());
    }

    void OnPoint_(const Point &point)
    {
        this->Display(this->values_.Get(), point);
    }

    void Display(const Values &values, const Point &point)
    {
        if (Size(values).Contains(point))
        {
            this->SetLabelText(std::to_string(values(point.y, point.x)));
            this->Show(true);
        }
        else
        {
            this->Show(false);
        }
    }

    pex::Terminus<PixelValue, ValuesControl> values_;
    PointTerminus<PixelValue> point_;
};


class PixelInfo: public wxPanel
{
public:
    static constexpr auto observerName = "PixelInfo";

    PixelInfo(
        wxWindow *parent,
        PointControl point);

    template<typename ValuesControl>
    void AddValue(ValuesControl values, const std::string &name)
    {
        wxpex::LayoutOptions options{};
        options.labelFlags = wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL;

        auto value = new PixelValue(
            this,
            values,
            PointControl(this->point_));

        auto label = new wxStaticText(this, wxID_ANY, name);
        this->sizer_->Add(label, 0, options.labelFlags);
        this->sizer_->Add(value, 1, options.widgetFlags);
        this->GetSizer()->SetSizeHints(this);
    }

    void OnPoint_(const Point &point);

private:
    PointTerminus<PixelInfo> point_;

    wxStaticText *x_;
    wxStaticText *y_;
    wxFlexGridSizer *sizer_;
};


class PixelInfoView: public wxFrame
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    PixelInfoView(
        wxWindow *parent,
        PixelViewControl control,
        const std::string &title);

protected:
    PixelInfo *pixelInfo_;
};


} // end namespace iris
