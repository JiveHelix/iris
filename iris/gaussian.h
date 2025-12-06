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
        fields::Field(&T::columnKernel, "columnKernel"),
        fields::Field(&T::rowKernelSum, "rowKernelSum"),
        fields::Field(&T::columnKernelSum, "columnKernelSum"),
        fields::Field(&T::sum, "sum"));
};


template<typename T, size_t order>
Eigen::VectorX<T> Sample(T sigma, Eigen::Index size)
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


template<bool normalize>
struct RowFunctors: public chunk::RowFunctors<normalize>
{
    static constexpr bool isByRow = true;

    template<typename Kernel, typename Input, typename Output>
    static void Filter(
        const Kernel &kernel,
        const Eigen::MatrixBase<Input> &input,
        Eigen::MatrixBase<Output> &output,
        const chunk::Chunk &chunk)
    {
        chunk::RowFunctors<normalize>::Filter(
            kernel.rowKernel,
            input,
            output,
            chunk);
    }
};


template<bool normalize>
struct ColumnFunctors: public chunk::ColumnFunctors<normalize>
{
    static constexpr bool isByRow = false;

    template<typename Kernel, typename Input, typename Output>
    static void Filter(
        const Kernel &kernel,
        const Eigen::MatrixBase<Input> &input,
        Eigen::MatrixBase<Output> &output,
        const chunk::Chunk &chunk)
    {
        chunk::ColumnFunctors<normalize>::Filter(
            kernel.columnKernel,
            input,
            output,
            chunk);
    }
};


template
<
    typename Functors,
    typename Kernel,
    typename Input,
    typename Output
>
class ThreadedGaussian
{
public:
    using Period = std::chrono::duration<double, std::micro>;
    using Clock = std::chrono::steady_clock;

    ThreadedGaussian(
        const Kernel &kernel,
        const Eigen::MatrixBase<Input> &input,
        Eigen::MatrixBase<Output> &output,
        size_t threadCount)
        :
        rowCount(input.rows()),
        columnCount(input.cols()),
        threadPool_(jive::GetThreadPool()),
        threadSentries_(),
        beginTime_(Clock::now())
    {
        assert(kernel.columnKernel.rows() < input.rows());
        assert(kernel.rowKernel.cols() < input.cols());

        // Output localCopy = output.derived();;

        auto chunks = Functors::MakeChunks(threadCount, input.derived());

        auto makeChunksTime = Clock::now();

        this->threadSentries_.reserve(chunks.size());

        for (auto &chunk: chunks)
        {
            this->threadSentries_.emplace_back(
                this->threadPool_->AddJob(
                    [chunk, &kernel, &input, &output]
                    {
                        Functors::template Filter<Kernel, Input, Output>(
                            kernel,
                            input,
                            output,
                            chunk);
                    }));
        }

        auto totalQueueTime = std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - makeChunksTime).count();

        std::cout << "\n\ngaussian threads: " << chunks.size();

        std::cout << "\nmakeChunksTime (us): "
            << std::chrono::duration_cast<std::chrono::microseconds>(makeChunksTime - this->beginTime_).count() << std::endl;

        std::cout << "totalQueueTime (us): " << totalQueueTime << std::endl;
    }

    void Await()
    {
        chunk::AwaitThreads(this->threadSentries_);
    }

private:
    Eigen::Index rowCount;
    Eigen::Index columnCount;
    std::shared_ptr<jive::ThreadPool> threadPool_;
    std::vector<jive::Sentry> threadSentries_;
    Clock::time_point beginTime_;
};


template<typename Kernel, typename Input, typename Output>
class ThreadedRowGaussian
    :
    public ThreadedGaussian
    <
        RowFunctors<Kernel::normalize>,
        Kernel,
        Input,
        Output
    >
{
public:
    using Base =
        ThreadedGaussian
        <
            RowFunctors<Kernel::normalize>,
            Kernel,
            Input,
            Output
        >;

    ThreadedRowGaussian(
        const Kernel &kernel,
        const Eigen::MatrixBase<Input> &input,
        Eigen::MatrixBase<Output> &output,
        size_t threadCount)
        :
        Base(kernel, input, output, threadCount)
    {
        assert(kernel.rowKernel.rows() == 1);
    }
};


template<typename Kernel, typename Input, typename Output>
class ThreadedColumnGaussian
    :
    public ThreadedGaussian
    <
        ColumnFunctors<Kernel::normalize>,
        Kernel,
        Input,
        Output
    >
{
public:
    using Base =
        ThreadedGaussian
        <
            ColumnFunctors<Kernel::normalize>,
            Kernel,
            Input,
            Output
        >;

    ThreadedColumnGaussian(
        const Kernel &kernel,
        const Eigen::MatrixBase<Input> &input,
        Eigen::MatrixBase<Output> &output,
        size_t threadCount)
        :
        Base(kernel, input, output, threadCount)
    {
        assert(kernel.columnKernel.cols() == 1);
    }
};


template<bool transpose, typename Kernel, typename Input, typename Output>
std::optional<int64_t> DoThreadedRowGaussian(
    const Kernel &kernel,
    const Eigen::MatrixBase<Input> &input,
    Eigen::MatrixBase<Output> &output,
    size_t threadCount)
{
    if constexpr (!transpose)
    {
        ThreadedRowGaussian(kernel, input, output, threadCount).Await();

        return {};
    }
    else
    {
        if constexpr (Input::IsRowMajor)
        {
            ThreadedRowGaussian(kernel, input, output, threadCount).Await();

            return {};
        }
        else
        {
            using namespace std::chrono;
            using Period = duration<double, std::micro>;
            using Clock = steady_clock;

            Clock::time_point beginTime = Clock::now();

            using Transposed =
                Eigen::Matrix
                <
                    typename Input::Scalar,
                    Input::RowsAtCompileTime,
                    Input::ColsAtCompileTime,
                    Eigen::RowMajor
                >;

            Transposed transposed = input;

            Clock::time_point endCopy = Clock::now();

            ThreadedRowGaussian(kernel, transposed, output, threadCount)
                .Await();

            return duration_cast<microseconds>(endCopy - beginTime).count();
        }
    }
}


template<bool transpose, typename Kernel, typename Input, typename Output>
std::optional<int64_t> DoThreadedColumnGaussian(
    const Kernel &kernel,
    const Eigen::MatrixBase<Input> &input,
    Eigen::MatrixBase<Output> &output,
    size_t threadCount)
{
    if constexpr (!transpose)
    {
        ThreadedColumnGaussian(kernel, input, output, threadCount).Await();

        return {};
    }
    else
    {
        if constexpr (!Input::IsRowMajor)
        {
            ThreadedColumnGaussian(kernel, input, output, threadCount).Await();

            return {};
        }
        else
        {
            using namespace std::chrono;
            using Period = duration<double, std::micro>;
            using Clock = steady_clock;

            Clock::time_point beginTime = Clock::now();

            using Transposed =
                Eigen::Matrix
                <
                    typename Input::Scalar,
                    Input::RowsAtCompileTime,
                    Input::ColsAtCompileTime,
                    Eigen::ColMajor
                >;

            Transposed transposed = input;

            Clock::time_point endCopy = Clock::now();

            ThreadedColumnGaussian(kernel, transposed, output, threadCount)
                .Await();

            return duration_cast<microseconds>(endCopy - beginTime).count();
        }
    }
}


template
<
    typename Kernel,
    typename Input,
    typename Output
>
void ThreadedKernelConvolve(
    const Kernel &kernel,
    const Eigen::MatrixBase<Input> &input,
    Eigen::MatrixBase<Output> &output,
    Partials partials,
    size_t threadCount)
{
    using Eigen::Index;

    using namespace std::chrono;
    using Period = duration<double, std::micro>;
    using Clock = steady_clock;

    if (partials == Partials::both)
    {
        Clock::time_point beginTime = Clock::now();

        auto rowCopyTime =
            DoThreadedRowGaussian<true>(kernel, input, output, threadCount);

        Clock::time_point endRowTime = Clock::now();

        auto colCopyTime =
            DoThreadedColumnGaussian<true>(kernel, output, output, threadCount);

        Clock::time_point endTime = Clock::now();

        auto rowTime = endRowTime - beginTime;
        auto colTime = endTime - endRowTime;
        auto totalTime = endTime - beginTime;

        fmt::print(
            "rowTime: {} us\n"
            "colTime: {} us\n"
            "totalTime: {} us\n"
            "kernel length: {}\n",
            duration_cast<microseconds>(rowTime).count(),
            duration_cast<microseconds>(colTime).count(),
            duration_cast<microseconds>(totalTime).count(),
            kernel.rowKernel.size());

        if (rowCopyTime)
        {
            fmt::print("rowCopyTime: {} us\n", *rowCopyTime);
        }

        if (colCopyTime)
        {
            fmt::print("colCopyTime: {} us\n", *colCopyTime);
        }
    }
    else if (partials == Partials::rows)
    {
        ThreadedRowGaussian(kernel, input, output, threadCount).Await();
    }
    else if (partials == Partials::columns)
    {
        ThreadedColumnGaussian(kernel, input, output, threadCount).Await();
    }
}


template
<
    typename Kernel,
    typename Input,
    typename Output
>
void KernelConvolve(
    const Kernel &kernel,
    const Eigen::MatrixBase<Input> &input,
    Eigen::MatrixBase<Output> &output,
    Partials partials,
    size_t threadCount = 1)
{
    static_assert(
        (Output::Flags & Eigen::LvalueBit) != 0,
        "output requires a writable (lvalue) matrix or block");

    if (threadCount >= 1)
    {
        std::cout << "ThreadedKernelConvolve" << std::endl;
        ThreadedKernelConvolve(kernel, input, output, partials, threadCount);

        return;
    }

    std::cout << "Non-threaded Kernel convolve" << std::endl;

    using Eigen::Index;

    assert(kernel.rowKernel.rows() == 1);
    assert(kernel.columnKernel.cols() == 1);

    static constexpr bool isIntegral =
        std::is_integral_v<typename Kernel::Type>;

    if (partials == Partials::both)
    {
        std::cout << "Partials::both" << std::endl;
        std::cout << "input max value: " << input.maxCoeff() << std::endl;
        tau::CorrelateRows(kernel.rowKernel, input, output);

        std::cout << "after correlate rows, output max value: "
            << output.maxCoeff() << std::endl;

        if constexpr (isIntegral)
        {
            output.array() /= kernel.rowKernelSum;

            std::cout << "normalize by " << kernel.rowKernelSum << std::endl;

            std::cout << "after normalize, output max value: "
                << output.maxCoeff() << std::endl;
        }

        tau::CorrelateColumns(kernel.columnKernel, output);

        if constexpr (isIntegral)
        {
            output.array() /= kernel.columnKernelSum;
        }
    }
    else if (partials == Partials::rows)
    {
        std::cout << "Partials::rows" << std::endl;
        tau::CorrelateRows(kernel.rowKernel, input, output);

        if constexpr (isIntegral)
        {
            output.array() /= kernel.rowKernelSum;
        }
    }
    else if (partials == Partials::columns)
    {
        std::cout << "Partials::columns" << std::endl;
        tau::CorrelateColumns(kernel.columnKernel, input, output);

        if constexpr (isIntegral)
        {
            output.array() /= kernel.columnKernelSum;
        }
    }
    else
    {
        output = input;
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
    // floating-point kernels are pre-normalized.
    static constexpr bool normalize = false;

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
            static_cast<Eigen::Index>(
                1 + 2 * std::round(GetRadius(sigma, threshold))))
    {
        this->columnKernel =
            Sample<T, order>(static_cast<T>(sigma), this->size);

        this->rowKernel = this->columnKernel.transpose();

        assert(this->rowKernel.rows() == 1);
        assert(this->columnKernel.cols() == 1);

        this->columnKernelSum = this->columnKernel.sum();
        this->rowKernelSum = this->rowKernel.sum();
        this->sum = (this->columnKernel * this->rowKernel).sum();
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
        GaussianKernel<T, S, 0> result(*this);

        Eigen::MatrixX<T> combined = this->columnKernel * this->rowKernel;
        T correction = std::sqrt(combined.sum());
        result.columnKernel.array() /= correction;
        result.rowKernel.array() /= correction;
        result.columnKernelSum = result.columnKernel.sum();
        result.rowKernelSum = result.rowKernel.sum();
        result.sum = 1.0;

        return result;
    }

    Matrix GetMatrix() const
    {
        return this->columnKernel * this->rowKernel;
    }

    template<typename Input, typename Output>
    void Filter(
        const Eigen::MatrixBase<Input> &input,
        Eigen::MatrixBase<Output> &output) const
    {
        KernelConvolve(*this, input, output, this->partials, this->threads);
    }

    S sigma;
    S threshold;
    Partials partials;
    size_t threads;
    Eigen::Index size;
    RowVector rowKernel;
    ColumnVector columnKernel;
    T rowKernelSum;
    T columnKernelSum;
    T sum;

    static constexpr auto fields = GaussianKernelFields<GaussianKernel>::fields;
};


template<typename T, typename S, size_t order>
struct GaussianKernel<T, S, order, std::enable_if_t<std::is_integral_v<T>>>
{
    // integer normals cannot be pre-normalized.
    static constexpr bool normalize = true;

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

        this->size = taps;

        this->threshold =
            1.0 / static_cast<S>(this->columnKernel.maxCoeff());

        this->rowKernelSum = this->rowKernel.sum();
        this->columnKernelSum = this->columnKernel.sum();
        this->sum = (this->columnKernel * this->rowKernel).sum();

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

    template<typename Input, typename Output>
    void Filter(
        const Eigen::MatrixBase<Input> &input,
        Eigen::MatrixBase<Output> &output) const
    {
        std::cout << "Integral gaussian kernel convolve" << std::endl;
        KernelConvolve(*this, input, output, this->partials, this->threads);
    }

    S sigma;
    S threshold;
    Partials partials;
    size_t threads;
    Eigen::Index size;
    RowVector rowKernel;
    ColumnVector columnKernel;
    T rowKernelSum;
    T columnKernelSum;
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
        std::cout << "Gaussian with settings:\n" << settings << std::endl;
    }

    bool Filter(const Matrix &input, Result &output) const
    {
        if (!this->isEnabled_)
        {
            return false;
        }

        this->kernel_.Filter(input, output);

        return true;
    }

    Eigen::Index GetSize() const
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
