#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <wxpex/app.h>
#include <wxpex/wxshim_app.h>
#include <wxpex/file_field.h>

#include <iris/views/mask_brain.h>

#include "../common/about_window.h"
#include "../common/observer.h"
#include "../common/gray_png_brain.h"
#include "../common/png_settings.h"
#include "../common/display_thread.h"

#include "demo_settings.h"
#include "demo_controls.h"
#include "filters.h"


class DemoBrain: public GrayPngBrain<DemoBrain>
{
public:
    using Base = GrayPngBrain<DemoBrain>;

    DemoBrain()
        :
        Base(),
        observer_(this, UserControl(this->user_)),
        demoModel_(),

        maskBrain_(
            iris::MaskControl(this->demoModel_.mask),
            this->userControl_.pixelView),

        demoEndpoint_(
            this,
            DemoControl(this->demoModel_),
            &DemoBrain::OnSettings_),

        filters_(
            iris::CancelControl(this->cancel_),
            DemoControl(this->demoModel_)),

        mutex_(),

        displayThread_(
            this->userControl_.pixelView.asyncPixels,
            iris::CancelControl(this->cancel_),
            std::bind(&DemoBrain::Process, this))
    {
        this->demoModel_.maximum.Set(pngMaximum);
        this->demoModel_.color.range.high.Set(pngMaximum);
    }

    std::string GetAppName() const
    {
        return "Vertex Demo";
    }

    void SetPngData(const SourceType &data)
    {
        this->filters_.source.SetData(data);
    }

    wxWindow * CreateControls(wxWindow *parent)
    {
        return new DemoControls(
            parent,
            this->GetUserControls(),
            DemoControl(this->demoModel_));
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
        wxAboutBox(MakeAboutDialogInfo("Vertex Demo"));
    }

    std::shared_ptr<draw::Pixels>
    MakePixels(const iris::ProcessMatrix &value) const
    {
        return this->filters_.color.Filter(value);
    }

    std::shared_ptr<draw::Pixels> Process()
    {
        this->userControl_.pixelView.asyncShapes.Set(
            draw::Shapes::MakeResetter());

        auto vertexResult = this->filters_.vertexChain.GetChainResults();

        if (!vertexResult)
        {
            auto levelResult = this->filters_.level.GetResult();

            if (!levelResult)
            {
                return {};
            }

            return this->MakePixels(*levelResult);
        }

        return vertexResult->Display(
            this->userControl_.pixelView.asyncShapes,
            this->demoModel_.vertexChain.shape.Get(),
            this->filters_.color);
    }

    void Display()
    {
        this->displayThread_.Display();
    }

    void Shutdown()
    {
        this->displayThread_.Shutdown();
        this->GrayPngBrain<DemoBrain>::Shutdown();
    }

private:
    void OnSettings_(const DemoSettings &)
    {
        if (this->png_)
        {
            this->Display();
        }
    }

private:
    Observer<DemoBrain> observer_;
    DemoModel demoModel_;
    iris::MaskBrain maskBrain_;
    pex::Endpoint<DemoBrain, DemoControl> demoEndpoint_;
    Filters filters_;
    mutable std::mutex mutex_;
    DisplayThread displayThread_;
};


// Creates the main function for us, and initializes the app's run loop.
wxshimAPP(wxpex::App<DemoBrain>)
