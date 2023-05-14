#pragma once


#include <wxpex/button.h>


namespace iris
{


template<typename Control>
std::unique_ptr<wxSizer> MakeReadWriteButtons(wxWindow *parent, Control control)
{
    auto writeButton = new wxpex::Button(parent, "Write", control.write);
    auto readButton = new wxpex::Button(parent, "Read", control.read);

    auto sizer = std::make_unique<wxBoxSizer>(wxHORIZONTAL);
    sizer->Add(readButton, wxRIGHT, 3);
    sizer->Add(writeButton);

    return sizer;
}


} // end namespace iris
