#pragma once


#include <mutex>
#include <pex/value.h>
#include <wxpex/button.h>
#include <wxpex/check_box.h>
#include <wx/scrolwin.h>

#include "iris/views/scale.h"
#include "iris/views/data_view_settings.h"


namespace iris
{


using Scrolled = wxScrolled<wxPanel>;

template<typename Data, typename Derived>
class DataCanvas: public Scrolled
{
public:
    static constexpr auto observerName = "DataCanvas";
    static constexpr int pixelsPerScrollUnit = 10;

    DataCanvas(
        wxWindow *parent,
        DataViewControl<Data> controls)
        :
        Scrolled(parent, wxID_ANY),
        ignoreViewPosition_(false),
        skipUpdateViewPosition_(false),
        imageSizeTerminus_(this, controls.viewSettings.imageSize),
        viewSizeTerminus_(this, controls.viewSettings.viewSize),
        viewPositionTerminus_(this, controls.viewSettings.viewPosition),
        scaleTerminus_(this, controls.viewSettings.scale),
        control_(controls),
        image_(
            this->imageSizeTerminus_.Get().width,
            this->imageSizeTerminus_.Get().height)
    {
        if (controls.buffered.Get())
        {
            this->SetBackgroundStyle(wxBG_STYLE_PAINT);
        }

        this->Bind(wxEVT_SIZE, &DataCanvas::OnSize_, this);

        this->Bind(
            wxEVT_MOTION,
            &DataCanvas::OnMouseMotion_,
            this);

        this->Bind(
            wxEVT_LEFT_DOWN,
            &DataCanvas::OnLeftDown_,
            this);

        this->Bind(
            wxEVT_LEFT_UP,
            &DataCanvas::OnLeftUp_,
            this);

        this->imageSizeTerminus_.Connect(&DataCanvas::OnImageSize_);
        this->scaleTerminus_.Connect(&DataCanvas::OnScale_);
        this->viewPositionTerminus_.Connect(&DataCanvas::OnViewPosition_);

        // Configure the scrollable area
        this->SizeVirtualPanel_(this->scaleTerminus_.Get());
    }

    Derived * GetDerived()
    {
        return static_cast<Derived *>(this);
    }

    void ScrollWindow(int dx, int dy, const wxRect *rect) override
    {
        this->Scrolled::ScrollWindow(dx, dy, rect);

        if (this->skipUpdateViewPosition_)
        {
            return;
        }

        this->ignoreViewPosition_ = true;
        auto delta = tau::Point2d<int>(-dx, -dy);
        auto position = this->viewPositionTerminus_.Get();
        this->viewPositionTerminus_.Set(position + delta);
        this->ignoreViewPosition_ = false;

        if (this->control_.buffered.Get())
        {
            this->GetDerived()->PrepareDrawing();
        }
    }

    Size GetVirtualSize() const
    {
        return this->virtualSize_;
    }

private:
    void OnImageSize_(const Size &imageSize)
    {
        this->image_ = wxImage(imageSize.width, imageSize.height, true);
        this->SizeVirtualPanel_(this->scaleTerminus_.Get());
    }

    void OnSize_(wxSizeEvent &event)
    {
        event.Skip();
        this->viewSizeTerminus_.Set(wxpex::ToSize<int>(this->GetClientSize()));
    }

    void OnMouseMotion_(wxMouseEvent &event)
    {
        event.Skip();
        this->control_.mousePosition.Set(
            wxpex::ToPoint<int>(event.GetPosition()));
    }

    void OnLeftDown_(wxMouseEvent &event)
    {
        event.Skip();
        this->control_.mouseDown.Set(true);
        this->control_.mousePosition.Set(wxpex::ToPoint<int>(event.GetPosition()));
    }

    void OnLeftUp_(wxMouseEvent &event)
    {
        event.Skip();
        this->control_.mouseDown.Set(false);
        this->control_.mousePosition.Set(
            wxpex::ToPoint<int>(event.GetPosition()));
    }

    void OnScale_(const Scale &scale)
    {
        this->SizeVirtualPanel_(scale);

        if (this->control_.buffered.Get())
        {
            this->GetDerived()->PrepareDrawing();
        }

        this->Refresh(false);
        this->Update();
    }

    void OnViewPosition_(const Point &viewPosition)
    {
        // The view position has been set from the model.
        if (this->ignoreViewPosition_)
        {
            // Ignoring because this viewPosition originated with this class
            // and is being echoed from the model.
            return;
        }

        if (this->control_.buffered.Get())
        {
            this->GetDerived()->PrepareDrawing();
        }

        // Scroll to the correct position, but do not allow ScrollWindow to
        // publish a redundant viewPosition.
        this->skipUpdateViewPosition_ = true;
        auto newViewStart = viewPosition / pixelsPerScrollUnit;

        this->Scroll(wxpex::ToWxPoint(newViewStart));
        this->skipUpdateViewPosition_ = false;
    }

    void SizeVirtualPanel_(const Scale &scale)
    {
        auto imageSize = this->imageSizeTerminus_.Get();
        auto virtualSize = imageSize.template Convert<double>();

        virtualSize.width *= scale.horizontal;
        virtualSize.height *= scale.vertical;
        this->virtualSize_ = virtualSize.template Convert<int, tau::Floor>();

        auto unitCount = this->virtualSize_ / DataCanvas::pixelsPerScrollUnit;

        auto positionInUnits =
            this->viewPositionTerminus_.Get() / pixelsPerScrollUnit;

        this->skipUpdateViewPosition_ = true;

        this->SetScrollbars(
            DataCanvas::pixelsPerScrollUnit,
            DataCanvas::pixelsPerScrollUnit,
            unitCount.width,
            unitCount.height,
            positionInUnits.x,
            positionInUnits.y);

        this->skipUpdateViewPosition_ = false;
    }

private:
    bool ignoreViewPosition_;
    bool skipUpdateViewPosition_;

protected:
    SizeTerminus<DataCanvas> imageSizeTerminus_;
    SizeGroup::Terminus<DataCanvas> viewSizeTerminus_;
    PointGroup::Terminus<DataCanvas> viewPositionTerminus_;

    ScaleGroup::Terminus<DataCanvas> scaleTerminus_;

    DataViewControl<Data> control_;
    wxImage image_;

private:
    Size virtualSize_;
};



template<typename Data, typename Canvas>
class DataView: public wxFrame
{
    static constexpr int margin = 5;
    static constexpr int gridSpacing = 3;

public:
    static constexpr auto observerName = "DataView";

    DataView(
        wxWindow *parent,
        DataViewControl<Data> controls,
        const std::string &title)
        :
        wxFrame(parent, wxID_ANY, title),
        imageSizeTerminus_(this, controls.viewSettings.imageSize),
        horizontalZoom_(
            new ScaleSlider(
                this,
                controls.viewSettings.scale.horizontal,
                controls.viewSettings.scale.horizontal.value,
                wxpex::Style::horizontal)),
        verticalZoom_(
            new ScaleSlider(
                this,
                controls.viewSettings.scale.vertical,
                controls.viewSettings.scale.vertical.value,
                wxpex::Style::vertical)),
        controlsSizer_(new wxBoxSizer(wxHORIZONTAL))
    {
        this->imageSizeTerminus_.Connect(&DataView::OnImageSize_);

        this->canvas_ = new Canvas(this, controls);

        auto zoomLabel = new wxStaticText(this, wxID_ANY, "Scale");

        auto resetZoom =
            new wxpex::Button(this, "Reset", controls.viewSettings.resetZoom);

        auto fitZoom =
            new wxpex::Button(this, "Fit", controls.viewSettings.fitZoom);

        auto linkZoom =
            new wxpex::CheckBox(this, "Link", controls.viewSettings.linkZoom);

        this->controlsSizer_->Add(this->horizontalZoom_, 0, wxRIGHT, 3);
        this->controlsSizer_->Add(resetZoom, 0, wxRIGHT, 3);
        this->controlsSizer_->Add(fitZoom, 0, wxRIGHT, 3);
        this->controlsSizer_->Add(linkZoom, 0, wxRIGHT);

        auto gridSizer = new wxFlexGridSizer(2, gridSpacing, gridSpacing);

        gridSizer->SetFlexibleDirection(wxBOTH);
        gridSizer->AddGrowableRow(1, 1);
        gridSizer->AddGrowableCol(1, 1);

        gridSizer->Add(zoomLabel, 0);
        gridSizer->Add(this->controlsSizer_, 0);
        gridSizer->Add(this->verticalZoom_, 1);
        gridSizer->Add(this->canvas_, 1, wxEXPAND);

        auto sizer = new wxBoxSizer(wxHORIZONTAL);
        sizer->Add(gridSizer, 1, wxALL | wxEXPAND, margin);
        this->SetSizer(sizer);
    }

    wxSize DoGetBestSize() const override
    {
        auto result =
            this->GetWindowSize_(this->canvas_->GetVirtualSize());

        return wxpex::ToWxSize(result);
    }


private:
    void OnImageSize_(const Size &imageSize)
    {
        // Recalculate window size based on new image size.
        auto size = this->GetWindowSize_(imageSize);
        this->SetClientSize(wxpex::ToWxSize(size));
    }

    Size GetWindowSize_(const Size &canvasSize) const
    {
        // Recalculate window size based on canvas size.
        auto verticalZoomSize =
            wxpex::ToSize<int>(this->verticalZoom_->GetBestSize());

        auto controlsSize = wxpex::ToSize<int>(this->controlsSizer_->GetMinSize());

        Size size(
            std::max(canvasSize.width, controlsSize.width)
                + verticalZoomSize.width,
            std::max(canvasSize.height, verticalZoomSize.height)
                + controlsSize.height);

        auto scrollbarPadding = 20;
        auto extra = gridSpacing + (margin * 2) + scrollbarPadding;
        size += extra;

        return size;
    }

private:
    SizeTerminus<DataView> imageSizeTerminus_;
    wxWindow *horizontalZoom_;
    wxWindow *verticalZoom_;
    wxBoxSizer *controlsSizer_;
    Canvas *canvas_;
};


} // end namespace iris
