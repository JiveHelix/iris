#include <iostream>
#include <future>
#include <wxpex/app.h>
#include <wxpex/file_field.h>
#include <iris/pixels.h>
#include <iris/gaussian_settings.h>
#include <iris/views/gaussian_settings_view.h>
#include <iris/gaussian.h>

#include "common/about_window.h"
#include "common/observer.h"
#include "common/brain.h"


class DemoMainFrame: public wxFrame
{
public:
    DemoMainFrame(
        UserControl userControl,
        iris::GaussianControl<Pixel> gaussianControl)
        :
        wxFrame(nullptr, wxID_ANY, "Gaussian Demo"),
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

        auto settings = new iris::GaussianSettingsView<Pixel>(
            this,
            gaussianControl,
            layoutOptions);

        settings->Expand();

        sizer->Add(fileSelector, 0, wxEXPAND | wxBOTTOM, 5);
        sizer->Add(settings, 1, wxEXPAND | wxBOTTOM, 5);
        auto topSizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
        topSizer->Add(sizer.release(), 1, wxEXPAND | wxALL, 5);
        this->SetSizerAndFit(topSizer.release());
    }

private:
    std::unique_ptr<wxpex::MenuShortcuts> shortcuts_;
};


class DemoBrain: public Brain<DemoBrain>
{
public:

    using Matrix = typename iris::PlanarRgb<Pixel>::Matrix;
    using Gaussian = iris::Gaussian<Matrix, 0>;

    DemoBrain()
        :
        observer_(this, UserControl(this->user_)),
        gaussianModel_(),
        gaussianTerminus_(this, this->gaussianModel_),
        gaussian_(this->gaussianModel_.Get())
    {
        this->gaussianTerminus_.Connect(&DemoBrain::OnSettings_);
    }

    wxpex::Window CreateControlFrame()
    {
        return new DemoMainFrame(
            this->GetUserControls(),
            iris::GaussianControl<Pixel>(this->gaussianModel_));
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
        wxAboutBox(MakeAboutDialogInfo("Gaussian Demo"));
    }

    static iris::Pixels MakePixels(const iris::PlanarRgb<Pixel> &planarRgb)
    {
        iris::PlanarRgb<uint8_t> result = planarRgb.template Cast<uint8_t>();
        auto size = result.GetSize();

        return {
            result.GetInterleaved(),
            size.height,
            size.width};
    }

    iris::Pixels Process() const
    {
        iris::PlanarRgb<Pixel> processed;
        const auto png = this->png_.GetRgbPixels();
        using Matrix = typename iris::PlanarRgb<Pixel>::Matrix;

        auto red = std::async(
            std::launch::async,
            &Gaussian::FilterExtend<Matrix>,
            &this->gaussian_,
            tau::GetRed(png));

        auto green = std::async(
            std::launch::async,
            &Gaussian::FilterExtend<Matrix>,
            &this->gaussian_,
            tau::GetGreen(png));

        auto blue = std::async(
            std::launch::async,
            &Gaussian::FilterExtend<Matrix>,
            &this->gaussian_,
            tau::GetBlue(png));

        tau::GetRed(processed) = red.get();
        tau::GetGreen(processed) = green.get();
        tau::GetBlue(processed) = blue.get();

        return MakePixels(processed);
    }

    void Display()
    {
        if (!this->png_)
        {
            return;
        }

        std::lock_guard lock(this->mutex_);

        if (this->gaussianModel_.enable)
        {
            this->user_.pixelView.pixels.Set(this->Process());
        }
        else
        {
            this->user_.pixelView.pixels.Set(
                MakePixels(this->png_.GetRgbPixels()));
        }
    }

private:
    void OnSettings_(const iris::GaussianSettings<Pixel> &settings)
    {
        {
            std::lock_guard lock(this->mutex_);
            this->gaussian_ = iris::Gaussian<Pixel, 0>(settings);
        }

        this->Display();
    }

private:
    Observer<DemoBrain> observer_;
    iris::GaussianModel<Pixel> gaussianModel_;
    iris::GaussianTerminus<Pixel ,DemoBrain> gaussianTerminus_;
    Gaussian gaussian_;
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP_CONSOLE(wxpex::App<DemoBrain>)
