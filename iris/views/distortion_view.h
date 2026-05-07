#pragma once


#include <wxpex/wxshim.h>
#include <wxpex/static_box.h>
#include <wxpex/labeled_widget.h>
#include <tau/intrinsics.h>
#include <iris/homography.h>


namespace iris
{


class DistortionView: public wxpex::StaticBox
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    DistortionView(
        wxWindow *parent,
        const std::string &name,
        const DistortionControl<double> &control,
        const LayoutOptions &layoutOptions = LayoutOptions{});
};


} // end namespace iris
