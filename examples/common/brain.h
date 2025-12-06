#pragma once


#include <wxpex/window.h>
#include <wxpex/splitter.h>

#include <draw/png.h>
#include <draw/open_png.h>
#include <draw/views/pixel_view.h>
#include "user.h"
#include "display_error.h"
#include "png_settings.h"


using Pixel = int32_t;


template<typename Derived>
class Brain
{
public:
    Brain()
        :
        user_{},
        userControl_(this->user_),
        applicationFrame_(new wxFrame(nullptr, wxID_ANY, "")),
        shortcuts_(
            std::make_unique<wxpex::MenuShortcuts>(
                wxpex::UnclosedWindow(this->applicationFrame_.Get()),
                MakeShortcuts(this->userControl_)))
    {
        this->applicationFrame_.Get()->SetMenuBar(
            this->shortcuts_->GetMenuBar());
    }

    void ExportPng()
    {
        std::cout << "ExportPng not implemented" << std::endl;
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
        auto splitter = new wxpex::Splitter(this->applicationFrame_.Get());
        auto controls = this->GetDerived()->CreateControls(splitter);
        auto pixelView = this->CreatePixelView_(splitter);

        splitter->SplitVerticallyLeft(
            controls,
            pixelView);

        auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
        sizer->Add(splitter, 1, wxEXPAND);
        this->applicationFrame_.Get()->SetSizerAndFit(sizer.release());

        this->applicationFrame_.Get()->SetTitle(
            this->GetDerived()->GetAppName());

        this->applicationFrame_.Get()->Maximize();
        this->GetDerived()->Display();
    }

    void OpenFile()
    {
        draw::OpenPng<PngPixel>(
            pex::StringControl(this->user_.fileName),
            draw::ViewSettingsControl(
                this->user_.pixelView.canvas.viewSettings),
            *this->GetDerived());
    }

    void Shutdown()
    {
        this->applicationFrame_.Close();
    }

protected:
    wxWindow * CreatePixelView_(wxWindow *parent)
    {
        return new draw::PixelView(
            parent,
            draw::PixelViewControl(this->user_.pixelView));
    }

protected:
    UserModel user_;
    UserControl userControl_;
    wxpex::UnclosedFrame applicationFrame_;
    std::unique_ptr<wxpex::MenuShortcuts> shortcuts_;
};
