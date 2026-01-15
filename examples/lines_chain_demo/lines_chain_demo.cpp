#include <iostream>
#include <chrono>
#include <wxpex/app.h>
#include <wxpex/wxshim_app.h>

#include <draw/pixels.h>
#include <draw/png.h>

#include <iris/views/mask_brain.h>

#include "../common/about_window.h"
#include "../common/observer.h"
#include "../common/gray_png_brain.h"
#include "../common/png_settings.h"
#include "../common/display_thread.h"

#include "demo_settings.h"
#include "demo_controls.h"
#include "filters.h"


template<typename T>
struct HoughUserFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::houghView, "houghView"));
};


template<template<typename> typename T>
struct HoughUserTemplate
{
    T<draw::PixelViewGroup> houghView;
};


using HoughUserGroup = pex::Group<HoughUserFields, HoughUserTemplate>;

using HoughUserControl = typename HoughUserGroup::DefaultControl;
using HoughUserModel = typename HoughUserGroup::Model;


class DemoBrain: public GrayPngBrain<DemoBrain>
{
public:

    using Base = GrayPngBrain<DemoBrain>;

    DemoBrain()
        :
        Base(),
        observer_(this, UserControl(this->user_)),
        demoModel_(),
        demoControl_(this->demoModel_),

        maskBrain_(
            iris::MaskControl(this->demoModel_.mask),
            this->userControl_.pixelView),

        demoEndpoint_(
            this,
            this->demoControl_,
            &DemoBrain::OnSettings_),

        houghEndpoint_(
            this,
            this->demoControl_.linesChain.hough,
            &DemoBrain::OnHoughSettings_),

        houghUser_{},
        houghUserControl_(this->houghUser_),

        filters_(
            iris::CancelControl(this->cancel_),
            DemoControl(this->demoModel_)),

        displayThread_(
            this->userControl_.pixelView.asyncPixels,
            iris::CancelControl(this->cancel_),
            std::bind(&DemoBrain::Process, this))
    {

    }

    ~DemoBrain()
    {
        this->Shutdown();
    }

    std::string GetAppName() const
    {
        return "Lines Demo";
    }

    void SetPngData(const SourceType &data)
    {
        this->filters_.source.SetData(data);
        this->filters_.level.AutoDetectSettings();
        this->filters_.linesChain.AutoDetectSettings();
    }

    void ExportPng()
    {
        draw::WritePng(
            "hough.png",
            *this->houghUserControl_.houghView.pixels.Get());
    }

    wxWindow * CreateControls(wxWindow *parent)
    {
        auto window = new DemoControls(
            parent,
            this->GetUserControls(),
            this->demoControl_);

        this->OnHoughSettings_(this->demoControl_.linesChain.hough.Get());

        return window;
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
        wxAboutBox(MakeAboutDialogInfo("Lines Demo"));
    }

    std::shared_ptr<draw::Pixels> MakePixels(
        const iris::ProcessMatrix &value) const
    {
        return this->filters_.color.Filter(value);
    }

    std::shared_ptr<draw::Pixels> Process()
    {
        this->userControl_.pixelView.asyncShapes.Set(
            draw::Shapes::MakeResetter());

        this->maskBrain_.UpdateDisplay();

        auto linesResult = this->filters_.linesChain.GetChainResults();

        if (!linesResult)
        {
            auto levelResult = this->filters_.level.GetResult();

            if (!levelResult)
            {
                return {};
            }

            return this->MakePixels(*levelResult);
        }

        return linesResult->Display(
            this->userControl_.pixelView.asyncShapes,
            this->demoModel_.linesChain.shape.Get(),
            this->filters_.color,
            &this->houghUserControl_.houghView.asyncPixels);
    }

    void Shutdown()
    {
        this->displayThread_.Shutdown();
        this->houghView_.Close();
        this->GrayPngBrain<DemoBrain>::Shutdown();
    }

    void Display()
    {
        this->displayThread_.Display();
    }

private:
    void OnSettings_(const DemoSettings &)
    {
        if (this->png_)
        {
            this->Display();
        }
    }

    void OnHoughEnable_(bool isEnabled)
    {
        if (isEnabled)
        {
            if (!this->houghView_)
            {
                this->houghView_ = {
                    new draw::PixelFrame(
                        this->houghUserControl_.houghView,
                        "Hough Space"),
                    MakeShortcuts(this->GetUserControls())};
            }

            this->houghView_.Get()->Show();
        }
        else if (this->houghView_)
        {
            this->houghView_.Get()->Show(false);
        }
    }

    void OnHoughSettings_(const iris::HoughSettings<double> &houghSettings)
    {
        this->houghUser_.houghView.canvas.viewSettings.imageSize.Set(
            draw::Size(
                static_cast<int>(houghSettings.thetaCount),
                static_cast<int>(houghSettings.rhoCount)));

        this->OnHoughEnable_(houghSettings.enable);
    }

private:
    Observer<DemoBrain> observer_;
    DemoModel demoModel_;
    DemoControl demoControl_;
    iris::MaskBrain maskBrain_;
    pex::Endpoint<DemoBrain, DemoControl> demoEndpoint_;
    pex::Endpoint<DemoBrain, iris::HoughControl<double>> houghEndpoint_;
    HoughUserModel houghUser_;
    HoughUserControl houghUserControl_;
    wxpex::ShortcutWindow houghView_;
    Filters filters_;
    DisplayThread displayThread_;
};


// Creates the main function for us, and initializes the app's run loop.
wxshimAPP(wxpex::App<DemoBrain>)
