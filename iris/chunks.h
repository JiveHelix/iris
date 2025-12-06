#pragma once


#include <cassert>
#include <vector>
#include <jive/thread_pool.h>
#include <tau/eigen_shim.h>
#include <tau/convolve.h>

#include "iris/error.h"


namespace iris
{


namespace chunk
{


CREATE_EXCEPTION(ChunkError, IrisError);


struct Chunk
{
    Eigen::Index index;
    Eigen::Index count;
};


using Chunks = std::vector<Chunk>;


inline
Chunks MakeChunks(
    size_t threadCount,
    Eigen::Index processCount,
    std::optional<Eigen::Index> minimumChunkCount = std::nullopt)
{
    assert(processCount >= static_cast<Eigen::Index>(threadCount));

    if (threadCount == 0)
    {
        throw ChunkError("threadCount must be greater than 0");
    }

    using Eigen::Index;

    Index chunkCount =
        processCount / static_cast<Index>(threadCount);

    if (minimumChunkCount)
    {
        if (processCount < *minimumChunkCount)
        {
            throw std::logic_error("Inconsistent constraint");
        }

        while (chunkCount < minimumChunkCount && threadCount > 1)
        {
            --threadCount;
            chunkCount = processCount / static_cast<Index>(threadCount);
        }

        // The check and throw above guarantees that processCount >=
        // minimumChunkCount; therefore, this condition is guaranteed.
        assert(chunkCount >= *minimumChunkCount);
    }

    Chunks result(threadCount, Chunk{0, chunkCount});

    for (size_t i = 0; i < threadCount; ++i)
    {
        result[i].index = static_cast<Index>(i) * chunkCount;
    }

    Index remainder =
        processCount
        - (static_cast<Index>(threadCount) * chunkCount);

    if (remainder)
    {
        // Add the remainder to the last thread.
        result.back().count += remainder;
    }

    return result;
}


inline void AwaitThreads(std::vector<jive::Sentry> &sentries)
{
    for (auto &sentry: sentries)
    {
        sentry.Wait();
    }
}


template<bool normalize = false>
struct RowFunctors
{
    template<typename Derived>
    static Chunks MakeChunks(
        size_t threadCount,
        const Eigen::MatrixBase<Derived> &data)
    {
        return chunk::MakeChunks(threadCount, data.rows());
    }

    template<typename Kernel, typename Input, typename Output>
    static void Filter(
        const Eigen::MatrixBase<Kernel> &kernel,
        const Eigen::MatrixBase<Input> &input,
        Eigen::MatrixBase<Output> &output,
        const Chunk &chunk)
    {
        assert(kernel.rows() == 1);

        auto outputView = tau::MakeView(
            output.block(chunk.index, 0, chunk.count, output.cols()));

        tau::CorrelateRows(
            tau::ViewTag{},
            kernel,
            tau::MakeView(
                input.block(chunk.index, 0, chunk.count, input.cols())),
            outputView);

        if constexpr (normalize)
        {
            outputView.array() /= kernel.sum();
        }
    }
};


template<bool normalize = false>
struct ColumnFunctors
{
    template<typename Derived>
    static Chunks MakeChunks(
        size_t threadCount,
        const Eigen::MatrixBase<Derived> &data)
    {
        return chunk::MakeChunks(threadCount, data.cols());
    }


    template<typename Kernel, typename Input, typename Output>
    static void Filter(
        const Eigen::MatrixBase<Kernel> &kernel,
        const Eigen::MatrixBase<Input> &input,
        Eigen::MatrixBase<Output> &output,
        const Chunk &chunk)
    {
        auto outputView = tau::MakeView(
            output.block(0, chunk.index, output.rows(), chunk.count));

        tau::CorrelateColumns(
            tau::ViewTag{},
            kernel,
            tau::MakeView(
                input.block(0, chunk.index, input.rows(), chunk.count)),
            outputView);

        if constexpr (normalize)
        {
            outputView.array() /= kernel.sum();
        }
    }
};


template
<
    typename Functors,
    typename Kernel,
    typename Input,
    typename Output
>
class PartialConvolution
{
public:
    PartialConvolution(
        const Kernel &kernel,
        const Eigen::MatrixBase<Input> &input,
        Eigen::MatrixBase<Output> &output,
        size_t threadCount)
        :
        rowCount(input.rows()),
        columnCount(input.cols()),
        threadPool_(jive::GetThreadPool()),
        threadSentries_()
    {
        assert(this->rowCount > 0);
        assert(this->columnCount > 0);

        assert(this->rowCount == output.rows());
        assert(this->columnCount == output.cols());

        auto chunks = Functors::MakeChunks(threadCount, input.derived());

        for (auto &chunk: chunks)
        {
            this->threadSentries_.emplace_back(
                this->threadPool_->AddJob(
                    [chunk, &kernel, &input, &output]
                    {
                        return Functors::template Filter<Kernel, Input, Output>(
                            kernel,
                            input,
                            output,
                            chunk);
                    }));
        }
    }

    void Await()
    {
        AwaitThreads(this->threadSentries_);
    }

private:
    Eigen::Index rowCount;
    Eigen::Index columnCount;
    std::shared_ptr<jive::ThreadPool> threadPool_;
    std::vector<jive::Sentry> threadSentries_;
};


template<bool normalize, typename Kernel, typename Input, typename Output>
class RowConvolution
    :
    public PartialConvolution<RowFunctors<normalize>, Kernel, Input, Output>
{
public:
    RowConvolution(
        const Kernel &kernel,
        const Eigen::MatrixBase<Input> &input,
        Eigen::MatrixBase<Output> &output,
        size_t threadCount)
        :
        PartialConvolution<RowFunctors<normalize>, Kernel, Input, Output>(
            kernel,
            input,
            output,
            threadCount)
    {

    }
};


template<bool normalize, typename Kernel, typename Input, typename Output>
class ColumnConvolution
    :
    public PartialConvolution<ColumnFunctors<normalize>, Kernel, Input, Output>
{
public:
    ColumnConvolution(
        const Kernel &kernel,
        const Eigen::MatrixBase<Input> &input,
        Eigen::MatrixBase<Output> &output,
        size_t threadCount)
        :
        PartialConvolution<ColumnFunctors<normalize>, Kernel, Input, Output>(
            kernel,
            input,
            output,
            threadCount)
    {

    }
};


} // end namespace chunk


} // end namespace iris
