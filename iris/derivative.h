#pragma once


#include <string>
#include <ostream>
#include <tau/eigen.h>
#include <tau/convolve.h>
#include "iris/error.h"


namespace iris
{


struct DerivativeSize
{
    enum class Size
    {
        three,
        five,
        seven
    };

    using Index = Eigen::Index;

    struct SizeChoices
    {
        using Type = Size;

        static std::vector<Size> GetChoices()
        {
            return {Size::three, Size::five, Size::seven};
        }
    };

    using MakeSelect = pex::MakeSelect<SizeChoices>;
    using Control = pex::ControlSelector<MakeSelect>;

    static Index GetSize(Size size)
    {
        switch (size)
        {
            case Size::three:
                return 3;

            case Size::five:
                return 5;

            case Size::seven:
                return 7;

            default:
                throw IrisError("Not a supported size");
        }
    }

    struct SizeToString
    {
        static std::string ToString(Size size)
        {
            return std::to_string(GetSize(size));
        }
    };

    static std::vector<Size> GetValidSizes()
    {
        return SizeChoices::GetChoices();
    }

    template<typename T>
    static Eigen::VectorX<T> GetKernel(Index size, T scale)
    {
        switch (size)
        {
            case 3:
                return Eigen::VectorX<T>{{-scale, 0, scale}};

            case 5:
                return Eigen::VectorX<T>{
                    {-scale, -2 * scale, 0, 2 * scale, scale}};

            case 7:
                return Eigen::VectorX<T>{{
                    -scale,
                    -2 * scale,
                    -5 * scale,
                    0,
                    5 * scale,
                    2 * scale,
                    scale}};

            default:
                throw IrisError("Not a supported size");
        }
    }

    template<typename T>
    static T GetWeight(Index size)
    {
        // Returns the sum of all (unscaled) positive terms of the kernel.

        switch (size)
        {
            case 3:
                return 1;

            case 5:
                return 3;

            case 7:
                return 7;

            default:
                throw IrisError("Not a supported size");
        }
    }
};


inline
std::ostream & operator<<(std::ostream &outputStream, DerivativeSize::Size size)
{
    return outputStream << DerivativeSize::SizeToString::ToString(size);
}


template<typename T>
struct Differentiate
{
    static_assert(sizeof(T) >= 2);
    static_assert(std::is_signed_v<T>);

    using RowVector = Eigen::Matrix<T, 1, Eigen::Dynamic>;
    using ColumnVector = Eigen::Matrix<T, Eigen::Dynamic, 1>;

    Differentiate() = default;

    Differentiate(T maximum, T scale, DerivativeSize::Size size)
        :
        maximum_(maximum),
        scale_{scale},
        size_{DerivativeSize::GetSize(size)},
        horizontal(DerivativeSize::GetKernel<T>(this->size_, this->scale_)),
        vertical(this->horizontal.transpose())
    {
        if (
            (maximum
                * DerivativeSize::GetWeight<T>(this->size_))
                > std::numeric_limits<T>::max())
        {
            // Overflow may occur.
            throw IrisError(
                "Choose a larger data type to accomodate larger inputs.");
        }

        this->SetScale(scale);
    }

    T GetScale() const
    {
        return this->scale_;
    }

    T SetScale(T scale)
    {
        auto weight = DerivativeSize::GetWeight<T>(this->size_);
        auto weightedScale = scale * weight;

        if (std::numeric_limits<T>::max() / weightedScale < this->maximum_)
        {
            // maximum_ * weightScale > maximum T: overflow
            weightedScale = std::numeric_limits<T>::max() / this->maximum_;
        }

        this->scale_ = weightedScale / weight;

        // Maximum weight is 7, and the minimum sizeof(T) is 2, and
        // maximum_ has been checked to be low enough.
        assert(this->scale_ >= 1);

        this->horizontal =
            DerivativeSize::GetKernel<T>(this->size_, this->scale_);

        this->vertical = this->horizontal.transpose();

        return this->scale_;
    }

    T GetMaximum() const
    {
        return this->maximum_
            * DerivativeSize::GetWeight<T>(this->size_);
    }

    RowVector GetHorizontal() const
    {
        return this->horizontal;
    }

    ColumnVector GetVertical() const
    {
        return this->vertical;
    }

    template<typename Data>
    Data X(const Eigen::MatrixBase<Data> &data) const
    {
        return tau::CorrelateRows(data, this->horizontal);
    }

    template<typename Data>
    Data Y(const Eigen::MatrixBase<Data> &data) const
    {
        return tau::CorrelateColumns(data, this->vertical);
    }

    Eigen::Index GetSize() const
    {
        return this->size_;
    }

private:
    T maximum_;
    T scale_;
    Eigen::Index size_;

public:
    RowVector horizontal;
    ColumnVector vertical;
};


} // end namespace iris
