#pragma once


#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>

#include <draw/pixels.h>


#include <iris/views/mask_brain.h>

#include "../common/about_window.h"
#include "../common/observer.h"
#include "../common/brain.h"

#include "./demo_settings.h"
#include "./main_frame.h"
#include "./filters.h"


enum class DisplayState
{
    waiting,
    processing
};


class DemoBrain: public Brain<DemoBrain>
{
public:
    DemoBrain();

    void LoadPng(const draw::Png<Pixel> &png);

    wxpex::Window CreateControlFrame();

    void SaveSettings() const;

    void LoadSettings();

    void ShowAbout();

    std::shared_ptr<draw::Pixels>
    MakePixels(const iris::ProcessMatrix &value) const;

    std::shared_ptr<draw::Pixels> Process();

    void Display();

    void Shutdown();

private:
    void OnSettings_(const DemoSettings &);

    void DisplayLoop_();

private:
    Observer<DemoBrain> observer_;
    DemoModel demoModel_;
    iris::MaskBrain maskBrain_;
    pex::Endpoint<DemoBrain, DemoControl> demoEndpoint_;
    bool pngIsLoaded_;
    Filters filters_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    DisplayState displayState_;
    std::atomic_bool displayRequested_;
    std::atomic_bool displayLoopIsRunning_;
    std::thread displayThread_;
};
