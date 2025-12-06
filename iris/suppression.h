#pragma once


#include <fields/fields.h>
#include <pex/interface.h>
#include <pex/group.h>
#include <pex/selectors.h>
#include <pex/range.h>
#include <tau/eigen.h>

#include "iris/detail/suppression_detail.h"


namespace iris
{


template<typename Input, typename Output>
class AsyncSuppression
{
public:
    using Index = typename Eigen::Index;

    AsyncSuppression(
        size_t threadCount,
        Index windowSize,
        const Eigen::MatrixBase<Input> &input,
        Eigen::MatrixBase<Output> &output)
        :
        threadCount_(threadCount),
        windowSize_(windowSize),
        rows_(input.rows()),
        columns_(input.cols()),
        suppressionChunks_(),
        output_(output)
    {
        Index maximumThreadCount;

        if constexpr (tau::MatrixTraits<Output>::isColumnMajor)
        {
            maximumThreadCount = this->columns_ / windowSize;
        }
        else
        {
            maximumThreadCount = this->rows_ / windowSize;
        }

        // Ensure that output has the right size.
        // Leave the values uninitialized for now.
        this->output_ = Output(this->rows_, this->columns_);

        this->threadCount_ =
            std::min(static_cast<size_t>(maximumThreadCount), threadCount);

        this->chunks_ = this->MakeChunks_();

        if (this->chunks_.at(0).count < windowSize)
        {
            throw IrisError("Chunk width is smaller than window size.");
        }

        this->suppressionChunks_.reserve(this->chunks_.size());

        if constexpr (tau::MatrixTraits<Output>::isColumnMajor)
        {
            if (this->rows_ < windowSize)
            {
                throw IrisError("Chunk height is smaller than window size.");
            }

            for (auto &chunk: this->chunks_)
            {
                this->suppressionChunks_.emplace_back(
                    chunk,
                    chunk::Chunk{0, this->columns_},
                    windowSize,
                    input,
                    output);
            }
        }
        else
        {
            if (this->columns_ < windowSize)
            {
                throw IrisError("Chunk height is smaller than window size.");
            }

            for (auto &chunk: this->chunks_)
            {
                this->suppressionChunks_.emplace_back(
                    chunk::Chunk{0, this->rows_},
                    chunk,
                    windowSize,
                    input,
                    output);
            }
        }
    }

    void Wait()
    {
        for (auto &chunk: this->suppressionChunks_)
        {
            chunk.Wait();
        }

        if (this->threadCount_ > 1)
        {
            if constexpr (tau::MatrixTraits<Output>::isColumnMajor)
            {
                this->ZipColumnMajor_();
            }
            else
            {
                this->ZipRowMajor_();
            }
        }
    }

private:
    chunk::Chunks MakeChunks_() const
    {
        if constexpr (tau::MatrixTraits<Output>::isColumnMajor)
        {
            // Create chunks along the columns
            auto chunks =
                chunk::MakeChunks(
                    this->threadCount_,
                    this->rows_,
                    this->windowSize_);

            assert(chunks.back().count >= chunks.at(0).count);

            return chunks;
        }
        else
        {
            // Create chunks along the rows
            auto chunks =
                chunk::MakeChunks(
                    this->threadCount_,
                    this->columns_,
                    this->windowSize_);

            assert(chunks.back().count >= chunks.at(0).count);

            return chunks;
        }
    }

    void ZipColumnMajor_()
    {
        assert(this->threadCount_ >= 2);

        // Zip up the boundary
        // Each thread applied the suppression window up to the boundary.
        // We need to apply the window to the seam, extending a full window
        // height or width into the neighboring region.
        auto chunkPtr = std::begin(this->chunks_);

        auto threadPool = jive::GetThreadPool();
        std::vector<jive::Sentry> sentries;
        sentries.reserve(this->chunks_.size() - 1);

        // We will look backwards starting at the second chunkPtr.
        while (++chunkPtr != std::end(this->chunks_))
        {
            auto chunk = *chunkPtr;

            sentries.emplace_back(
                threadPool->AddJob(
                    [chunk, this]()
                    {
                        Index zipBegin = chunk.index - this->windowSize_ + 1;
                        Index zipSize = 2 * (this->windowSize_ - 1);

                        auto block = this->output_.block(
                            zipBegin,
                            0,
                            zipSize,
                            this->columns_);

                        detail::Suppress(
                            zipSize - this->windowSize_ + 1,
                            this->columns_ - this->windowSize_ + 1,
                            this->windowSize_,
                            tau::MakeView(block));
                    }));
        }

        for (auto &sentry: sentries)
        {
            sentry.Wait();
        }
    }

    void ZipRowMajor_()
    {
        assert(this->threadCount_ >= 2);

        // Zip up the boundary
        // Each thread applied the suppression window up to the boundary.
        // We need to apply the window to the seam, extending a full window
        // height or width into the neighboring region.
        auto chunkPtr = std::begin(this->chunks_);

        auto threadPool = jive::GetThreadPool();
        std::vector<jive::Sentry> sentries;
        sentries.reserve(this->chunks_.size() - 1);

        // We will look backwards starting at the second chunkPtr.
        while (++chunkPtr != std::end(this->chunks_))
        {
            auto chunk = *chunkPtr;

            sentries.emplace_back(
                threadPool->AddJob(
                    [chunk, this]()
                    {
                        Index zipBegin = chunk.index - this->windowSize_ + 1;
                        Index zipSize = 2 * (this->windowSize_ - 1);

                        auto block = this->output_.block(
                            0,
                            zipBegin,
                            this->rows_,
                            zipSize);

                        detail::Suppress(
                            this->rows_ - this->windowSize_ + 1,
                            zipSize - this->windowSize_ + 1,
                            this->windowSize_,
                            tau::MakeView(block));
                    }));
        }

        for (auto &sentry: sentries)
        {
            sentry.Wait();
        }
    }

private:
    size_t threadCount_;
    Index windowSize_;
    Index rows_;
    Index columns_;
    std::vector<detail::SuppressionChunk<Input, Output>> suppressionChunks_;
    chunk::Chunks chunks_;
    Eigen::MatrixBase<Output> &output_;
};


template<typename Input, typename Output>
void Suppression(
    size_t threadCount,
    Eigen::Index windowSize,
    const Eigen::MatrixBase<Input> &input,
    Eigen::MatrixBase<Output> &output)
{
    AsyncSuppression(threadCount, windowSize, input, output).Wait();
}


} // end namespace iris
