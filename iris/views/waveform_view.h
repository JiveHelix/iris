#pragma once


#include <mutex>
#include <pex/value.h>
#include <wxpex/wxshim.h>

#include "iris/pixels.h"
#include "iris/waveform_settings.h"


namespace iris
{


class WaveformPixels: public wxPanel
{
public:
    static constexpr auto observerName = "WaveformPixels";

    WaveformPixels(
        wxWindow *parent,
        PixelsControl pixelsControl,
        WaveformControl waveformControl);

    Eigen::Vector<wxCoord, Eigen::Dynamic> GetLines() const;

private:
    void OnPaint_(wxPaintEvent &);

    void OnPixels_(const Pixels &pixels);

    void OnSize_(wxSizeEvent &);

private:
    PixelsTerminus<WaveformPixels> waveformPixels_;
    WaveformControl waveformControl_;
    Pixels waveformData_;
    wxImage image_;
};


class WaveformView: public wxFrame
{
public:
    static constexpr auto observerName = "WaveformView";

    WaveformView(
        wxWindow *parent,
        PixelsControl pixelsControl,
        WaveformControl waveformControl,
        const std::string &title);

private:
    void OnPaint_(wxPaintEvent &);

    void OnWaveformSize_(size_t);

    void OnWaveformVerticalScale_(double);

private:
    WaveformPixels *waveformPixels_;
    WaveformTerminus<WaveformView> waveformTerminus_;
};


} // end namespace iris
