#include "demo_brain.h"


void DemoBrain::LoadPng(const draw::Png<Pixel> &png)
{
    auto maximum = 1023;

    if (sizeof(Pixel) == 1)
    {
        maximum = 255;
    }

    // Prevent drawing until new dimensions and source data are
    // synchronized.
    this->pngIsLoaded_ = false;

    this->demoModel_.Mute();
    this->demoModel_.maximum.Set(maximum);
    this->demoModel_.imageSize.Set(png.GetSize());
    this->demoModel_.Unmute();

    auto pngValues =
        png.GetValue(1.0).template cast<int32_t>().eval();

    this->filters_.source.SetData(pngValues);

    this->pngIsLoaded_ = true;
    this->Display();
}
