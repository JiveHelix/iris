#pragma once


#include <tau/view.h>
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


template<tau::IsEigenRef InOut>
auto GetInitialMaximum(
    Eigen::Index windowSize,
    InOut inOut)
{
    using Eigen::Index;

    using LocalMaximum = LocalMaximumTemplate<tau::RefScalar<InOut>>;

    LocalMaximum maximum;

    auto initialBlock = inOut.block(0, 0, windowSize, windowSize);

    maximum.value =
        initialBlock.maxCoeff(&maximum.row, &maximum.column);

    initialBlock.array() = 0;
    initialBlock(maximum.row, maximum.column) = maximum.value;

    return maximum;
}


template<tau::IsEigenRef InOut>
struct Suppressor
{
    using Index = Eigen::Index;

    using LocalMaximum = LocalMaximumTemplate<tau::RefScalar<InOut>>;

    Suppressor(Index windowSize, InOut inOut_)
        :
        result(inOut_),
        localMaximum(GetInitialMaximum(windowSize, this->result))
    {

    }

    template<bool compareRow>
    void ResetLocalMaximum(Eigen::Index index)
    {
        if constexpr (compareRow)
        {
            if (this->localMaximum && this->localMaximum->row < index)
            {
                // We have shifted beyond the range of the previous
                // localMaximum.
                this->localMaximum.reset();
            }
        }
        else
        {
            if (this->localMaximum && this->localMaximum->column < index)
            {
                // We have shifted beyond the range of the previous
                // localMaximum.
                this->localMaximum.reset();
            }
        }
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

    InOut result;
    std::optional<LocalMaximum> localMaximum;
    LocalMaximum nextMaximum;
};


template<tau::IsEigenRef InOut>
void SuppressColumnMajor(
    Eigen::Index limitRow,
    Eigen::Index limitColumn,
    Eigen::Index windowSize,
    InOut inOut)
{
    // InOut is column-major.
    // As we slide the window across the data, the bulk of our reads should be
    // column-wise.
    using Eigen::Index;

    Suppressor<InOut> suppressor(windowSize, inOut);
    Index ignored;

    // First row
    for (Eigen::Index column = 1; column < limitColumn; ++column)
    {
        // Read the next column.
        auto nextColumn = column + windowSize - 1;

        auto columnVector = inOut.col(nextColumn).segment(0, windowSize);

        suppressor.nextMaximum.column = nextColumn;

        suppressor.nextMaximum.value =
            columnVector.maxCoeff(&suppressor.nextMaximum.row, &ignored);

        // Zero the column
        columnVector.array() = 0;

        suppressor.ResetLocalMaximum<false>(column);
        suppressor.UpdateLocalMaximum();
    }

    // The other rows
    for (Eigen::Index row = 1; row < limitRow; ++row)
    {
        auto nextBlock = inOut.block(row, 0, windowSize, windowSize);

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

            auto columnVector = inOut.col(nextColumn).segment(row, windowSize);

            suppressor.nextMaximum.column = nextColumn;

            suppressor.nextMaximum.value =
                columnVector.maxCoeff(&suppressor.nextMaximum.row, &ignored);

            suppressor.nextMaximum.row += row;

            columnVector.array() = 0;

            suppressor.ResetLocalMaximum<false>(column);
            suppressor.UpdateLocalMaximum();
        }
    }

}


template<tau::IsEigenRef InOut>
void SuppressRowMajor(
    Eigen::Index limitRow,
    Eigen::Index limitColumn,
    Eigen::Index windowSize,
    InOut inOut)
{
    using Eigen::Index;

    // InOut is row-major.
    // As we slide the window across the data, the bulk of our reads should be
    // row-wise.

    Suppressor<InOut> suppressor(windowSize, inOut);
    Index ignored;

    // Suppressor contains the maximum value for the first block of data.
    // First column
    for (Index row = 1; row < limitRow; ++row)
    {
        // Read the next row.
        auto nextRow = row + windowSize - 1;

        auto rowVector = inOut.row(nextRow).segment(0, windowSize);

        suppressor.nextMaximum.row = nextRow;

        suppressor.nextMaximum.value =
            rowVector.maxCoeff(&ignored, &suppressor.nextMaximum.column);

        // Zero the row
        rowVector.array() = 0;

        suppressor.ResetLocalMaximum<true>(row);
        suppressor.UpdateLocalMaximum();
    }

    // The other columns
    for (Index column = 1; column < limitColumn; ++column)
    {
        // Start with a block on the first row
        auto nextBlock = inOut.block(0, column, windowSize, windowSize);

        suppressor.nextMaximum.value =
            nextBlock.maxCoeff(
                &suppressor.nextMaximum.row,
                &suppressor.nextMaximum.column);

        // We found the column within nextBlock.
        // Adjust column index of maximum relative to the result origin.
        suppressor.nextMaximum.column += column;

        // Initialize the localMaximum for this column pass.
        suppressor.localMaximum = suppressor.nextMaximum;

        // Only set the right-most column in this block to zero.
        // The other columns have already been zeroed, leaving behind only
        // local maxima.
        nextBlock.rightCols(1).array() = 0;

        suppressor.result(
            suppressor.nextMaximum.row,
            suppressor.nextMaximum.column) = suppressor.nextMaximum.value;

        for (Index row = 1; row < limitRow; ++row)
        {
            // Read the next row.
            auto nextRow = row + windowSize - 1;

            auto rowVector = inOut.row(nextRow).segment(column, windowSize);

            suppressor.nextMaximum.row = nextRow;

            suppressor.nextMaximum.value =
                rowVector.maxCoeff(&ignored, &suppressor.nextMaximum.column);

            suppressor.nextMaximum.column += column;

            rowVector.array() = 0;

            suppressor.ResetLocalMaximum<true>(row);
            suppressor.UpdateLocalMaximum();
        }
    }
}


template<tau::IsEigenConstRef Input, tau::IsEigenRef Output>
void Suppress(
    Eigen::Index limitRow,
    Eigen::Index limitColumn,
    Eigen::Index windowSize,
    Input input,
    Output output)
{
    output = input;

    if constexpr (tau::RefTraits<Output>::isColumnMajor)
    {
        SuppressColumnMajor(limitRow, limitColumn, windowSize, output);
    }
    else
    {
        SuppressRowMajor(limitRow, limitColumn, windowSize, output);
    }
}


template<tau::IsEigenRef InOut>
void Suppress(
    Eigen::Index limitRow,
    Eigen::Index limitColumn,
    Eigen::Index windowSize,
    InOut inOut)
{
    if constexpr (tau::RefTraits<InOut>::isColumnMajor)
    {
        SuppressColumnMajor(limitRow, limitColumn, windowSize, inOut);
    }
    else
    {
        SuppressRowMajor(limitRow, limitColumn, windowSize, inOut);
    }
}


template<typename Input, typename Output>
class SuppressionChunk
{
public:
    SuppressionChunk(
        chunk::Chunk rows,
        chunk::Chunk columns,
        Eigen::Index windowSize,
        const Eigen::MatrixBase<Input> &input,
        Eigen::MatrixBase<Output> &output)
        :
        sentry_(
            jive::GetThreadPool()->AddJob(
                [&input, &output, rows, columns, windowSize]()
                {
                    using Eigen::Index;
                    // Limit the bounds of the operation so that the window
                    // never exceeds the bounds of input; Add +1 because we are
                    // limiting the starting index of the windowed operation.

                    Index limitRow = (rows.count - windowSize) + 1;
                    Index limitColumn = (columns.count - windowSize) + 1;

                    Suppress(
                        limitRow,
                        limitColumn,
                        windowSize,

                        tau::MakeView(
                            input.block(
                                rows.index,
                                columns.index,
                                rows.count,
                                columns.count)),

                        tau::MakeView(
                            output.block(
                                rows.index,
                                columns.index,
                                rows.count,
                                columns.count)));
                }))
    {

    }

    void Wait()
    {
        return this->sentry_.Wait();
    }

private:
    jive::Sentry sentry_;
};


} // end namespace detail


} // end namespace iris
