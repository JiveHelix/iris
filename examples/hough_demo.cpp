#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <wxpex/app.h>
#include <wxpex/file_field.h>

#include <iris/pixels.h>

#include <iris/gaussian_settings.h>
#include <iris/gradient_settings.h>
#include <iris/canny_settings.h>
#include <iris/hough_settings.h>
#include <iris/color_settings.h>

#include <iris/node.h>
#include <iris/gaussian.h>
#include <iris/gradient.h>
#include <iris/canny.h>
#include <iris/hough.h>
#include <iris/color.h>

#include <iris/views/gaussian_settings_view.h>
#include <iris/views/gradient_settings_view.h>
#include <iris/views/canny_settings_view.h>
#include <iris/views/hough_settings_view.h>
#include <iris/views/color_settings_view.h>
#include <iris/views/draw_lines.h>

#include "common/about_window.h"
#include "common/observer.h"
#include "common/brain.h"


template<typename T>
struct DemoFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::gaussian, "gaussian"),
        fields::Field(&T::gradient, "gradient"),
        fields::Field(&T::canny, "canny"),
        fields::Field(&T::hough, "hough"),
        fields::Field(&T::houghView, "houghView"),
        fields::Field(&T::color, "color"));
};


using InProcess = int32_t;


template<template<typename> typename T>
struct DemoTemplate
{
    T<iris::GaussianGroupMaker<InProcess>> gaussian;
    T<iris::GradientGroupMaker<InProcess>> gradient;
    T<iris::CannyGroupMaker<float>> canny;
    T<iris::HoughGroupMaker<float>> hough;
    T<iris::PixelViewGroupMaker> houghView;
    T<iris::ColorGroupMaker<InProcess>> color;
};


using DemoGroup = pex::Group<DemoFields, DemoTemplate>;
using DemoSettings = typename DemoGroup::Plain;
using DemoModel = typename DemoGroup::Model;
using DemoControl = typename DemoGroup::template Control<void>;

template<typename Observer>
using DemoTerminus = typename DemoGroup::template Terminus<Observer>;


template<typename T>
struct HoughUserFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::houghView, "houghView"));
};


template<template<typename> typename T>
struct HoughUserTemplate
{
    T<iris::PixelViewGroupMaker> houghView;
};


using HoughUserGroup = pex::Group<HoughUserFields, HoughUserTemplate>;

using HoughUserControl = typename HoughUserGroup::Control<void>;
using HoughUserModel = typename HoughUserGroup::Model;

template<typename Observer>
using HoughUserTerminus = typename HoughUserGroup::Terminus<Observer>;



class DemoMainFrame: public wxFrame
{
public:
    DemoMainFrame(
        UserControl userControl,
        DemoControl control)
        :
        wxFrame(nullptr, wxID_ANY, "Hough Demo"),
        shortcuts_(
            std::make_unique<wxpex::MenuShortcuts>(
                wxpex::Window(this),
                MakeShortcuts(userControl)))
    {
        this->SetMenuBar(this->shortcuts_->GetMenuBar());

        auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);

        wxpex::LayoutOptions layoutOptions{};
        layoutOptions.labelFlags = wxALIGN_RIGHT;

        wxpex::FileDialogOptions options{};
        options.message = "Choose a PNG file";
        options.wildcard = "*.png";

        auto fileSelector = new wxpex::FileField(
            this,
            userControl.fileName,
            options);

        auto gaussian = new iris::GaussianSettingsView<InProcess>(
            this,
            control.gaussian,
            layoutOptions);

        gaussian->Expand();

        auto gradient = new iris::GradientSettingsView<InProcess>(
            this,
            control.gradient,
            layoutOptions);

        gradient->Expand();

        auto canny = new iris::CannySettingsView<float>(
            this,
            control.canny,
            layoutOptions);

        canny->Expand();

        auto hough = new iris::HoughSettingsView<float>(
            this,
            control.hough,
            layoutOptions);

        hough->Expand();

        auto color = new iris::ColorSettingsView<InProcess>(
            this,
            control.color,
            layoutOptions);

        sizer->Add(fileSelector, 0, wxEXPAND | wxBOTTOM, 5);
        sizer->Add(gaussian, 1, wxEXPAND | wxBOTTOM, 5);
        sizer->Add(gradient, 1, wxEXPAND | wxBOTTOM, 5);
        sizer->Add(canny, 1, wxEXPAND | wxBOTTOM, 5);
        sizer->Add(hough, 1, wxEXPAND | wxBOTTOM, 5);
        sizer->Add(color, 1, wxEXPAND | wxBOTTOM, 5);
        auto topSizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
        topSizer->Add(sizer.release(), 1, wxEXPAND | wxALL, 5);
        this->SetSizerAndFit(topSizer.release());
    }

private:
    std::unique_ptr<wxpex::MenuShortcuts> shortcuts_;
};


using ProcessMatrix =
    Eigen::Matrix
    <
        InProcess,
        Eigen::Dynamic,
        Eigen::Dynamic,
        Eigen::RowMajor
    >;


class PngSource
{
public:
    using Result = ProcessMatrix;

    PngSource()
        :
        pngData_()
    {

    }

    void SetPng(const iris::Png<Pixel> &png, float scale = 4095)
    {
        this->pngData_ = png.GetValue(scale).template cast<InProcess>();
    }

    bool HasResult() const
    {
        return !!this->pngData_;
    }

    std::optional<ProcessMatrix> GetResult()
    {
        return this->pngData_;
    }

    std::optional<ProcessMatrix> pngData_;
};


struct Filters
{
    using Gaussian = iris::Gaussian<ProcessMatrix, 0>;
    using Gradient = iris::Gradient<ProcessMatrix>;
    using Canny = iris::Canny<float>;
    using Hough = iris::Hough<float>;
    using Color = iris::ThreadsafeColor<InProcess>;

    using GaussianNode =
        iris::Node<PngSource, Gaussian, iris::GaussianControl<InProcess>>;

    using GradientNode =
        iris::Node<GaussianNode, Gradient, iris::GradientControl<InProcess>>;

    using CannyNode =
        iris::Node<GradientNode, Canny, iris::CannyControl<float>>;

    using HoughNode =
        iris::Node<CannyNode, Hough, iris::HoughControl<float>>;

    iris::Cancel cancel;
    PngSource pngSource;
    GaussianNode gaussian;
    GradientNode gradient;
    CannyNode canny;
    HoughNode hough;
    Color color;

    template<typename Controls>
    Filters(Controls controls)
        :
        cancel(false),
        pngSource(),
        gaussian(
            this->pngSource,
            controls.gaussian,
            iris::CancelControl(this->cancel)),
        gradient(
            this->gaussian,
            controls.gradient,
            iris::CancelControl(this->cancel)),
        canny(
            this->gradient,
            controls.canny,
            iris::CancelControl(this->cancel)),
        hough(
            this->canny,
            controls.hough,
            iris::CancelControl(this->cancel)),
        color(controls.color)
    {

    }
};


enum class DisplayState
{
    waiting,
    processing
};


class DemoBrain: public Brain<DemoBrain>
{
public:

    DemoBrain()
        :
        observer_(this, UserControl(this->user_)),
        demoModel_(),
        demoTerminus_(this, this->demoModel_),
        houghUser_{},
        houghUserControl_(this->houghUser_),
        filters_(DemoControl(this->demoModel_)),
        mutex_(),
        condition_(),
        displayState_(DisplayState::waiting),
        displayRequested_(false),
        displayLoopIsRunning_(true),
        displayThread_(
            std::bind(&DemoBrain::DisplayLoop_, this))
    {
        this->demoModel_.gradient.enable = false;
        this->demoModel_.gradient.maximumInput.Set(8096);
        this->demoModel_.color.level.SetMaximumValue(8096);
        this->demoModel_.color.level.high.Set(8096);
        this->demoTerminus_.Connect(&DemoBrain::OnSettings_);
        this->demoTerminus_.hough.Connect(&DemoBrain::OnHoughSettings_);
    }

    void LoadPng(const iris::Png<Pixel> &png)
    {
        auto scale =
            static_cast<float>(this->demoModel_.color.level.high.GetMaximum());

        this->filters_.pngSource.SetPng(png, scale);
        this->demoModel_.hough.imageSize.Set(png.GetSize());
    }

    wxpex::Window CreateControlFrame()
    {
        auto window = wxpex::Window(new DemoMainFrame(
            this->GetUserControls(),
            DemoControl(this->demoModel_)));

        this->OnHoughSettings_(this->demoTerminus_.hough.Get());
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
        wxAboutBox(MakeAboutDialogInfo("Hough Demo"));
    }

    iris::Pixels MakePixels(const ProcessMatrix &value) const
    {
        return this->filters_.color.Filter(value);
    }

    iris::Pixels Process()
    {
        assert(this->filters_.pngSource.HasResult());

        auto start = std::chrono::steady_clock::now();
        auto houghResult = this->filters_.hough.GetResult();
        auto end = std::chrono::steady_clock::now();

        auto elapsed =
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        if (this->filters_.cancel.Get())
        {
            return {};
        }

        if (houghResult)
        {
            std::cout << "Hough completed in " << elapsed.count() << " ms"
                << std::endl;

            auto scale = static_cast<float>(
                this->demoModel_.color.level.high.GetMaximum());

            ProcessMatrix space = houghResult->GetScaledSpace<InProcess>(scale);

            this->houghUserControl_.houghView.asyncData.Set(
                this->MakePixels(space));

            auto cannyResult = this->filters_.canny.GetResult();

            if (!cannyResult || this->filters_.cancel.Get())
            {
                return {};
            }

            if (houghResult->lines.empty())
            {
                return cannyResult->Colorize<uint8_t>();
            }

            auto pixels = cannyResult->Colorize<uint8_t>();

            auto image = wxImage(
                static_cast<int>(pixels.size.width),
                static_cast<int>(pixels.size.height),
                pixels.data.data(),
                true);

            auto bitmap = wxBitmap(image);
            auto dc = wxMemoryDC(bitmap);

            dc.SetPen(wxPen(*wxBLUE, 2));
            iris::DrawLines(dc, houghResult->lines);

            dc.SelectObject(wxNullBitmap);

            auto updated = bitmap.ConvertToImage();

            pixels.data = Eigen::Map<typename tau::RgbPixels<uint8_t>::Data>(
                updated.GetData(),
                pixels.size.GetArea(),
                3);

            return pixels;
        }

        // Hough didn't return a result.
        auto cannyResult = this->filters_.canny.GetResult();

        if (this->filters_.cancel.Get())
        {
            return {};
        }

        if (cannyResult)
        {
            return cannyResult->Colorize<uint8_t>();
        }

        // Canny didn't return a result.
        auto gradientResult = this->filters_.gradient.GetResult();

        if (this->filters_.cancel.Get())
        {
            return {};
        }

        if (gradientResult)
        {
            return gradientResult->Colorize<uint8_t>();
        }

        // Gradient didn't return a result.
        auto gaussianResult = this->filters_.gaussian.GetResult();

        if (gaussianResult && !this->filters_.cancel.Get())
        {
            return this->MakePixels(*gaussianResult);
        }

        return {};
    }

    void Display()
    {
        if (!this->filters_.pngSource.HasResult())
        {
            return;
        }

        std::lock_guard lock(this->mutex_);

        if (this->displayState_ == DisplayState::processing)
        {
            this->filters_.cancel.Set(true);
        }

        this->displayRequested_ = true;
        this->condition_.notify_one();
    }

    void Shutdown()
    {
        {
            std::lock_guard lock(this->mutex_);
            this->displayLoopIsRunning_ = false;
            this->filters_.cancel.Set(true);
            this->condition_.notify_one();
        }

        this->displayThread_.join();

        this->houghView_.Close();
        Brain<DemoBrain>::Shutdown();
    }

private:
    void OnSettings_(const DemoSettings &)
    {
        if (this->filters_.pngSource.HasResult())
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
                    new iris::PixelView(
                        nullptr,
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

    void OnHoughSettings_(const iris::HoughSettings<float> &houghSettings)
    {
        this->houghUser_.houghView.viewSettings.imageSize.Set(
            iris::Size(
                static_cast<int>(houghSettings.thetaCount),
                static_cast<int>(houghSettings.rhoCount)));

        this->OnHoughEnable_(houghSettings.enable);
    }

    void DisplayLoop_()
    {
        while (this->displayLoopIsRunning_)
        {
            {
                std::unique_lock lock(this->mutex_);

                if (!this->displayRequested_)
                {
                    // Sleep until display is requested
                    this->displayState_ = DisplayState::waiting;

                    this->condition_.wait(
                        lock,
                        [this]() -> bool
                        {
                            return this->displayRequested_
                                || !this->displayLoopIsRunning_;
                        });
                }

                if (!this->displayLoopIsRunning_)
                {
                    return;
                }

                this->displayState_ = DisplayState::processing;
            }

            iris::Pixels pixels = this->Process();

            {
                std::lock_guard lock(this->mutex_);

                if (this->filters_.cancel.Get())
                {
                    this->filters_.cancel.Set(false);
                    continue;
                }
                else
                {
                    this->displayRequested_ = false;
                }
            }

            this->userControl_.pixelView.asyncData.Set(pixels);
        }
    }

private:
    Observer<DemoBrain> observer_;
    DemoModel demoModel_;
    DemoTerminus<DemoBrain> demoTerminus_;
    HoughUserModel houghUser_;
    HoughUserControl houghUserControl_;
    wxpex::ShortcutWindow houghView_;
    Filters filters_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    DisplayState displayState_;
    std::atomic_bool displayRequested_;
    std::atomic_bool displayLoopIsRunning_;
    std::thread displayThread_;
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP_CONSOLE(wxpex::App<DemoBrain>)
