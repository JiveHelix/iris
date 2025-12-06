#include <iostream>
#include <future>
#include <wxpex/app.h>
#include <wxpex/wxshim_app.h>
#include <wxpex/file_field.h>
#include <draw/pixels.h>
#include <iris/node.h>
#include <iris/gaussian_settings.h>
#include <iris/views/gaussian_settings_view.h>
#include <iris/gaussian.h>

#include "common/about_window.h"
#include "common/observer.h"
#include "common/brain.h"
#include "common/display_thread.h"


class DemoControls: public wxPanel
{
public:
    DemoControls(
        wxWindow *parent,
        UserControl userControl,
        iris::GaussianControl<iris::InProcess> gaussianControl)
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

        auto settings = new iris::GaussianSettingsView<iris::InProcess>(
            this,
            "Gaussian Blur",
            gaussianControl,
            nullptr,
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
    using Png = draw::Png<PngPixel>;
    using Rgb = draw::PlanarRgb<iris::InProcess>;
    using Gaussian = iris::Gaussian<iris::InProcess, 0>;


    DemoBrain()
        :
        mutex_(),
        sourceMutex_(),
        observer_(this, UserControl(this->user_)),
        gaussianModel_(),
        gaussianEndpoint_(
            this,
            iris::GaussianControl<iris::InProcess>(this->gaussianModel_),
            &DemoBrain::OnSettings_),
        png_{},
        cancel_(),
        source_(),
        gaussian_(this->gaussianModel_.Get()),

        displayThread_(
            this->userControl_.pixelView.asyncPixels,
            iris::CancelControl(this->cancel_),
            std::bind(&DemoBrain::Process, this))
    {

    }

    void Display()
    {
        this->displayThread_.Display();
    }

    void Shutdown()
    {
        this->displayThread_.Shutdown();
        this->Brain<DemoBrain>::Shutdown();
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
            iris::GaussianControl<iris::InProcess>(this->gaussianModel_));
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

    void LoadPng(const draw::Png<PngPixel> &png)
    {
        auto pngSize = png.GetSize();
        this->user_.pixelView.canvas.viewSettings.imageSize.Set(pngSize);

        std::lock_guard lock(this->sourceMutex_);
        this->png_ = png;
        this->source_.SetData(png.GetRgb().template Cast<iris::InProcess>());
    }

    static std::shared_ptr<draw::Pixels>
    MakePixels(
        const tau::Margins &margins,
        const draw::PlanarRgb<iris::InProcess> &planarRgb)
    {
        auto constrained = planarRgb.RemoveMargin(margins);
        constrained.Constrain(0, 255);
        draw::PlanarRgb<uint8_t> result = constrained.template Cast<uint8_t>();

        return draw::Pixels::CreateShared(result);
    }

    std::shared_ptr<draw::Pixels> Process()
    {
        bool enabled;
        Rgb sourcePixels;
        Gaussian gaussian;
        tau::Margins margins;

        {
            std::lock_guard lock(this->sourceMutex_);

            if (!this->png_)
            {
                return {};
            }
        }

        {
            std::lock_guard lock(this->mutex_);

            enabled = this->gaussianModel_.enable.Get();
            gaussian = this->gaussian_;
        }

        {
            std::lock_guard lock(this->sourceMutex_);
            auto minimumMargins = tau::Margins::Create(gaussian.GetSize() / 2);
            margins = this->source_.GetMargins();

            std::cout << "margins: " << fields::Describe(margins) << std::endl;
            std::cout << "minimumMargins: " << fields::Describe(minimumMargins)
                << std::endl;

            if (!margins.Contains(minimumMargins))
            {
                // Our existing margins do not contain the new requirement.
                // Create new margins.
                std::cout << "!margins.Contains(minimumMargins)" << std::endl;
                this->source_.SetMargins(minimumMargins);
                margins = minimumMargins;
            }
            else
            {
                std::cout << "margins.Contains(minimumMargins)" << std::endl;
            }

            sourcePixels = *this->source_.GetResult();
        }

        Rgb processed(sourcePixels.GetSize());

        if (enabled)
        {
            gaussian.Filter(
                tau::GetRed(sourcePixels),
                tau::GetRed(processed));

            gaussian.Filter(
                tau::GetGreen(sourcePixels),
                tau::GetGreen(processed));

            gaussian.Filter(
                tau::GetBlue(sourcePixels),
                tau::GetBlue(processed));
        }
        else
        {
            processed = sourcePixels;
        }

        return this->MakePixels(margins, processed);
    }

private:
    void OnSettings_(const iris::GaussianSettings<iris::InProcess> &settings)
    {
        {
            std::lock_guard lock(this->mutex_);
            this->gaussian_ = iris::Gaussian<iris::InProcess, 0>(settings);
        }

        this->Display();
    }

private:
    mutable std::mutex mutex_;
    mutable std::mutex sourceMutex_;
    Observer<DemoBrain> observer_;
    iris::GaussianModel<iris::InProcess> gaussianModel_;
    pex::Endpoint<DemoBrain, iris::GaussianControl<iris::InProcess>> gaussianEndpoint_;
    iris::Cancel cancel_;
    std::optional<Png> png_;
    iris::Source<Rgb> source_;
    Gaussian gaussian_;
    DisplayThread displayThread_;
};


// Creates the main function for us, and initializes the app's run loop.
wxshimAPP(wxpex::App<DemoBrain>)
