#pragma once

#include <iris/node.h>
#include "brain.h"
#include "about_window.h"


using SourceType = tau::MonoImage<iris::InProcess>;
using GrayPngSource = iris::Source<SourceType>;


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

        this->GetDerived()->SetPngData(
            png.GetValues().template cast<iris::InProcess>());
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
    iris::Cancel cancel_;
};
