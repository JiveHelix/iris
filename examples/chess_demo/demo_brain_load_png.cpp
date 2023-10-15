#include "demo_brain.h"
#include "../common/png_settings.h"


void DemoBrain::LoadPng(const draw::GrayPng<PngPixel> &png)
{
    int32_t maximum = pngMaximum;

    // Prevent drawing until new dimensions and source data are
    // synchronized.
    this->pngIsLoaded_ = false;

    this->demoModel_.color.range.high.SetMaximum(maximum);
    this->demoModel_.color.range.high.Set(maximum);
    this->demoModel_.maximum.Set(maximum);
    this->demoModel_.imageSize.Set(png.GetSize());
    this->filters_.source.SetData(png.GetValues().template cast<int32_t>());

    this->pngIsLoaded_ = true;

    this->Display();
}
