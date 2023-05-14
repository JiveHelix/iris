#pragma once


#include <tau/vector3d.h>


namespace iris
{


template<typename T>
using Vector3 = Eigen::Vector<T, 3>;

template<typename T>
using Vector4 = Eigen::Vector<T, 4>;


template<typename T>
bool IsScaled(const Vector3<T> &first, const Vector3<T> &second)
{
    return first.normalized().isApprox(second.normalized());
}


template<typename T>
bool IsSameDirection(
    const Vector3<T> &first,
    const Vector3<T> &second,
    T threshold = static_cast<T>(0.9))
{
    return first.transpose().dot(second) > threshold;
}


template<typename T>
bool IsLinear(const Vector3<T> first, const Vector3<T> second)
{
    if (first.isApprox(second))
    {
        return true;
    }

    Vector3<T> reversed = second.array() * -1;

    return (first.isApprox(reversed));
}



template<typename T>
struct Point3d: public tau::Point3d<T>
{
    using Base = tau::Point3d<T>;
    using Base::Base;

    // Compare equal to 6 decimal places.
    static constexpr ssize_t precision = 6;

    Point3d(const Vector3<T> &vector_)
        :
        Point3d(vector_(0), vector_(1), vector_(2))
    {

    }

    Point3d(const tau::Point3d<T> &point)
        :
        Base(point)
    {

    }

    T operator()(Eigen::Index index) const
    {
        if (index == 0)
        {
            return this->x;
        }
        else if (index == 1)
        {
            return this->y;
        }
        else if (index == 2)
        {
            return this->z;
        }
        else
        {
            throw std::out_of_range("Index not valid for Point3d");
        }
    }

    T & operator()(Eigen::Index index)
    {
        if (index == 0)
        {
            return this->x;
        }
        else if (index == 1)
        {
            return this->y;
        }
        else if (index == 2)
        {
            return this->z;
        }
        else
        {
            throw std::out_of_range("Index not valid for Point3d");
        }
    }
};


TEMPLATE_OPERATOR_EQUALS(Point3d)
TEMPLATE_OUTPUT_STREAM(Point3d)


} // end namespace iris
