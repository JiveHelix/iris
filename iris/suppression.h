#pragma once


#include <fields/fields.h>
#include <pex/interface.h>
#include <pex/group.h>
#include <pex/selectors.h>
#include <pex/range.h>
#include <tau/eigen.h>
#include <tau/image.h>

#include "iris/detail/suppression_detail.h"


namespace iris
{


template<typename Data>
class AsyncSuppression
{
public:
    using Index = typename Eigen::Index;

    AsyncSuppression(
        size_t threadCount,
        Index windowSize,
        const Eigen::MatrixBase<Data> &data)
        :
        threadCount_(threadCount),
        windowSize_(windowSize),
        rows_(data.rows()),
        columns_(data.cols()),
        threads_()
    {
        auto chunks = this->MakeChunks_();

        if (chunks.at(0).count < windowSize)
        {
            throw IrisError("Chunk width is smaller than window size.");
        }

        if constexpr (tau::MatrixTraits<Data>::isColumnMajor)
        {
            if (this->rows_ < windowSize)
            {
                throw IrisError("Chunk height is smaller than window size.");
            }

            for (auto &chunk: chunks)
            {
                this->threads_.emplace_back(
                    chunk::Chunk{0, this->rows_},
                    chunk,
                    windowSize,
                    data);
            }
        }
        else
        {
            if (this->columns_ < windowSize)
            {
                throw IrisError("Chunk height is smaller than window size.");
            }

            for (auto &chunk: chunks)
            {
                this->threads_.emplace_back(
                    chunk,
                    chunk::Chunk{0, this->columns_},
                    windowSize,
                    data);
            }
        }
    }

    Data Get()
    {
        Data result(this->rows_, this->columns_);
        auto chunks = this->MakeChunks_();

        if constexpr (tau::MatrixTraits<Data>::isColumnMajor)
        {
            // Create chunks along the columns
            chunk::Chunk rowChunk{0, this->rows_};

            for (size_t i = 0; i < this->threads_.size(); ++i)
            {
                auto &columnChunk = chunks.at(i);

                result.block(
                    rowChunk.index,
                    columnChunk.index,
                    rowChunk.count,
                    columnChunk.count) = this->threads_[i].Get();
            }

            if (this->threadCount_ > 1)
            {
                this->ZipColumnMajor_(result, chunks);
            }
        }
        else
        {
            // Create chunks along the rows
            chunk::Chunk columnChunk{0, this->columns_};

            for (size_t i = 0; i < this->threads_.size(); ++i)
            {
                auto &rowChunk = chunks.at(i);

                result.block(
                    rowChunk.index,
                    columnChunk.index,
                    rowChunk.count,
                    columnChunk.count) = this->threads_[i].Get();
            }

            if (this->threadCount_ > 1)
            {
                this->ZipRowMajor_(result, chunks);
            }
        }

        return result;
    }

private:
    chunk::Chunks MakeChunks_() const
    {
        if constexpr (tau::MatrixTraits<Data>::isColumnMajor)
        {
            // Create chunks along the columns
            auto chunks = chunk::MakeChunks(this->threadCount_, this->columns_);
            assert(chunks.back().count >= chunks.at(0).count);
            return chunks;
        }
        else
        {
            // Create chunks along the rows
            auto chunks = chunk::MakeChunks(this->threadCount_, this->rows_);
            assert(chunks.back().count >= chunks.at(0).count);
            return chunks;
        }
    }

    void ZipColumnMajor_(Data &result, const chunk::Chunks &chunks)
    {
        assert(this->threadCount_ >= 2);

        // Zip up the boundary
        // Each thread applied the suppression window up to the boundary.
        // We need to apply the window to the seam, extending a full window
        // height or width into the neighboring region.
        Index zipSize = 2 * (this->windowSize_ - 1);
        auto chunk = std::begin(chunks);

        // We will look backwards starting at the second chunk.
        while (++chunk != std::end(chunks))
        {
            Index zipBegin = chunk->index - this->windowSize_ + 1;

            auto block = result.block(
                0,
                zipBegin,
                this->rows_,
                zipSize);

            detail::Suppress(
                this->rows_ - this->windowSize_ + 1,
                zipSize - this->windowSize_ + 1,
                this->windowSize_,
                block);
        }
    }

    void ZipRowMajor_(Data &result, const chunk::Chunks &chunks)
    {
        assert(this->threadCount_ >= 2);

        // Zip up the boundary
        // Each thread applied the suppression window up to the boundary.
        // We need to apply the window to the seam, extending a full window
        // height or width into the neighboring region.
        Index zipSize = 2 * (this->windowSize_ - 1);
        auto chunk = std::begin(chunks);

        // We will look backwards starting at the second chunk.
        while (++chunk != std::end(chunks))
        {
            Index zipBegin = chunk->index - this->windowSize_ + 1;

            auto block = result.block(
                zipBegin,
                0,
                zipSize,
                this->columns_);

            detail::Suppress(
                zipSize - this->windowSize_ + 1,
                this->columns_ - this->windowSize_ + 1,
                this->windowSize_,
                block);
        }
    }


private:
    size_t threadCount_;
    Index windowSize_;
    Index rows_;
    Index columns_;
    std::vector<detail::SuppressionChunk<Data>> threads_;
};


template<typename Data>
Data Suppression(
    size_t threadCount,
    Eigen::Index windowSize,
    const Eigen::MatrixBase<Data> &data)
{
    return AsyncSuppression<Data>(threadCount, windowSize, data).Get();
}


} // end namespace iris
