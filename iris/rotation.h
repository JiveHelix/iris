#pragma once

#include <iterator>
#include <fields/fields.h>
#include <pex/group.h>
#include <pex/interface.h>
#include <Eigen/Dense>
#include <cmath>
#include <tau/angles.h>
#include "iris/error.h"


namespace iris
{


CREATE_EXCEPTION(RotationError, IrisError);


template<typename T>
using Rotation = Eigen::Matrix<T, 3, 3>;


template<size_t axis, typename T>
Rotation<T> MakeAxial(T rotation_deg)
{
    static_assert(axis < 3, "Designed for 3-D only");

    auto rotation_rad = tau::ToRadians(rotation_deg);
    auto sine = std::sin(rotation_rad);
    auto cosine = std::cos(rotation_rad);

    if constexpr (axis == 0)
    {
        return Rotation<T>{
            {1.0, 0.0, 0.0},
            {0.0, cosine, -sine},
            {0.0, sine, cosine}};
    }
    else if constexpr (axis == 1)
    {
        return Rotation<T>{
            {cosine, 0.0, sine},
            {0.0, 1.0, 0.0},
            {-sine, 0.0, cosine}};
    }
    else
    {
        // axis == 2
        return Rotation<T>{
            {cosine, -sine, 0.0},
            {sine, cosine, 0.0},
            {0.0, 0.0, 1.0}};
    }
}


template<typename T>
Rotation<T> MakeAxial(size_t axis, T rotation_deg)
{
    switch (axis)
    {
        case 0:
            return MakeAxial<0, T>(rotation_deg);
        case 1:
            return MakeAxial<1, T>(rotation_deg);
        case 2:
            return MakeAxial<2, T>(rotation_deg);
        default:
            throw RotationError("Unsupported axis");
    }
}


template<size_t first, size_t second, size_t third, typename T>
Rotation<T> MakeIntrinsic(
    T first_deg,
    T second_deg,
    T third_deg)
{
    return MakeAxial<first>(first_deg)
        * MakeAxial<second>(second_deg)
        * MakeAxial<third>(third_deg);
}


struct AxisOrder
{
    static const std::array<std::string, 3> axisNames;

    size_t first;
    size_t second;
    size_t third;

    std::ostream & ToStream(std::ostream &outputStream) const
    {
        return outputStream << axisNames.at(this->first) << "-"
            << axisNames.at(this->second) << "-"
            << axisNames.at(this->third);
    }
};

inline const std::array<std::string, 3>
AxisOrder::axisNames{"roll", "pitch", "yaw"};


struct AxisOrderConverter
{
    std::string ToString(const AxisOrder &axisOrder)
    {
        std::ostringstream outputStream;
        axisOrder.ToStream(outputStream);
        return outputStream.str();
    }

    AxisOrder ToValue(const std::string &asString)
    {
        auto axisNames = jive::strings::Split(asString, '-');

        if (axisNames.size() != 3)
        {
            throw RotationError("Expected 3 axis names");
        }

        std::vector<size_t> axes;

        auto begin = std::begin(AxisOrder::axisNames);
        auto end = std::end(AxisOrder::axisNames);

        for (auto &name: axisNames)
        {
            auto found = std::find(begin, end, name);

            if (found == end)
            {
                throw RotationError("Unexpected axis name " + name);
            }

            axes.push_back(static_cast<size_t>(std::distance(begin, found)));
        }

        return {axes.at(0), axes.at(1), axes.at(2)};
    }
};


inline
std::ostream & operator<<(
    std::ostream &outputStream,
    const AxisOrder &axisOrder)
{
    return axisOrder.ToStream(outputStream);
}


template<typename T>
Rotation<T> MakeIntrinsic(
    const AxisOrder &axisOrder,
    T first_deg,
    T second_deg,
    T third_deg)
{
    return MakeAxial(axisOrder.first, first_deg)
        * MakeAxial(axisOrder.second, second_deg)
        * MakeAxial(axisOrder.third, third_deg);
}


template<typename T>
struct AnglesFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::yaw, "yaw"),
        fields::Field(&T::pitch, "pitch"),
        fields::Field(&T::roll, "roll"),
        fields::Field(&T::axisOrder, "axisOrder"));
};


template<typename U>
struct AnglesTemplate
{
    using AngleRange = pex::MakeRange<U, pex::Limit<-180>, pex::Limit<180>>;

    template<template<typename> typename T>
    struct Template
    {
        T<AngleRange> yaw;
        T<AngleRange> pitch;
        T<AngleRange> roll;
        T<pex::MakeSelect<AxisOrder>> axisOrder;

        static constexpr auto fields = AnglesFields<Template>::fields;
        static constexpr auto fieldsTypeName = "Angles";
    };
};


template<typename T>
struct Angles:
    public AnglesTemplate<T>::template Template<pex::Identity>
{
    using Base =
        typename AnglesTemplate<T>::template Template<pex::Identity>;

    static constexpr auto defaultAxisOrder = AxisOrder{2, 1, 0};

    Angles()
        :
        Base{0, 0, 0, defaultAxisOrder}
    {

    }

    Angles(
        T first,
        T second,
        T third,
        const AxisOrder &axisOrder_ = defaultAxisOrder)
    {
        this->axisOrder = axisOrder_;
        (*this)(axisOrder_.first) = first;
        (*this)(axisOrder_.second) = second;
        (*this)(axisOrder_.third) = third;
    }

    static Angles Default()
    {
        return Angles();
    }

    Angles(
        const Rotation<T> &rotation,
        const AxisOrder &axisOrder_ = defaultAxisOrder)
    {
        using Vector = Eigen::Vector<T, 3>;

        using Eigen::Index;

        Vector angles = tau::ToDegrees(
            rotation.eulerAngles(
                static_cast<Index>(axisOrder_.first),
                static_cast<Index>(axisOrder_.second),
                static_cast<Index>(axisOrder_.third)));

        std::cout << "AxisOrder: " << axisOrder_ << std::endl;
        std::cout << "Angles from rotation: " << angles << std::endl;

        (*this)(axisOrder_.first) = angles(0);
        (*this)(axisOrder_.second) = angles(1);
        (*this)(axisOrder_.third) = angles(2);
        this->axisOrder = axisOrder_;
    }

    T & operator()(size_t axis)
    {
        switch (axis)
        {
            case 0:
                return this->roll;

            case 1:
                return this->pitch;

            case 2:
                return this->yaw;

            default:
                throw RotationError("out of bounds index");
        }
    }

    T operator()(size_t axis) const
    {
        return const_cast<Angles<T> *>(this)->operator()(axis);
    }

    Rotation<T> GetRotation() const
    {
        return MakeIntrinsic(
            this->axisOrder,
            (*this)(this->axisOrder.first),
            (*this)(this->axisOrder.second),
            (*this)(this->axisOrder.third));
    }
};


TEMPLATE_OUTPUT_STREAM(Angles)


template<typename T>
using AnglesGroup =
    pex::Group
    <
        AnglesFields,
        AnglesTemplate<T>::template Template,
        Angles<T>
    >;

template<typename T>
struct AnglesModel: AnglesGroup<T>::Model
{
    using Base = AnglesGroup<T>::Model;

    AnglesModel()
        :
        Base(Angles<T>::Default())
    {
        this->axisOrder.SetChoices(
            {
                {2, 1, 0},
                {2, 0, 1},
                {1, 2, 0},
                {1, 0, 2},
                {0, 2, 1},
                {0, 1, 2}});
    }
};


template<typename T>
using AnglesControl =
    typename AnglesGroup<T>::template Control<void>;

template<typename T, typename Observer>
using AnglesTerminus =
    typename AnglesGroup<T>::template Terminus<Observer>;

template<typename T>
using AnglesGroupMaker =
    pex::MakeGroup<AnglesGroup<T>, AnglesModel<T>>;


template<typename T>
Rotation<T> AboutX(T rotation_deg)
{
    return MakeAxial<0>(rotation_deg);
}


template<typename T>
Rotation<T> AboutY(T rotation_deg)
{
    return MakeAxial<1>(rotation_deg);
}


template<typename T>
Rotation<T> AboutZ(T rotation_deg)
{
    return MakeAxial<2>(rotation_deg);
}


/***** Intrinsic Rotation Matrices *****/

/* Note: Yaw-Pitch-Roll intrinsic rotation is computed as Roll-Pitch-Yaw */

/* Yaw-Pitch-Roll using camera coordinate system
 *      Y positive down (row of image sensor)
 *      X positive left to right (column of image sensor (pitch axis)
 *      Z perpendicular to image plane (roll axis)
 */
template<typename T>
Rotation<T> MakeYxz(T y_deg, T x_deg, T z_deg)
{
    return MakeIntrinsic<1, 0, 2>(y_deg, x_deg, z_deg);
}


/* Yaw-Pitch-Roll using world coordinate system
 *      X positive Forward (roll axis)
 *      Y positive to left (pitch axis)
 *      Z up (yaw axis)
 */
template<typename T>
Rotation<T> MakeYawPitchRoll(T yaw, T pitch, T roll)
{
    return MakeIntrinsic<2, 1, 0>(yaw, pitch, roll);
}


/* Pitch-Yaw-Roll using world coordinate system
 *      X positive Forward (roll axis)
 *      Y positive to left (pitch axis)
 *      Z up (yaw axis)
 */
template<typename T>
Rotation<T> MakePitchYawRoll(T pitch, T yaw, T roll)
{
    return MakeIntrinsic<1, 2, 0>(pitch, yaw, roll);
}


/* The rotation of the sensor axes relative to the world axes */
template<typename T>
Rotation<T> SensorRelativeToWorld()
{
    // To move from world coordinates to sensor coordinates:
    // Yaw is -90
    // Pitch is 0
    // Roll is -90
    return MakeYawPitchRoll(
        static_cast<T>(-90),
        static_cast<T>(0),
        static_cast<T>(-90));
}

/* The rotation of the world axes relative to the sensor axes */
template<typename T>
Rotation<T> WorldRelativeToSensor()
{
    // To move from sensor coordinates to world coordinates:
    // Yaw is 90
    // Pitch is -90
    // Roll is 0
    return MakeYawPitchRoll(
        static_cast<T>(90),
        static_cast<T>(-90),
        static_cast<T>(0));
}


} // namespace iris
