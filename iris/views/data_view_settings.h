#pragma once


#include <pex/group.h>
#include <wxpex/async.h>
#include "iris/views/view_settings.h"


namespace iris
{


template<typename T>
struct DataViewFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::viewSettings, "viewSettings"),
        fields::Field(&T::mousePosition, "mousePosition"),
        fields::Field(&T::mouseDown, "mouseDown"),
        fields::Field(&T::logicalPosition, "logicalPosition"),
        fields::Field(&T::data, "data"),
        fields::Field(&T::buffered, "buffered"));
};


template<typename Data>
struct DataViewTemplate
{
    using AsyncData = wxpex::MakeAsync<Data>;

    template<template<typename> typename T>
    struct Template
    {
        T<ViewSettingsGroupMaker> viewSettings;
        T<pex::MakeGroup<PointGroup>> mousePosition;
        T<bool> mouseDown;
        T<pex::MakeGroup<PointGroup>> logicalPosition;
        T<AsyncData> data;
        T<bool> buffered;
    };
};


template<typename Data>
using DataViewGroup = pex::Group
    <
        DataViewFields,
        DataViewTemplate<Data>::template Template
    >;


template<typename Data>
struct DataViewModel: public DataViewGroup<Data>::Model
{
    DataViewModel()
        :
        DataViewGroup<Data>::Model(),
        viewSettings_(this, this->viewSettings),
        mousePosition_(this, this->mousePosition) {
        this->viewSettings_.Connect(&DataViewModel::OnViewSettings_);
        this->mousePosition_.Connect(&DataViewModel::OnMousePosition_);
        this->OnViewSettings_(this->viewSettings.Get());
    }

    void OnViewSettings_(const ViewSettings &settings)
    {
        this->logicalPosition.Set(
            settings.GetLogicalPosition(this->mousePosition.Get()));
    }

    void OnMousePosition_(const Point &point)
    {
        this->logicalPosition.Set(
            this->viewSettings.Get().GetLogicalPosition(point));
    }

private:
    ViewSettingsTerminus<DataViewModel> viewSettings_;
    PointTerminus<DataViewModel> mousePosition_;
};


template<typename Data>
struct DataViewControl_
{
    template<typename Observer>
    struct Control: public DataViewGroup<Data>::template Control<Observer>
    {
        using AsyncControl =
            typename DataViewTemplate<Data>::AsyncData
                ::template Control<Observer>;

        AsyncControl asyncData;

        Control() = default;

        Control(DataViewModel<Data> &dataViewModel)
            :
            DataViewGroup<Data>::template Control<Observer>(dataViewModel),
            asyncData(dataViewModel.data.GetWorkerControl())
        {

        }
    };
};


template<typename Data>
using DataViewControl = typename DataViewControl_<Data>::template Control<void>;

template<typename Data, typename Observer>
using DataViewTerminus =
    typename DataViewGroup<Data>::template Terminus<Observer>;

template<typename Data>
using DataViewGroupMaker = pex::MakeGroup
    <
        DataViewGroup<Data>,
        DataViewModel<Data>,
        DataViewControl_<Data>::template Control
    >;


} // end namespace iris
