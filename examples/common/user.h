#pragma once


#include <fields/core.h>
#include <pex/group.h>
#include <pex/endpoint.h>
#include <wxpex/shortcut.h>
#include <draw/views/pixel_view_settings.h>


template<template<typename> typename T>
struct UserSchema
{
    T<pex::MakeSignal> openFile;
    T<std::string> fileName;
    T<pex::MakeSignal> saveSettings;
    T<pex::MakeSignal> loadSettings;
    T<pex::MakeSignal> exportPng;
    T<pex::MakeSignal> quit;
    T<pex::MakeSignal> about;
    T<pex::MakeSignal> showPixelViewLayout;
    T<draw::PixelViewGroup> pixelView;
    T<std::string> errors;
};


using UserGroup = pex::Group<UserSchema>;
using UserControl = typename UserGroup::DefaultControl;
using UserModel = typename UserGroup::Model;


template<typename Observer>
using UserEndpoints = pex::EndpointGroup<Observer, UserControl>;


wxpex::ShortcutGroups MakeShortcuts(const UserControl &userControl);
