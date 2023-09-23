#include "demo_brain.h"


void DemoBrain::LoadPng(const draw::Png<Pixel> &png)
{
    auto maximum = this->demoModel_.color.level.high.GetMaximum();
    auto scale = static_cast<double>(maximum);

    // Prevent drawing until new dimensions and source data are
    // synchronized.
    this->pngIsLoaded_ = false;

    this->demoModel_.Mute();
    this->demoModel_.maximum.Set(maximum);
    this->demoModel_.imageSize.Set(png.GetSize());
    this->demoModel_.Unmute();

    this->filters_.source.SetData(
        png.GetValue(scale).template cast<int32_t>().eval());

    this->pngIsLoaded_ = true;
    this->Display();
}
