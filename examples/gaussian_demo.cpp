#include <iostream>
#include <future>
#include <wxpex/app.h>
#include <wxpex/file_field.h>
#include <draw/pixels.h>
#include <iris/gaussian_settings.h>
#include <iris/views/gaussian_settings_view.h>
#include <iris/gaussian.h>

#include "common/about_window.h"
#include "common/observer.h"
#include "common/brain.h"


class DemoControls: public wxPanel
{
public:
    DemoControls(
        wxWindow *parent,
        UserControl userControl,
        iris::GaussianControl<Pixel> gaussianControl)
        :
        wxPanel(parent, wxID_ANY)
    {
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
            "Gaussian Blur",
            gaussianControl,
            layoutOptions);

        settings->Expand();

        auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);

        sizer->Add(fileSelector, 0, wxEXPAND | wxBOTTOM, 5);
        sizer->Add(settings, 1, wxEXPAND | wxBOTTOM, 5);
        auto topSizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
        topSizer->Add(sizer.release(), 1, wxEXPAND | wxALL, 5);
        this->SetSizerAndFit(topSizer.release());
    }
};


class DemoBrain: public Brain<DemoBrain>
{
public:

    using Matrix = typename draw::PlanarRgb<Pixel>::Matrix;
    using Gaussian = iris::Gaussian<Matrix, 0>;

    DemoBrain()
        :
        observer_(this, UserControl(this->user_)),
        gaussianModel_(),
        gaussianTerminus_(
            this,
            iris::GaussianControl<Pixel>(this->gaussianModel_),
            DemoBrain::OnSettings_),
        gaussian_(this->gaussianModel_.Get())
    {

    }

    std::string GetAppName() const
    {
        return "Gaussian Demo";
    }

    wxWindow * CreateControls(wxWindow *parent)
    {
        return new DemoControls(
            parent,
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

    static std::shared_ptr<draw::Pixels>
    MakePixels(const draw::PlanarRgb<Pixel> &planarRgb)
    {
        draw::PlanarRgb<uint8_t> result = planarRgb.template Cast<uint8_t>();
        auto size = result.GetSize();

        return std::make_shared<draw::Pixels>(
            result.GetInterleaved(),
            size.height,
            size.width);
    }

    std::shared_ptr<draw::Pixels> Process() const
    {
        draw::PlanarRgb<Pixel> processed;
        const auto png = this->png_.GetRgbPixels();
        using Matrix = typename draw::PlanarRgb<Pixel>::Matrix;

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
    pex::Endpoint<DemoBrain, iris::GaussianControl<Pixel>> gaussianEndpoint_;
    Gaussian gaussian_;
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP_CONSOLE(wxpex::App<DemoBrain>)
