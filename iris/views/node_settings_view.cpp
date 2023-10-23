#include "iris/views/node_settings_view.h"
#include "iris/views/defaults.h"
#include <wxpex/color.h>


#include <wx/settings.h>


namespace iris
{


NodeSettingsView::NodeSettingsView(
    wxWindow *parent,
    const std::string &nodeName,
    std::optional<NodeSettingsControl> control)
    :
    wxpex::Collapsible(parent, nodeName, borderStyle),
    control_(control),
    highlightEndpoint_()
{
    if (this->control_)
    {
        this->Bind(wxEVT_LEFT_DOWN, &NodeSettingsView::OnLeftDown_, this);

        this->highlightEndpoint_ = Endpoint(
            this,
            this->control_->highlight,
            &NodeSettingsView::OnHighlight_);
    }
}


void NodeSettingsView::OnLeftDown_(wxMouseEvent &event)
{
    event.Skip();
    assert(this->control_);
    this->control_->select.Trigger();
}


void NodeSettingsView::OnHighlight_(bool isHighlighted)
{
    assert(this->control_);

    if (isHighlighted)
    {
        this->SetBackgroundColour(
            wxpex::ToWxColour(this->control_->highlightColor.Get()));
    }
    else
    {
        this->SetBackgroundColour(wxColour());
    }

    this->Refresh();
}


} // end namespace iris
