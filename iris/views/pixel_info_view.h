#pragma once


#include <wxpex/labeled_widget.h>
#include <pex/value.h>

#include <wxpex/ignores.h>

WXSHIM_PUSH_IGNORES
#include <wx/listctrl.h>
WXSHIM_POP_IGNORES

#include <draw/views/pixel_view_settings.h>


namespace iris
{


template<typename ValuesControl>
class PixelValue: public wxStaticText
{
public:
    using Values = typename ValuesControl::Type;

    PixelValue(wxWindow *parent, ValuesControl values, draw::PointControl point)
        :
        wxStaticText(parent, wxID_ANY, "    "),
        values_(this, values, &PixelValue::OnValues_),
        point_(this, point, &PixelValue::OnPoint_)
    {

    }

    void OnValues_(const Values &values)
    {
        this->Display(values, this->point_.control.Get());
    }

    void OnPoint_(const draw::Point &point)
    {
        this->Display(this->values_.control.Get(), point);
    }

    void Display(const Values &values, const draw::Point &point)
    {
        if (draw::Size(values).Contains(point))
        {
            this->SetLabelText(std::to_string(values(point.y, point.x)));
            this->Show(true);
        }
        else
        {
            this->Show(false);
        }
    }

    pex::EndpointControl<PixelValue, ValuesControl> values_;
    pex::EndpointControl<PixelValue, draw::PointControl> point_;
};


class PixelInfo: public wxPanel
{
public:
    static constexpr auto observerName = "PixelInfo";

    PixelInfo(
        wxWindow *parent,
        draw::PointControl point);

    template<typename ValuesControl>
    void AddValue(ValuesControl values, const std::string &name)
    {
        wxpex::LayoutOptions options{};
        options.labelFlags = wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL;

        auto value = new PixelValue(
            this,
            values,
            this->pointControl_);

        auto label = new wxStaticText(this, wxID_ANY, name);
        this->sizer_->Add(label, 0, options.labelFlags);
        this->sizer_->Add(value, 1, options.widgetFlags);
        this->GetSizer()->SetSizeHints(this);
    }

    void OnPoint_(const draw::Point &point);

private:
    draw::PointControl pointControl_;
    pex::Endpoint<PixelInfo, draw::PointControl> point_;

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
        draw::PixelViewControl control,
        const std::string &title);

protected:
    PixelInfo *pixelInfo_;
};


} // end namespace iris
