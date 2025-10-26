#pragma once

#include <cassert>
#include <cmath>
#include <future>
#include <optional>
#include <fields/fields.h>
#include <jive/overflow.h>

#include <tau/eigen.h>
#include <tau/convolve.h>
#include <tau/angles.h>
#include <tau/mono_image.h>

#include "iris/error.h"
#include "iris/gaussian_settings.h"
#include "iris/chunks.h"


namespace iris
{


template<typename T>
struct GaussianKernelFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::sigma, "sigma"),
        fields::Field(&T::threshold, "threshold"),
        fields::Field(&T::threads, "threads"),
        fields::Field(&T::size, "size"),
        fields::Field(&T::rowKernel, "rowKernel"),
        fields::Field(&T::sum, "sum"));
};


template<typename T, size_t order>
Eigen::VectorX<T> Sample(T sigma, size_t size)
{
    using Vector = Eigen::VectorX<T>;

    T count = static_cast<T>(size);
    auto range = (count - 1) / 2;

    Vector x = Vector::LinSpaced(
        static_cast<Eigen::Index>(size),
        -range,
        range);

    Vector exponential = (x.array().pow(2.0) / (-2 * sigma * sigma)).exp();

    if constexpr (order == 0)
    {
        T divisor = sigma * std::sqrt(static_cast<T>(2.0) * tau::Angles<T>::pi);
        return exponential.array() / divisor;
    }
    else if constexpr (order == 1)
    {
        T divisor = sigma * sigma * sigma
            * std::sqrt(static_cast<T>(2.0) * tau::Angles<T>::pi);

        return -x.array() * exponential.array() / divisor;
    }
    else
    {
        static_assert(order < 2);
    }
}


struct RowFunctors: public chunk::RowFunctors
{
    template<typename Kernel, typename Derived>
    static Derived Filter(
        const Kernel &kernel,
        const Eigen::MatrixBase<Derived> &data,
        const chunk::Chunk &chunk)
    {
        Derived partial =
            chunk::RowFunctors::Filter(kernel.rowKernel, data, chunk);

        static constexpr bool isIntegral =
            std::is_integral_v<typename Kernel::Type>;

        if constexpr (isIntegral)
        {
            partial.array() /= kernel.sum;
        }

        return partial;
    }
};


struct ColumnFunctors: public chunk::ColumnFunctors
{
    template<typename Kernel, typename Derived>
    static Derived Filter(
        const Kernel &kernel,
        const Eigen::MatrixBase<Derived> &data,
        const chunk::Chunk &chunk)
    {
        Derived partial =
            chunk::ColumnFunctors::Filter(kernel.columnKernel, data, chunk);

        static constexpr bool isIntegral =
            std::is_integral_v<typename Kernel::Type>;

        if constexpr (isIntegral)
        {
            partial.array() /= kernel.sum;
        }

        return partial;
    }
};


template
<
    typename Functors,
    typename Kernel,
    typename Derived
>
class ThreadedGaussian
{
public:
    ThreadedGaussian(
        const Kernel &kernel,
        const Eigen::MatrixBase<Derived> &data,
        size_t threadCount)
        :
        rowCount(data.rows()),
        columnCount(data.cols()),
        threadResults()
    {
        auto chunks = Functors::MakeChunks(threadCount, data.derived());

        for (auto &chunk: chunks)
        {
            this->threadResults.emplace_back(
                std::async(
                    std::launch::async,
                    Functors::template Filter<Kernel, Derived>,
                    kernel,
                    data.derived(),
                    chunk),
                chunk);
        }
    }

    Derived Get()
    {
        Derived result(this->rowCount, this->columnCount);
        Functors::Get(result, this->threadResults);
        return result;
    }

private:
    Eigen::Index rowCount;
    Eigen::Index columnCount;
    std::vector<std::pair<std::future<Derived>, chunk::Chunk>> threadResults;
};


template<typename Kernel, typename Derived>
class ThreadedRowGaussian
    :
    public ThreadedGaussian<RowFunctors, Kernel, Derived>
{
public:
    ThreadedRowGaussian(
        const Kernel &kernel,
        const Eigen::MatrixBase<Derived> &data,
        size_t threadCount)
        :
        ThreadedGaussian<RowFunctors, Kernel, Derived>(
            kernel,
            data,
            threadCount)
    {
        assert(kernel.rowKernel.rows() == 1);
    }
};


template<typename Kernel, typename Derived>
class ThreadedColumnGaussian
    :
    public ThreadedGaussian<ColumnFunctors, Kernel, Derived>
{
public:
    ThreadedColumnGaussian(
        const Kernel &kernel,
        const Eigen::MatrixBase<Derived> &data,
        size_t threadCount)
        :
        ThreadedGaussian<ColumnFunctors, Kernel, Derived>(
            kernel,
            data,
            threadCount)
    {
        assert(kernel.columnKernel.cols() == 1);
    }
};


template
<
    typename Kernel,
    typename Derived
>
Derived ThreadedKernelConvolve(
    const Kernel &kernel,
    const Eigen::MatrixBase<Derived> &data,
    Partials partials,
    size_t threadCount)
{
    using Eigen::Index;

    if (partials == Partials::both)
    {
        auto rowResult = ThreadedRowGaussian(kernel, data, threadCount).Get();
        return ThreadedColumnGaussian(kernel, rowResult, threadCount).Get();
    }
    else if (partials == Partials::rows)
    {
        return ThreadedRowGaussian(kernel, data, threadCount).Get();
    }
    else if (partials == Partials::columns)
    {
        return ThreadedColumnGaussian(kernel, data, threadCount).Get();
    }
    else
    {
        return data;
    }
}


template
<
    typename Kernel,
    typename Derived
>
Derived KernelConvolve(
    const Kernel &kernel,
    const Eigen::MatrixBase<Derived> &data,
    Partials partials,
    size_t threadCount = 1)
{
    if (threadCount > 1)
    {
        return ThreadedKernelConvolve(kernel, data, partials, threadCount);
    }

    using Eigen::Index;

    assert(kernel.rowKernel.rows() == 1);
    assert(kernel.columnKernel.cols() == 1);

    static constexpr bool isIntegral =
        std::is_integral_v<typename Kernel::Type>;

    if (partials == Partials::both)
    {
        Derived partial = tau::DoConvolve2d(data, kernel.rowKernel);

        if constexpr (isIntegral)
        {
            partial.array() /= kernel.sum;
        }

        Derived result = tau::DoConvolve2d(partial, kernel.columnKernel);

        if constexpr (isIntegral)
        {
            result.array() /= kernel.sum;
        }

        return result;
    }
    else if (partials == Partials::rows)
    {
        Derived result = tau::DoConvolve2d(data, kernel.rowKernel);

        if constexpr (isIntegral)
        {
            result.array() /= kernel.sum;
        }

        return result;
    }
    else if (partials == Partials::columns)
    {
        Derived result = tau::DoConvolve2d(data, kernel.columnKernel);

        if constexpr (isIntegral)
        {
            result.array() /= kernel.sum;
        }

        return result;
    }
    else
    {
        return data;
    }
}


template<typename T, typename S, size_t order, typename Enable = void>
struct GaussianKernel
{

};


template<typename T, typename S, size_t order>
struct GaussianKernel
<
    T,
    S,
    order,
    std::enable_if_t<std::is_floating_point_v<T>>
>
{
    static_assert(std::is_floating_point_v<S>);
    using Type = T;

    using ColumnVector = Eigen::VectorX<T>;
    using RowVector = Eigen::RowVectorX<T>;
    using Matrix = Eigen::MatrixX<T>;

    GaussianKernel() = default;

    static S GetRadius(S sigma, S threshold)
    {
        // Using the zeroth gaussian to estimate kernel size.
        S scale = sigma * std::sqrt(static_cast<S>(2.0) * tau::Angles<S>::pi);
        S centerValue = static_cast<S>(1.0) / scale;
        S edgeValue = centerValue * threshold;

        return std::sqrt(
            static_cast<S>(-2.0) * sigma * sigma * std::log(scale * edgeValue));
    }

    GaussianKernel(S sigma_, S threshold_, Partials partials_, size_t threads_)
        :
        sigma(sigma_),
        threshold(threshold_),
        partials(partials_),
        threads(threads_),
        size(
            static_cast<size_t>(
                1 + 2 * std::round(GetRadius(sigma, threshold))))
    {
        this->columnKernel =
            Sample<T, order>(static_cast<T>(sigma), this->size);

        this->rowKernel = this->columnKernel.transpose();

        assert(this->rowKernel.rows() == 1);
        assert(this->columnKernel.cols() == 1);

        this->sum = this->columnKernel.sum();
    }

    GaussianKernel(const GaussianSettings<T> &settings)
        :
        GaussianKernel(
            settings.sigma,
            settings.threshold,
            settings.partials,
            settings.threads)
    {

    }

    GaussianKernel<T, S, 0> Normalize() const
    {
        // Some truncation occurs depending on the threshold, and the sum will
        // be close to 1.0.
        // Scale the kernel to unity gain.
        Eigen::MatrixX<T> combined = this->columnKernel * this->rowKernel;
        T correction = std::sqrt(combined.sum());
        GaussianKernel<T, S, 0> result(*this);
        result.columnKernel.array() /= correction;
        result.rowKernel.array() /= correction;
        result.sum = result.columnKernel.sum();

        return result;
    }

    Matrix GetMatrix() const
    {
        return this->columnKernel * this->rowKernel;
    }

    template<typename Derived>
    Derived Filter(const Eigen::MatrixBase<Derived> &data) const
    {
        return KernelConvolve(*this, data, this->partials, threads);
    }

    S sigma;
    S threshold;
    Partials partials;
    size_t threads;
    size_t size;
    RowVector rowKernel;
    ColumnVector columnKernel;
    T sum;

    static constexpr auto fields = GaussianKernelFields<GaussianKernel>::fields;
};


template<typename T, typename S, size_t order>
struct GaussianKernel<T, S, order, std::enable_if_t<std::is_integral_v<T>>>
{
    static_assert(std::is_floating_point_v<S>);
    using Type = T;

    using ColumnVector = Eigen::VectorX<T>;
    using RowVector = Eigen::RowVectorX<T>;
    using Matrix = Eigen::MatrixX<T>;

    GaussianKernel() = default;

    GaussianKernel(
        S sigma_,
        T maximum,
        S threshold_,
        Partials partials_,
        size_t threads_)
        :
        sigma(sigma_),
        threshold(threshold_),
        partials(partials_),
        threads(threads_),
        size()
    {
        using Eigen::Index;

        // Use a floating-point kernel to design our integral kernel.
        GaussianKernel<S, S, order> designKernel(
            sigma_,
            threshold_,
            partials_,
            threads_);

        auto normalized = designKernel.Normalize();

        // A normalized floating-point kernel sums to 1.
        // Find the maximum scale
        S maximumScale = std::floor(
            static_cast<S>(
                std::numeric_limits<T>::max() / maximum));

        Index startingIndex = 0;
        S scale = 1.0 / normalized.columnKernel(startingIndex);

        Index midpoint = static_cast<Index>(normalized.size - 1) / 2;

        while (scale > maximumScale && startingIndex < midpoint)
        {
            scale = 1.0 / normalized.columnKernel(++startingIndex);
        }

        if (startingIndex >= midpoint)
        {
            throw IrisError("Unable to create integral filter");
        }

        Index taps = ((midpoint - startingIndex) * 2) + 1;

        this->columnKernel =
            (normalized.columnKernel.array() * scale).round()
                .segment(startingIndex, taps).template cast<T>();

        this->rowKernel = this->columnKernel.transpose();

        assert(this->rowKernel.rows() == 1);
        assert(this->columnKernel.cols() == 1);

        this->size = static_cast<size_t>(taps);

        this->threshold =
            1.0 / static_cast<S>(this->columnKernel.maxCoeff());

        this->sum = this->columnKernel.sum();

        // Assert that data will not be lost to overflow.
        assert(
            std::numeric_limits<T>::max() / this->sum >= maximum);
    }

    GaussianKernel(const GaussianSettings<T> &settings)
        :
        GaussianKernel(
            settings.sigma,
            settings.maximum,
            settings.threshold,
            settings.partials,
            settings.threads)
    {

    }

    Matrix GetMatrix() const
    {
        return this->columnKernel * this->rowKernel;
    }

    template<typename Derived>
    Derived Filter(const Eigen::MatrixBase<Derived> &data) const
    {
        return KernelConvolve(*this, data, partials, threads);
    }

    S sigma;
    S threshold;
    Partials partials;
    size_t threads;
    size_t size;
    RowVector rowKernel;
    ColumnVector columnKernel;
    T sum;

    static constexpr auto fields = GaussianKernelFields<GaussianKernel>::fields;
};


template<typename Value, size_t order, typename S = double>
class Gaussian
{
public:
    using Result = tau::MonoImage<Value>;
    using Matrix = Result;
    using Kernel = GaussianKernel<Value, S, order>;

    Gaussian()
        :
        isEnabled_(false),
        kernel_()
    {

    }

    Gaussian(const GaussianSettings<Value> &settings)
        :
        isEnabled_(settings.enable),
        kernel_(settings)
    {

    }

    std::optional<Result> FilterNoExtend(const Matrix &data) const
    {
        if (!this->isEnabled_)
        {
            // Pass through the input.
            return data;
        }

        return this->kernel_.Filter(data);
    }

    std::optional<Result> Filter(const Matrix &data) const
    {
        if (!this->isEnabled_)
        {
            // Pass through the input.
            return data;
        }

        auto size = static_cast<Eigen::Index>(this->kernel_.size);
        auto filtered = this->kernel_.Filter(tau::Extend(data, size, size));
        return filtered.block(size, size, data.rows(), data.cols());
    }

    size_t GetSize() const
    {
        return this->kernel_.size;
    }

private:
    bool isEnabled_;
    Kernel kernel_;
};


extern template class Gaussian<int32_t, 0, double>;
using DefaultGaussian = Gaussian<int32_t, 0, double>;

} // end namespace iris
