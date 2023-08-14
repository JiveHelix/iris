#pragma once

#include <wxpex/collapsible.h>
#include <wxpex/labeled_widget.h>

#include "iris/projection_settings.h"
#include "iris/view/intrinsics_view.h"
#include "iris/view/pose_view.h"
#include "irsi/views/defaults.h"


namespace iris
{


template<typename T>
class ProjectionSettingsView: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    ProjectionSettingsView(
        wxWindow *parent,
        const std::string &name,
        ProjectionControl<T> control)
        :
        wxpex::Collapsible(parent, name)
    {
        auto pane = this->GetBorderPane(borderStyle);

        auto intrinsics = wxpex::LabeledWidget(
            pane,
            "Intrinsics",
            new IntrinsicsView<T>(pane, control.intrinsics));

        auto pose = wxpex::LabeledWidget(
            pane,
            "Pose",
            new PoseView<T>(pane, control.pose));

        auto sizer = wxpex::LayoutItems(
            verticalItems,
            intrinsics.Layout(wxVERTICAL).release(),
            pose.Layout(wxVERTICAL).release());

        this->ConfigureBorderPane(borderPixels, std::move(sizer));
    }
};


template<typename T>
class StereoProjectionSettingsView: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    StereoProjectionSettingsView(
        wxWindow *parent,
        StereoProjectionControl<T> control)
        :
        wxpex::Collapsible(parent, "Projection")
    {
        auto camera0 = new ProjectionSettingsView<T>(
            this->GetPane(),
            "Camera 0",
            control.camera0);

        auto camera1 = new ProjectionSettingsView<T>(
            this->GetPane(),
            "Camera 1",
            control.camera1);

        auto sizer = wxpex::LayoutItems(
            wxVERTICAL,
            wxEXPAND | wxALL,
            5,
            camera0,
            camera1);

        this->ConfigureTopSizer(sizer.release());
    }
};


} // end namespace iris
