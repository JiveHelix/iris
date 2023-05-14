#pragma once


#include <wxpex/ignores.h>

WXSHIM_PUSH_IGNORES
#include <wx/display.h>
WXSHIM_POP_IGNORES

#include <iris/views/pixel_view.h>
#include <iris/png.h>
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

    void LayoutWindows()
    {
        if (!this->controlFrame_)
        {
            this->controlFrame_ = this->GetDerived()->CreateControlFrame();

            // Allow the window to fully initialize before layout continues.
            this->doLayoutWindows_();
            return;
        }

        if (!this->dataView_)
        {
            this->GetDerived()->CreateDataView_();

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

        auto dataView = this->dataView_.Get();

        auto display = wxDisplay(static_cast<unsigned int>(displayIndex));
        auto clientArea = display.GetClientArea();
        auto topLeft = wxpex::ToPoint<int>(clientArea.GetTopLeft());
        auto clientSize = wxpex::ToSize<int>(clientArea.GetSize());
        auto controlFrameSize = wxpex::ToSize<int>(controlFrame->GetSize());
        auto remainingWidth = clientSize.width - controlFrameSize.width;

        controlFrame->SetPosition(wxpex::ToWxPoint(topLeft));
        topLeft.x += controlFrameSize.width;

        auto asFrame = dynamic_cast<wxFrame *>(dataView);

        if (asFrame)
        {
            if (asFrame->IsMaximized())
            {
                std::cout << "DataView came up maximized." << std::endl;
                asFrame->Maximize(false);
            }
        }

        auto dataViewSize = wxpex::ToSize<int>(dataView->GetSize());
        dataViewSize.height = clientSize.height;
        dataViewSize.width = remainingWidth;

        dataView->SetPosition(wxpex::ToWxPoint(topLeft));
        dataView->SetSize(wxpex::ToWxSize(dataViewSize));

        this->GetDerived()->Display();
    }

    void OpenFile()
    {
        // Open PNG file, and read data into Eigen matrix.
        // Display pixel view.
        iris::Png<Pixel> png = iris::Png<Pixel>(this->user_.fileName.Get());

        this->GetDerived()->LoadPng(png);

        if (!this->dataView_)
        {
            this->GetDerived()->CreateDataView_();
        }

        this->user_.pixelView.viewSettings.imageSize.Set(png.GetSize());
        this->LayoutWindows();
        this->user_.pixelView.viewSettings.FitZoom();

        this->GetDerived()->Display();
    }

    void Shutdown()
    {
        this->controlFrame_.Close();
        this->dataView_.Close();
    }

protected:
    void CreateDataView_()
    {
        this->dataView_ = {
            new iris::PixelView(
                nullptr,
                iris::PixelViewControl(this->user_.pixelView),
                "View"),
            MakeShortcuts(this->GetUserControls())};

        this->dataView_.Get()->Show();
    }

protected:
    UserModel user_;
    UserControl userControl_;
    wxpex::CallAfter doLayoutWindows_;
    wxpex::Window controlFrame_;
    wxpex::ShortcutWindow dataView_;
};
