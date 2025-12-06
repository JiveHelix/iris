#pragma once

// #define LOAD_PNG_HIGH

#ifdef LOAD_PNG_HIGH

using PngPixel = int32_t;
static constexpr int32_t pngMaximum = 65535;

#else

using PngPixel = uint8_t;
static constexpr int32_t pngMaximum = 255;

#endif
