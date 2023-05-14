#include "iris/homography.h"
#include "iris/error.h"
#include <jive/equal.h>


namespace iris
{


ConstrainedElements GetConstrainedElements(
    const HomographyMatrix &homography,
    Eigen::Index i,
    Eigen::Index j)
{
    ConstrainedElements result{};

    result(0) = homography(0, i) * homography(0, j);

    result(1) = homography(0, i) * homography(1, j)
        + homography(1, i) * homography(0, j);

    result(2) = homography(2, i) * homography(0, j)
        + homography(0, i) * homography(2, j);

    result(3) = homography(1, i) * homography(1, j);

    result(4) = homography(2, i) * homography(1, j)
        + homography(1, i) * homography(2, j);

    result(5) = homography(2, i) * homography(2, j);

    return result;
}


ConstrainedFactors GetConstrainedFactors(const HomographyMatrix &homography)
{
    ConstrainedFactors result;

    result.block<1, 6>(0, 0) = GetConstrainedElements(homography, 0, 1);

    result.block<1, 6>(1, 0) =
        GetConstrainedElements(homography, 0, 0)
        - GetConstrainedElements(homography, 1, 1);

    return result;
}


Homography::Homography(
    double chessSquareSize_mm,
    const tau::Size<double> &sensorSize_pixels)
    :
    world_(chessSquareSize_mm),
    normalize_(sensorSize_pixels)
{

}


Eigen::Matrix<double, 2, 9>
Homography::GetHomographyFactors(const ChessIntersection & intersection)
{
    auto world = this->world_(intersection.logical);
    auto sensor = this->normalize_(intersection.pixel);

    return tau::Matrix<2, 9, double>(
        // The factors dependent on sensor x coordinates
        -world.x,
        -world.y,
        -1,
        0,
        0,
        0,
        sensor.x * world.x,
        sensor.x * world.y,
        sensor.x,

        // The factors dependent on sensor y coordinates
        0,
        0,
        0,
        -world.x,
        -world.y,
        -1,
        sensor.y * world.x,
        sensor.y * world.y,
        sensor.y);
}


Homography::Factors Homography::CombineHomographyFactors(
    const std::vector<ChessIntersection> &intersections)
{
    using Index = Eigen::Index;
    auto intersectionCount = static_cast<Index>(intersections.size());
    Homography::Factors result(2 * intersectionCount, 9);

    for (auto i: jive::Range<Index>(0, intersectionCount))
    {
        const auto &intersection = intersections[static_cast<size_t>(i)];
        result.block<2, 9>(i * 2, 0) = this->GetHomographyFactors(intersection);
    }

    return result;
}


template<typename Derived>
Eigen::VectorX<typename Derived::Scalar>
SvdSolve(Eigen::MatrixBase<Derived> &factors)
{
    using Scalar = typename Derived::Scalar;
    using Result = Eigen::VectorX<Scalar>;

#ifdef __GNUG__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#endif

    using Svd = Eigen::JacobiSVD<Derived>;

    Svd svd;

    svd.compute(
        factors.derived(),
        Eigen::ColPivHouseholderQRPreconditioner
            | Eigen::ComputeFullU
            | Eigen::ComputeFullV);

#ifdef __GNUG__
#pragma GCC diagnostic pop
#endif

    Result nullSpace;

    Eigen::MatrixX<Scalar> values = svd.matrixV();

    Eigen::Index solutionIndex = values.cols() - 1;

    while (solutionIndex > 0)
    {
        nullSpace = svd.matrixV()(Eigen::all, solutionIndex);
        Scalar magnitude = nullSpace.transpose() * nullSpace;

        if (jive::Roughly(magnitude, 1e-6) == 1)
        {
            return nullSpace;
        }
        else
        {
            std::cout << "magnitude not 1: " << magnitude << std::endl;
            std::cout << "solutionIndex: " << solutionIndex << std::endl;
        }

        --solutionIndex;
    }

    throw IrisError("Unable to find solution with magnitude 1");
}



HomographyMatrix Homography::GetHomographyMatrix(
    const std::vector<ChessIntersection> &intersections)
{
    auto factors = this->CombineHomographyFactors(intersections);

    HomographyMatrix homographyMatrix =
        SvdSolve(factors).reshaped<Eigen::RowMajor>(3, 3);

    return homographyMatrix;
}


Homography::Intrinsics Homography::ComputeIntrinsics(
    const std::vector<ChessSolution> &chessSolutions)
{
    if (chessSolutions.size() < 3)
    {
        throw ChessError("Underdetermined solution");
    }

    using ConstrainedFactorGroup = Eigen::Matrix<double, Eigen::Dynamic, 6>;

    Eigen::Index solutionCount =
        static_cast<Eigen::Index>(chessSolutions.size());

    ConstrainedFactorGroup factors(2 * solutionCount, 6);

    for (auto i: jive::Range<Eigen::Index>(0, solutionCount))
    {
        const auto &solution = chessSolutions[static_cast<size_t>(i)];

        factors.block<2, 6>(2 * i, 0) = GetConstrainedFactors(
            this->GetHomographyMatrix(solution.intersections));
    }


    Eigen::Vector<double, 6> solution = SvdSolve(factors);

    using Beta = Eigen::Matrix<double, 3, 3>;

    Beta beta{};
    beta(0, 0) = solution(0);
    beta(1, 0) = solution(1);
    beta(2, 0) = solution(2);

    beta(0, 1) = solution(1);
    beta(1, 1) = solution(3);
    beta(2, 1) = solution(4);

    beta(0, 2) = solution(2);
    beta(1, 2) = solution(4);
    beta(2, 2) = solution(5);

    using Cholesky = Eigen::LLT<Beta, Eigen::Upper>;
    Cholesky cholesky(beta);
    Eigen::Matrix<double, 3, 3> kInverseTranspose = cholesky.matrixL();

    Homography::Intrinsics intrinsics
        = kInverseTranspose.transpose().inverse();

    intrinsics.array() /= intrinsics(2, 2);

    const auto &n = this->normalize_;

    intrinsics(0, 0) = n.Unscale(intrinsics(0, 0), true);
    intrinsics(1, 1) = n.Unscale(intrinsics(1, 1), false);
    intrinsics(0, 2) = n.ToPixel(intrinsics(0, 2), true);
    intrinsics(1, 2) = n.ToPixel(intrinsics(1, 2), false);
    intrinsics(0, 1) = n.Unscale(intrinsics(0, 1), true);

    return intrinsics;
}


} // end namespace iris
