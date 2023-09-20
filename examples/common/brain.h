#pragma once


#include <wxpex/ignores.h>

WXSHIM_PUSH_IGNORES
#include <wx/display.h>
WXSHIM_POP_IGNORES

#include <draw/png.h>
#include <draw/views/pixel_view.h>
#include "user.h"
#include "display_error.h"


using Pixel = uint32_t;


template<typename Derived>
class Brain
{
public:
    Brain()
        :
        user_{},
        userControl_(this->user_),
        doLayoutWindows_([this](){this->LayoutWindows();})
    {

    }

    Derived * GetDerived()
    {
        return static_cast<Derived *>(this);
    }

    const Derived * GetDerived() const
    {
        return static_cast<const Derived *>(this);
    }

    UserControl GetUserControls()
    {
        return UserControl(this->user_);
    }

    void CreateFrame()
    {
        this->LayoutWindows();
    }

    void LayoutWindows()
    {
        if (!this->controlFrame_)
        {
            this->controlFrame_ = this->GetDerived()->CreateControlFrame();

            // Allow the window to fully initialize before layout continues.
            this->doLayoutWindows_();
            return;
        }

        if (!this->pixelView_)
        {
            this->GetDerived()->CreatePixelView_();

            // Allow the window to fully initialize before layout continues.
            this->doLayoutWindows_();
            return;
        }

        auto displayIndex = wxDisplay::GetFromWindow(this->controlFrame_.Get());

        if (displayIndex < 0)
        {
            DisplayError("Layout Error", "Unable to detect display");
            return;
        }

        auto controlFrame = this->controlFrame_.Get();
        controlFrame->Layout();

        auto pixelView = this->pixelView_.Get();

        auto display = wxDisplay(static_cast<unsigned int>(displayIndex));
        auto clientArea = display.GetClientArea();
        auto topLeft = wxpex::ToPoint<int>(clientArea.GetTopLeft());
        auto clientSize = wxpex::ToSize<int>(clientArea.GetSize());
        auto controlFrameSize = wxpex::ToSize<int>(controlFrame->GetSize());
        controlFrameSize.height = clientSize.height;
        auto remainingWidth = clientSize.width - controlFrameSize.width;

        controlFrame->SetPosition(wxpex::ToWxPoint(topLeft));
        controlFrame->SetSize(wxpex::ToWxSize(controlFrameSize));
        topLeft.x += controlFrameSize.width;

        auto asFrame = dynamic_cast<wxFrame *>(pixelView);

        if (asFrame)
        {
            if (asFrame->IsMaximized())
            {
                asFrame->Maximize(false);
            }
        }

        auto pixelViewSize = wxpex::ToSize<int>(pixelView->GetSize());
        pixelViewSize.height = clientSize.height;
        pixelViewSize.width = remainingWidth;

        pixelView->SetPosition(wxpex::ToWxPoint(topLeft));
        pixelView->SetSize(wxpex::ToWxSize(pixelViewSize));

        this->GetDerived()->Display();
    }

    void OpenFile()
    {
        // Open PNG file, and read data into Eigen matrix.
        // Display pixel view.
        draw::Png<Pixel> png =
            draw::Png<Pixel>(this->user_.fileName.Get(), true);

        this->GetDerived()->LoadPng(png);

        if (!this->pixelView_)
        {
            this->GetDerived()->CreatePixelView_();
        }

        this->user_.pixelView.viewSettings.imageSize.Set(png.GetSize());
        this->LayoutWindows();
        this->user_.pixelView.viewSettings.FitZoom();

        this->GetDerived()->Display();
    }

    void Shutdown()
    {
        this->controlFrame_.Close();
        this->pixelView_.Close();
    }

protected:
    void CreatePixelView_()
    {
        this->pixelView_ = {
            new draw::PixelView(
                nullptr,
                draw::PixelViewControl(this->user_.pixelView)),
            MakeShortcuts(this->GetUserControls())};

        this->pixelView_.Get()->Show();
    }

protected:
    UserModel user_;
    UserControl userControl_;
    wxpex::CallAfter doLayoutWindows_;
    wxpex::Window controlFrame_;
    wxpex::ShortcutWindow pixelView_;
};
