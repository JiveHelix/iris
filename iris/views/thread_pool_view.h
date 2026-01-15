#pragma once


#include <wxpex/static_box.h>
#include <wxpex/labeled_widget.h>
#include <iris/thread_pool.h>


namespace iris
{


class ThreadPoolView: public wxpex::StaticBox
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    ThreadPoolView(
        wxWindow *parent,
        const iris::ThreadPoolControl &control,
        const LayoutOptions &layoutOptions = LayoutOptions{});
};


} // end namespace iris
