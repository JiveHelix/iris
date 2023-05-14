#pragma once


#include <fields/core.h>
#include <pex/group.h>
#include <wxpex/shortcut.h>
#include <iris/views/pixel_view_settings.h>


template<typename T>
struct UserFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::openFile, "openFile"),
        fields::Field(&T::fileName, "fileName"),
        fields::Field(&T::saveSettings, "saveSettings"),
        fields::Field(&T::loadSettings, "loadSettings"),
        fields::Field(&T::quit, "quit"),
        fields::Field(&T::about, "about"),
        fields::Field(&T::layoutWindows, "layoutWindows"),
        fields::Field(&T::showDataViewLayout, "showDataViewLayout"),
        fields::Field(&T::pixelView, "pixelView"),
        fields::Field(&T::errors, "errors"));
};


template<template<typename> typename T>
struct UserTemplate
{
    T<pex::MakeSignal> openFile;
    T<std::string> fileName;
    T<pex::MakeSignal> saveSettings;
    T<pex::MakeSignal> loadSettings;
    T<pex::MakeSignal> quit;
    T<pex::MakeSignal> about;
    T<pex::MakeSignal> layoutWindows;
    T<pex::MakeSignal> showDataViewLayout;
    T<iris::PixelViewGroupMaker> pixelView;
    T<std::string> errors;
};


using UserGroup = pex::Group<UserFields, UserTemplate>;
using UserControl = typename UserGroup::Control<void>;
using UserModel = typename UserGroup::Model;

template<typename Observer>
using UserTerminus = typename UserGroup::Terminus<Observer>;


wxpex::ShortcutGroups MakeShortcuts(UserControl userControl);
