#pragma once


#include "brain.h"
#include "about_window.h"


template<typename Derived>
class GrayPngBrain: public Brain<Derived>
{
public:
    GrayPngBrain()
        :
        Brain<Derived>(),
        mutex_(),
        sourceMutex_(),
        png_(),
        source_(),
        cancel_()
    {

    }

    void LoadGrayPng(const draw::GrayPng<PngPixel> &png)
    {
        auto pngSize = png.GetSize();
        std::cout << "LoadGrayPng pngSize: " << pngSize << std::endl;
        this->user_.pixelView.canvas.viewSettings.imageSize.Set(pngSize);

        std::lock_guard lock(this->sourceMutex_);
        this->png_ = png;
        this->source_.SetData(png.GetValues().template cast<iris::InProcess>());
    }

    void SaveSettings() const
    {
        std::cout << "TODO: Persist the processing settings." << std::endl;
    }

    void LoadSettings()
    {
        std::cout << "TODO: Restore the processing settings." << std::endl;
    }

    void ShowAbout()
    {
        wxAboutBox(MakeAboutDialogInfo(this->GetDerived()->GetAppName()));
    }

protected:
    mutable std::mutex mutex_;
    mutable std::mutex sourceMutex_;
    std::optional<draw::GrayPng<PngPixel>> png_;
    iris::Source<tau::MonoImage<iris::InProcess>> source_;
    iris::Cancel cancel_;
};
