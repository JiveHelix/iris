#pragma once

#include <fields/fields.h>
#include <jive/scope_flag.h>
#include <jive/path.h>
#include <pex/interface.h>
#include <pex/endpoint.h>
#include <tau/intrinsics.h>
#include <tau/pose.h>


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
        T<pex::MakeGroup<tau::IntrinsicsGroup<Float>>> settings;
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
        T<pex::MakeGroup<tau::PoseGroup<Float>>> settings;
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
using IntrinsicsControl = typename IntrinsicsGroup<T>::Control;


template<typename T>
using PoseGroup = pex::Group<FileFields, PoseTemplate<T>::template Template>;

template<typename T>
using PoseSettings = typename PoseGroup<T>::Plain;

template<typename T>
using PoseControl = typename PoseGroup<T>::Control;


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
using ProjectionControl = typename ProjectionGroup<T>::Control;


template<typename T>
struct StereoProjectionFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::camera0, "camera0"),
        fields::Field(&T::camera1, "camera1"));
};

template<typename Float>
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
    using IntrinsicsControl = decltype(ProjectionControl<T>::intrinsics);
    using FileNameControl = decltype(IntrinsicsControl::fileName);

    using FileNameEndpoint =
        pex::Endpoint<StereoProjectionModel, FileNameControl>;

    bool ignore_;
    FileNameControl intrinsicsControl0_;
    FileNameControl intrinsicsControl1_;
    FileNameControl poseControl0_;
    FileNameControl poseControl1_;
    FileNameEndpoint intrinsics0_;
    FileNameEndpoint intrinsics1_;
    FileNameEndpoint pose0_;
    FileNameEndpoint pose1_;

public:
    StereoProjectionModel()
        :
        StereoProjectionGroup<T>::Model(),
        ignore_(false),
        intrinsicsControl0_(this->camera0.intrinsics.fileName),
        intrinsicsControl1_(this->camera1.intrinsics.fileName),
        poseControl0_(this->camera0.pose.fileName),
        poseControl1_(this->camera1.pose.fileName),
        intrinsics0_(
            this,
            this->intrinsicsControl0_,
            &StereoProjectionModel::OnFileName_),
        intrinsics1_(
            this,
            this->intrinsicsControl1_,
            &StereoProjectionModel::OnFileName_),
        pose0_(
            this,
            this->poseControl0_,
            &StereoProjectionModel::OnFileName_),
        pose1_(
            this,
            this->poseControl1_,
            &StereoProjectionModel::OnFileName_)
    {

    }

private:
    static void UpdateIfUnset_(
        FileNameControl &control,
        const std::string &directory)
    {
        auto file = jive::path::Base(control.Get());

        if (!file.empty())
        {
            // This value has been set.
            // Do not update the directory.
            return;
        }

        control.Set(directory);
    }

    void OnFileName_(const std::string &fileName)
    {
        if (this->ignore_)
        {
            return;
        }

        auto ignore = jive::ScopeFlag(this->ignore_);

        auto directory = jive::path::Directory(fileName) + "/";
        UpdateIfUnset_(this->intrinsicsControl0_, directory);
        UpdateIfUnset_(this->intrinsicsControl1_, directory);
        UpdateIfUnset_(this->poseControl0_, directory);
        UpdateIfUnset_(this->poseControl1_, directory);
    }
};


template<typename T>
using StereoProjectionControl =
    typename StereoProjectionGroup<T>::Control;

template<typename T>
using StereoProjectionGroupMaker =
    pex::MakeGroup<StereoProjectionGroup<T>, StereoProjectionModel<T>>;


extern template struct StereoProjectionModel<float>;
extern template struct StereoProjectionModel<double>;


} // end namespace iris


extern template struct pex::Group
    <
        iris::file::FileFields,
        iris::file::IntrinsicsTemplate<float>::template Template
    >;

extern template struct pex::Group
    <
        iris::file::FileFields,
        iris::file::IntrinsicsTemplate<double>::template Template
    >;

extern template struct pex::Group
    <
        iris::file::FileFields,
        iris::file::PoseTemplate<float>::template Template
    >;

extern template struct pex::Group
    <
        iris::file::FileFields,
        iris::file::PoseTemplate<double>::template Template
    >;

extern template struct pex::Group
    <
        iris::ProjectionFields,
        iris::ProjectionTemplate<float>::template Template
    >;

extern template struct pex::Group
    <
        iris::ProjectionFields,
        iris::ProjectionTemplate<double>::template Template
    >;


extern template struct pex::Group
    <
        iris::StereoProjectionFields,
        iris::StereoProjectionTemplate<float>::template Template
    >;

extern template struct pex::Group
    <
        iris::StereoProjectionFields,
        iris::StereoProjectionTemplate<double>::template Template
    >;
