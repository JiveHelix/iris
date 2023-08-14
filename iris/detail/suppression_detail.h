#pragma once


#include <future>
#include "iris/chunks.h"


namespace iris
{


namespace detail
{


template<typename Data>
void Suppress(
    Eigen::Index limitRow,
    Eigen::Index limitColumn,
    Eigen::Index windowSize,
    Eigen::MatrixBase<Data> &result)
{
    using Eigen::Index;

    Index x;
    Index y;

    if constexpr (tau::MatrixTraits<Data>::isColumnMajor)
    {
        for (Eigen::Index column = 0; column < limitColumn; ++column)
        {
            for (Eigen::Index row = 0; row < limitRow; ++row)
            {
                auto block = result.block(
                    row,
                    column,
                    windowSize,
                    windowSize);

                auto maximum = block.maxCoeff(&y, &x);
                block.array() = 0;
                block(y, x) = maximum;
            }
        }
    }
    else
    {
        for (Eigen::Index row = 0; row < limitRow; ++row)
        {
            for (Eigen::Index column = 0; column < limitColumn; ++column)
            {
                auto block = result.block(
                    row,
                    column,
                    windowSize,
                    windowSize);

                auto maximum = block.maxCoeff(&y, &x);
                block.array() = 0;
                block(y, x) = maximum;
            }
        }
    }
}


template<typename Data>
Data CopySuppress(
    Eigen::Index limitRow,
    Eigen::Index limitColumn,
    Eigen::Index windowSize,
    const Eigen::MatrixBase<Data> &input)
{
    Data result = input;
    Suppress(limitRow, limitColumn, windowSize, result);
    return result;
}


template<typename Data>
class SuppressionChunk
{
public:
    SuppressionChunk(
        chunk::Chunk rows,
        chunk::Chunk columns,
        Eigen::Index windowSize,
        const Eigen::MatrixBase<Data> &data)
    {
        using Eigen::Index;

        // Limit the bounds of the operation so that the window never exceeds
        // the bounds of data_;
        Index limitRow = rows.count - windowSize + 1;
        Index limitColumn = columns.count - windowSize + 1;

        this->result_ = std::async(
            std::launch::async,
            &CopySuppress<Data>,
            limitRow,
            limitColumn,
            windowSize,
            data.block(
                rows.index,
                columns.index,
                rows.count,
                columns.count).eval());
    }

    Data Get()
    {
        return this->result_.get();
    }

private:
    Data data_;
    std::future<Data> result_;
};


} // end namespace detail


} // end namespace iris
