#include "iris/views/thread_pool_view.h"

#include <wxpex/view.h>
#include <wxpex/slider.h>
#include <wxpex/gauge.h>
#include <wxpex/layout_items.h>


namespace iris
{


ThreadPoolView::ThreadPoolView(
    wxWindow *parent,
    const iris::ThreadPoolControl &control,
    const LayoutOptions &layoutOptions)
    :
    wxpex::StaticBox(parent, "Thread Pool")
{
    auto loadFactor = wxpex::CreateFieldSlider<3>(
        this,
        control.loadFactor);

    auto concurrency = wxpex::LabeledWidget(
        this,
        "concurrency",
        CreateView<1>(this, control.concurrency));

    auto activeCount = wxpex::LabeledWidget(
        this,
        "activeCount",
        CreateView<1>(this, control.activeCount));

    auto queuedCount = wxpex::LabeledWidget(
        this,
        "queuedCount",
        CreateView<1>(this, control.queuedCount));

    auto pressure = wxpex::LabeledWidget(
        this,
        "pressure",
        CreateView<3>(this, control.pressure));

    auto controlsSizer = wxpex::LayoutItems(
        wxpex::verticalItems,
        loadFactor,
        wxpex::LayoutLabeled(
            layoutOptions,
            concurrency,
            activeCount,
            queuedCount,
            pressure));

    this->ConfigureSizer(std::move(controlsSizer));
}


} // end namespace iris
