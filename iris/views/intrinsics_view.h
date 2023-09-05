#pragma once


#include <wxpex/wxshim.h>
#include <wxpex/static_box.h>
#include <wxpex/labeled_widget.h>
#include <tau/intrinsics.h>


namespace iris
{


class IntrinsicsView: public wxpex::StaticBox
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    IntrinsicsView(
        wxWindow *parent,
        const std::string &name,
        tau::IntrinsicsControl<double> control,
        const LayoutOptions &layoutOptions = LayoutOptions{});
};


} // end namespace iris
