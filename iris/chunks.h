#pragma once


#include <vector>
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
    Eigen::Index processCount)
{
    if (threadCount == 0)
    {
        throw ChunkError("threadCount must be greater than 0");
    }

    using Eigen::Index;

    Index chunkCount =
        processCount / static_cast<Index>(threadCount);

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


struct RowFunctors
{
    template<typename Derived>
    static Chunks MakeChunks(
        size_t threadCount,
        const Eigen::MatrixBase<Derived> &data)
    {
        return chunk::MakeChunks(threadCount, data.rows());
    }

    template<typename Kernel, typename Derived>
    static Derived Filter(
        const Eigen::MatrixBase<Kernel> &kernel,
        const Eigen::MatrixBase<Derived> &data,
        const Chunk &chunk)
    {
        assert(kernel.rows() == 1);

        return tau::DoConvolve(
                data.block(chunk.index, 0, chunk.count, data.cols()).eval(),
                kernel);
    }

    template<typename Derived, typename ThreadResults>
    static void Get(
        Eigen::MatrixBase<Derived> &result,
        ThreadResults &threadResults)
    {
        for (auto &threadResult: threadResults)
        {
            auto &chunk = threadResult.second;

            result.block(chunk.index, 0, chunk.count, result.cols()) =
                threadResult.first.get();
        }
    }
};


struct ColumnFunctors
{
    template<typename Derived>
    static Chunks MakeChunks(
        size_t threadCount,
        const Eigen::MatrixBase<Derived> &data)
    {
        return chunk::MakeChunks(threadCount, data.cols());
    }


    template<typename Kernel, typename Derived>
    static Derived Filter(
        const Eigen::MatrixBase<Kernel> &kernel,
        const Eigen::MatrixBase<Derived> &data,
        const Chunk &chunk)
    {
        assert(kernel.cols() == 1);

        return tau::DoConvolve(
            data.block(0, chunk.index, data.rows(), chunk.count).eval(),
            kernel);
    }


    template<typename Derived, typename ThreadResults>
    static void Get(
        Eigen::MatrixBase<Derived> &result,
        ThreadResults &threadResults)
    {
        for (auto &threadResult: threadResults)
        {
            auto &chunk = threadResult.second;

            result.block(0, chunk.index, result.rows(), chunk.count) =
                threadResult.first.get();
        }
    }
};


template<typename Functors, typename Kernel, typename Data>
class PartialConvolution
{
public:
    PartialConvolution(
        const Kernel &kernel,
        const Eigen::MatrixBase<Data> &data,
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
                    Functors::template Filter<Kernel, Data>,
                    kernel,
                    data.derived(),
                    chunk),
                chunk);
        }
    }

    Data Get()
    {
        Data result(this->rowCount, this->columnCount);
        Functors::Get(result, this->threadResults);
        return result;
    }

private:
    Eigen::Index rowCount;
    Eigen::Index columnCount;
    std::vector<std::pair<std::future<Data>, Chunk>> threadResults;
};


template<typename Kernel, typename Data>
class RowConvolution
    :
    public PartialConvolution<RowFunctors, Kernel, Data>
{
public:
    RowConvolution(
        const Kernel &kernel,
        const Eigen::MatrixBase<Data> &data,
        size_t threadCount)
        :
        PartialConvolution<RowFunctors, Kernel, Data>(
            kernel,
            data,
            threadCount)
    {

    }
};


template<typename Kernel, typename Data>
class ColumnConvolution
    :
    public PartialConvolution<ColumnFunctors, Kernel, Data>
{
public:
    ColumnConvolution(
        const Kernel &kernel,
        const Eigen::MatrixBase<Data> &data,
        size_t threadCount)
        :
        PartialConvolution<ColumnFunctors, Kernel, Data>(
            kernel,
            data,
            threadCount)
    {

    }
};


} // end namespace chunk


} // end namespace iris
