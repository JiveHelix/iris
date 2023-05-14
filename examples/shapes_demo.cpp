#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <wxpex/app.h>
#include <wxpex/file_field.h>

#include <iris/pixels.h>

#include <iris/views/ellipse_shape.h>
#include <iris/views/ellipse_shape_view.h>
#include <iris/views/shape_view_settings.h>
#include <iris/views/shape_view.h>

#include "common/observer.h"
#include "common/about_window.h"
#include "common/brain.h"


template<typename T>
struct DemoFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::ellipse, "ellipse"),
        fields::Field(&T::shapeView, "shapeView"));
};


template<template<typename> typename T>
struct DemoTemplate
{
    T<pex::MakeGroup<iris::EllipseShapeGroup>> ellipse;
    T<iris::ShapesViewGroupMaker> shapeView;
};


using DemoGroup = pex::Group<DemoFields, DemoTemplate>;
using DemoSettings = typename DemoGroup::Plain;
using DemoModel = typename DemoGroup::Model;
using DemoControl = typename DemoGroup::template Control<void>;

template<typename Observer>
using DemoTerminus =
    typename DemoGroup::template Terminus<Observer>;


class DemoMainFrame: public wxFrame
{
public:
    DemoMainFrame(
        UserControl userControl,
        DemoControl control)
        :
        wxFrame(nullptr, wxID_ANY, "Shapes Demo"),
        shortcuts_(
            std::make_unique<wxpex::MenuShortcuts>(
                wxpex::Window(this),
                MakeShortcuts(userControl)))
    {
        this->SetMenuBar(this->shortcuts_->GetMenuBar());

        auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);

        wxpex::LayoutOptions layoutOptions{};
        layoutOptions.labelFlags = wxALIGN_RIGHT;

        auto ellipseShapeView =
            new iris::EllipseShapeView(this, control.ellipse, layoutOptions);

        ellipseShapeView->Expand();

        auto buffered =
            new wxpex::CheckBox(this, "Buffered", control.shapeView.buffered);

        sizer->Add(ellipseShapeView, 1, wxEXPAND | wxBOTTOM, 5);
        sizer->Add(buffered, 0, wxEXPAND | wxBOTTOM, 5);

        auto topSizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
        topSizer->Add(sizer.release(), 3, wxEXPAND | wxALL, 5);
        this->SetSizerAndFit(topSizer.release());
    }

private:
    std::unique_ptr<wxpex::MenuShortcuts> shortcuts_;
};



class DemoBrain: public Brain<DemoBrain>
{
public:
    DemoBrain()
        :
        observer_(this, UserControl(this->user_)),
        demoModel_(),
        demoTerminus_(this, this->demoModel_),
        demoControl_(this->demoModel_)
    {
        this->demoTerminus_.ellipse.Connect(&DemoBrain::OnSettings_);
        this->demoTerminus_.shapeView.buffered.Connect(&DemoBrain::OnBuffered_);
    }

    wxpex::Window CreateControlFrame()
    {
        auto window = wxpex::Window(new DemoMainFrame(
            this->GetUserControls(),
            DemoControl(this->demoModel_)));

        this->userControl_.pixelView.viewSettings.imageSize.Set(
            iris::Size(1920, 1080));

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
        wxAboutBox(MakeAboutDialogInfo("Ellipse Demo"));
    }

    void Display()
    {
        auto shapes = iris::Shapes();

        shapes.push_back(
            std::make_shared<iris::EllipseShape>(
                this->demoModel_.ellipse.Get()));

        this->demoControl_.shapeView.asyncData.Set(shapes);
    }

    void Shutdown()
    {
        Brain<DemoBrain>::Shutdown();
    }

    void LoadPng(const iris::Png<Pixel> &)
    {

    }

    void CreateDataView_()
    {
        this->dataView_ = {
            new iris::ShapesView(
                nullptr,
                iris::ShapesViewControl(this->demoModel_.shapeView),
                "Ellipse View"),
            MakeShortcuts(this->GetUserControls())};

        this->dataView_.Get()->Show();
    }

private:
    void OnSettings_(const iris::EllipseShape &)
    {
        this->Display();
    }

    void OnBuffered_(bool)
    {
        this->Display();
    }


private:
    Observer<DemoBrain> observer_;
    DemoModel demoModel_;
    DemoTerminus<DemoBrain> demoTerminus_;
    DemoControl demoControl_;
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP_CONSOLE(wxpex::App<DemoBrain>)
