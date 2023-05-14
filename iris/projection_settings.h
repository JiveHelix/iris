#pragma once

#include <fields/fields.h>
#include <jive/scope_flag.h>
#include <pex/interface.h>
#include <iris/intrinsics.h>
#include <iris/pose.h>


namespace iris
{


namespace file
{


constexpr auto intrinsicsExtension = ".intrinsic";
constexpr auto poseExtension = ".pose";



template<typename T>
struct FileFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::settings, "settings"),
        fields::Field(&T::fileName, "fileName"),
        fields::Field(&T::read, "read"),
        fields::Field(&T::write, "write"));
};


template<typename Float>
struct IntrinsicsTemplate
{
    template<template<typename> typename T>
    struct Template
    {
        T<pex::MakeGroup<::iris::IntrinsicsGroup<Float>>> settings;
        T<std::string> fileName;
        T<pex::MakeSignal> read;
        T<pex::MakeSignal> write;

        static constexpr auto fields = FileFields<Template>::fields;
        static constexpr auto fieldsTypeName = "IntrinsicsFile";
    };
};


template<typename Float>
struct PoseTemplate
{
    template<template<typename> typename T>
    struct Template
    {
        T<pex::MakeGroup<::iris::PoseGroup<Float>>> settings;
        T<std::string> fileName;
        T<pex::MakeSignal> read;
        T<pex::MakeSignal> write;

        static constexpr auto fields = FileFields<Template>::fields;
        static constexpr auto fieldsTypeName = "PoseFile";
    };
};


template<typename T>
using IntrinsicsGroup =
    pex::Group<FileFields, IntrinsicsTemplate<T>::template Template>;

template<typename T>
using IntrinsicsSettings = typename IntrinsicsGroup<T>::Plain;

template<typename T>
using IntrinsicsControl = typename IntrinsicsGroup<T>::template Control<void>;

template<typename T, typename Observer>
using IntrinsicsTerminus =
    typename IntrinsicsGroup<T>::template Terminus<Observer>;


template<typename T>
using PoseGroup = pex::Group<FileFields, PoseTemplate<T>::template Template>;

template<typename T>
using PoseSettings = typename PoseGroup<T>::Plain;

template<typename T>
using PoseControl = typename PoseGroup<T>::template Control<void>;

template<typename T, typename Observer>
using PoseTerminus = typename PoseGroup<T>::template Terminus<Observer>;


} // end namespace file


template<typename T>
struct ProjectionFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::intrinsics, "intrinsics"),
        fields::Field(&T::pose, "pose"));
};


template<typename Float>
struct ProjectionTemplate
{
    template<template<typename> typename T>
    struct Template
    {
        T<pex::MakeGroup<file::IntrinsicsGroup<Float>>> intrinsics;
        T<pex::MakeGroup<file::PoseGroup<Float>>> pose;

        static constexpr auto fields = ProjectionFields<Template>::fields;
        static constexpr auto fieldsTypeName = "ProjectionFile";
    };
};


template<typename T>
using ProjectionGroup =
    pex::Group<ProjectionFields, ProjectionTemplate<T>::template Template>;

template<typename T>
using ProjectionSettings = typename ProjectionGroup<T>::Plain;

template<typename T>
using ProjectionModel = typename ProjectionGroup<T>::Model;

template<typename T>
using ProjectionControl = typename ProjectionGroup<T>::template Control<void>;

template<typename T, typename Observer>
using ProjectionTerminus =
    typename ProjectionGroup<T>::template Terminus<Observer>;


template<typename T>
struct StereoProjectionFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::camera0, "camera0"),
        fields::Field(&T::camera1, "camera1"));
};

template<typename Float
struct StereoProjectionTemplate
{
    template<template<typename> typename T>
    struct Template
    {
        T<pex::MakeGroup<ProjectionGroup<Float>>> camera0;
        T<pex::MakeGroup<ProjectionGroup<Float>>> camera1;

        static constexpr auto fields =
            StereoProjectionFields<Template>::fields;

        static constexpr auto fieldsTypeName = "StereoProjectionFile";
    };
};

template<typename T>
using StereoProjectionGroup =
    pex::Group
    <
        StereoProjectionFields,
        StereoProjectionTemplate<T>::template Template
    >;

template<typename T>
using StereoProjectionSettings = typename StereoProjectionGroup<T>::Plain;

template<typename T>
struct StereoProjectionModel: public StereoProjectionGroup<T>::Model
{
private:
    using Terminus =
        pex::Terminus<StereoProjectionModel, pex::model::Value<std::string>>;

    bool ignore_;
    Terminus intrinsics0_;
    Terminus intrinsics1_;
    Terminus pose0_;
    Terminus pose1_;

public:
    StereoProjectionModel()
        :
        StereoProjectionGroup<T>::Model(),
        ignore_(false),
        intrinsics0_(this, this->camera0.intrinsics.fileName),
        intrinsics1_(this, this->camera1.intrinsics.fileName),
        pose0_(this, this->camera0.pose.fileName),
        pose1_(this, this->camera1.pose.fileName)
    {
        this->intrinsics0_.Connect(&StereoProjectionModel::OnFileName_);
        this->intrinsics1_.Connect(&StereoProjectionModel::OnFileName_);
        this->pose0_.Connect(&StereoProjectionModel::OnFileName_);
        this->pose1_.Connect(&StereoProjectionModel::OnFileName_);
    }

private:
    static void UpdateIfUnset_(Terminus &terminus, const std::string &directory)
    {
        auto file = jive::path::Base(terminus.Get());

        if (!file.empty())
        {
            // This value has been set.
            // Do not update the directory.
            return;
        }

        terminus.Set(directory);
    }

    void OnFileName_(const std::string &fileName)
    {
        if (this->ignore_)
        {
            return;
        }

        auto ignore = jive::ScopeFlag(this->ignore_);

        auto directory = jive::path::Directory(fileName) + "/";
        UpdateIfUnset_(this->intrinsics0_, directory);
        UpdateIfUnset_(this->intrinsics1_, directory);
        UpdateIfUnset_(this->pose0_, directory);
        UpdateIfUnset_(this->pose1_, directory);
    }
};


template<typename T>
using StereoProjectionControl =
    typename StereoProjectionGroup<T>::template Control<void>;

template<typename T, typename Observer>
using StereoProjectionTerminus =
    typename StereoProjectionGroup<T>::template Terminus<Observer>;

template<typename T>
using StereoProjectionGroupMaker =
    pex::MakeGroup<StereoProjectionGroup<T>, StereoProjectionModel<T>>;


} // end namespace iris
