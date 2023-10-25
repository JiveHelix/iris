#pragma once


#include <future>
#include "iris/chunks.h"


namespace iris
{


namespace detail
{


template<typename Scalar>
struct LocalMaximumTemplate
{
    using Index = Eigen::Index;

    Index column;
    Index row;
    Scalar value;
};


template<typename Scalar>
std::ostream & operator<<(
    std::ostream &output,
    const LocalMaximumTemplate<Scalar> &localMaximum)
{
    return output << "LocalMaximum "
        << localMaximum.value
        << " at ("
        << localMaximum.row
        << ", "
        << localMaximum.column
        << ")";
}


template<typename Data>
auto GetInitialMaximum(
    Eigen::Index windowSize,
    Eigen::MatrixBase<Data> &result)
{
    using Eigen::Index;
    using LocalMaximum = LocalMaximumTemplate<typename Data::Scalar>;

    LocalMaximum maximum;

    auto initialBlock = result.block(0, 0, windowSize, windowSize);

    maximum.value =
        initialBlock.maxCoeff(&maximum.row, &maximum.column);

    initialBlock.array() = 0;
    initialBlock(maximum.row, maximum.column) = maximum.value;

    return maximum;
}


template<typename Data>
struct Suppressor
{
    using Index = Eigen::Index;
    using LocalMaximum = LocalMaximumTemplate<typename Data::Scalar>;

    Suppressor(Index windowSize, Eigen::MatrixBase<Data> &result_)
        :
        result(result_),
        localMaximum(GetInitialMaximum(windowSize, this->result))
    {

    }

    void UpdateLocalMaximum()
    {
        if (!this->localMaximum)
        {
            // There was no localMaximum
            // nextMaximum is it
            this->localMaximum = this->nextMaximum;

            // Restore the value of the new maximum
            this->result(this->nextMaximum.row, this->nextMaximum.column)
                = this->nextMaximum.value;

            return;
        }

        if (this->nextMaximum.value > this->localMaximum->value)
        {
            // Zero the previous maximum
            this->result(
                this->localMaximum->row,
                this->localMaximum->column) = 0;

            // Restore the value of the new maximum
            this->result(this->nextMaximum.row, this->nextMaximum.column) =
                this->nextMaximum.value;

            // Assign a new localMaximum
            this->localMaximum = this->nextMaximum;
        }
        else
        {
            // Zero the value of the nextMaximum that has now
            // been discovered to be non-maximum
            this->result(this->nextMaximum.row, this->nextMaximum.column) = 0;
        }
    }

    Eigen::MatrixBase<Data> &result;
    std::optional<LocalMaximum> localMaximum;
    LocalMaximum nextMaximum;
};


template<typename Data>
struct ColumnMajorSuppressor: public Suppressor<Data>
{
    using Suppressor<Data>::Suppressor;

    void ResetLocalMaximum(Eigen::Index index)
    {
        if (this->localMaximum && this->localMaximum->row < index)
        {
            // We have shifted beyond the range of the previous
            // localMaximum.
            this->localMaximum.reset();
        }
    }
};


template<typename Data>
struct RowMajorSuppressor: public Suppressor<Data>
{
    using Suppressor<Data>::Suppressor;

    void ResetLocalMaximum(Eigen::Index index)
    {
        if (this->localMaximum && this->localMaximum->column < index)
        {
            // We have shifted beyond the range of the previous
            // localMaximum.
            this->localMaximum.reset();
        }
    }
};


template<typename Data>
void SuppressColumnMajor(
    Eigen::Index limitRow,
    Eigen::Index limitColumn,
    Eigen::Index windowSize,
    Eigen::MatrixBase<Data> &result)
{
    using Eigen::Index;

    ColumnMajorSuppressor<Data> suppressor(windowSize, result);
    Index ignored;

    // First column
    for (Index row = 1; row < limitRow; ++row)
    {
        // Read the next row.
        auto nextRow = row + windowSize - 1;

        auto rowVector = result.block(
            nextRow,
            0,
            1,
            windowSize);

        suppressor.nextMaximum.row = nextRow;

        suppressor.nextMaximum.value =
            rowVector.maxCoeff(&ignored, &suppressor.nextMaximum.column);

        // Zero the row
        rowVector.array() = 0;

        suppressor.ResetLocalMaximum(row);
        suppressor.UpdateLocalMaximum();
    }

    // The other columns
    for (Index column = 1; column < limitColumn; ++column)
    {
        // Start with a block on the first row
        auto nextBlock = result.block(0, column, windowSize, windowSize);

        suppressor.nextMaximum.value =
            nextBlock.maxCoeff(
                &suppressor.nextMaximum.row,
                &suppressor.nextMaximum.column);

        // Adjust column index of maximum
        suppressor.nextMaximum.column += column;

        suppressor.localMaximum = suppressor.nextMaximum;
        nextBlock.rightCols(1).array() = 0;

        suppressor.result(
            suppressor.nextMaximum.row,
            suppressor.nextMaximum.column) = suppressor.nextMaximum.value;

        for (Index row = 1; row < limitRow; ++row)
        {
            // Read the next row.
            auto nextRow = row + windowSize - 1;
            auto rowVector = result.block(
                nextRow,
                column,
                1,
                windowSize);

            suppressor.nextMaximum.row = nextRow;

            suppressor.nextMaximum.value =
                rowVector.maxCoeff(&ignored, &suppressor.nextMaximum.column);

            suppressor.nextMaximum.column += column;

            rowVector.array() = 0;

            suppressor.ResetLocalMaximum(row);
            suppressor.UpdateLocalMaximum();
        }
    }
}


template<typename Data>
void SuppressRowMajor(
    Eigen::Index limitRow,
    Eigen::Index limitColumn,
    Eigen::Index windowSize,
    Eigen::MatrixBase<Data> &result)
{
    using Eigen::Index;

    RowMajorSuppressor<Data> suppressor(windowSize, result);
    Index ignored;

    // First row
    for (Eigen::Index column = 1; column < limitColumn; ++column)
    {
        // Read the next column.
        auto nextColumn = column + windowSize - 1;

        auto columnVector = result.block(
            0,
            nextColumn,
            windowSize,
            1);

        suppressor.nextMaximum.column = nextColumn;

        suppressor.nextMaximum.value =
            columnVector.maxCoeff(&suppressor.nextMaximum.row, &ignored);

        // Zero the column
        columnVector.array() = 0;

        suppressor.ResetLocalMaximum(column);
        suppressor.UpdateLocalMaximum();
    }

    // The other rows
    for (Eigen::Index row = 1; row < limitRow; ++row)
    {
        auto nextBlock = result.block(row, 0, windowSize, windowSize);

        suppressor.nextMaximum.value =
            nextBlock.maxCoeff(
                &suppressor.nextMaximum.row,
                &suppressor.nextMaximum.column);

        suppressor.nextMaximum.row += row;
        suppressor.localMaximum = suppressor.nextMaximum;
        nextBlock.bottomRows(1).array() = 0;

        suppressor.result(
            suppressor.nextMaximum.row,
            suppressor.nextMaximum.column) = suppressor.nextMaximum.value;

        for (Eigen::Index column = 1; column < limitColumn; ++column)
        {
            // Read the next column.
            auto nextColumn = column + windowSize - 1;

            auto columnVector = result.block(
                row,
                nextColumn,
                windowSize,
                1);

            suppressor.nextMaximum.column = nextColumn;

            suppressor.nextMaximum.value =
                columnVector.maxCoeff(&suppressor.nextMaximum.row, &ignored);

            suppressor.nextMaximum.row += row;

            columnVector.array() = 0;

            suppressor.ResetLocalMaximum(column);
            suppressor.UpdateLocalMaximum();
        }
    }
}


template<typename Data>
void Suppress(
    Eigen::Index limitRow,
    Eigen::Index limitColumn,
    Eigen::Index windowSize,
    Eigen::MatrixBase<Data> &result)
{
    if constexpr (tau::MatrixTraits<Data>::isColumnMajor)
    {
        SuppressColumnMajor(limitRow, limitColumn, windowSize, result);
    }
    else
    {
        SuppressRowMajor(limitRow, limitColumn, windowSize, result);
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
        // Add +1 because we are limiting the starting index of the windowed
        // operation.
        Index limitRow = (rows.count - windowSize) + 1;
        Index limitColumn = (columns.count - windowSize) + 1;

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
