#include <catch2/catch.hpp>
#include <jive/equal.h>

#include <tau/literals.h>
#include <iris/homography.h>
#include <iris/intrinsics.h>
#include <iris/pose.h>
#include <iris/projection.h>


static constexpr double tolerance = 1e-4;

using namespace tau::literals;


TEST_CASE("Normalized center", "[homography]")
{
    auto sensorSize = tau::Size<double>{1920, 1080};
    auto normalize = iris::Normalize(sensorSize);

    auto center = tau::Point2d<double>{1920.0 / 2, 1080 / 2};
    auto normalized = normalize.ToNormalized(center);
    auto expected = tau::Point2d<double>{0.0, 0.0};

    REQUIRE(jive::Roughly(expected.x, tolerance) == normalized.x);
    REQUIRE(jive::Roughly(expected.y, tolerance) == normalized.y);

    auto roundTrip = normalize.ToPixel(normalized);
    REQUIRE(jive::Roughly(roundTrip.x) == center.x);
    REQUIRE(jive::Roughly(roundTrip.y) == center.y);
}


TEST_CASE("Normalized topLeft", "[homography]")
{
    auto sensorSize = tau::Size<double>{1920, 1080};
    auto normalize = iris::Normalize(sensorSize);

    auto topLeft = tau::Point2d<double>{0, 0};
    auto normalized = normalize.ToNormalized(topLeft);
    auto expected = tau::Point2d<double>{-1.0, -1.0};

    REQUIRE(jive::Roughly(expected.x, tolerance) == normalized.x);
    REQUIRE(jive::Roughly(expected.y, tolerance) == normalized.y);

    auto roundTrip = normalize.ToPixel(normalized);
    REQUIRE(jive::Roughly(roundTrip.x) == topLeft.x);
    REQUIRE(jive::Roughly(roundTrip.y) == topLeft.y);
}


TEST_CASE("Normalized topRight", "[homography]")
{
    auto sensorSize = tau::Size<double>{1920, 1080};
    auto normalize = iris::Normalize(sensorSize);

    auto topRight = tau::Point2d<double>{1919, 0};
    auto normalized = normalize.ToNormalized(topRight);
    auto expected = tau::Point2d<double>{0.9990, -1.0};

    REQUIRE(jive::Roughly(expected.x, tolerance) == normalized.x);
    REQUIRE(jive::Roughly(expected.y, tolerance) == normalized.y);

    auto roundTrip = normalize.ToPixel(normalized);
    REQUIRE(jive::Roughly(roundTrip.x) == topRight.x);
    REQUIRE(jive::Roughly(roundTrip.y) == topRight.y);
}


TEST_CASE("Normalized bottomLeft", "[homography]")
{
    auto sensorSize = tau::Size<double>{1920, 1080};
    auto normalize = iris::Normalize(sensorSize);

    auto bottomLeft = tau::Point2d<double>{0, 1079};
    auto normalized = normalize.ToNormalized(bottomLeft);
    auto expected = tau::Point2d<double>{-1.0, 0.9981};

    REQUIRE(jive::Roughly(expected.x, tolerance) == normalized.x);
    REQUIRE(jive::Roughly(expected.y, tolerance) == normalized.y);

    auto roundTrip = normalize.ToPixel(normalized);
    REQUIRE(jive::Roughly(roundTrip.x) == bottomLeft.x);
    REQUIRE(jive::Roughly(roundTrip.y) == bottomLeft.y);
}

TEST_CASE("Normalized bottomRight", "[homography]")
{
    auto sensorSize = tau::Size<double>{1920, 1080};
    auto normalize = iris::Normalize(sensorSize);

    auto bottomRight = tau::Point2d<double>{1919, 1079};
    auto normalized = normalize.ToNormalized(bottomRight);
    auto expected = tau::Point2d<double>{0.9990, 0.9981};

    REQUIRE(jive::Roughly(expected.x, tolerance) == normalized.x);
    REQUIRE(jive::Roughly(expected.y, tolerance) == normalized.y);

    auto roundTrip = normalize.ToPixel(normalized);
    REQUIRE(jive::Roughly(roundTrip.x) == bottomRight.x);
    REQUIRE(jive::Roughly(roundTrip.y) == bottomRight.y);
}


using Intersections = std::vector<iris::ChessIntersection>;

Intersections CreateIntersections(
    double x_m,
    double squareSize_mm,
    const iris::Intrinsics<double> &intrinsics,
    const iris::Pose<double> &pose)
{
    iris::Projection projection(intrinsics, pose);

    Intersections intersections;
    iris::ChessIntersection current{};

    double squareSize_m = squareSize_mm * 1e-3;

    double startingY = 0.1;
    double startingZ = 0.075;

    for (size_t i = 0; i < 8; ++i)
    {
        current.logical.x = i;

        for (size_t j = 0; j < 6; ++j)
        {
            current.logical.y = j;

            iris::Vector3<double> world(
                x_m,
                startingY - static_cast<double>(i) * squareSize_m,
                startingZ - static_cast<double>(j) * squareSize_m);

            iris::Vector3<double> sensor = projection.WorldToCamera(world);
            current.pixel.x = sensor(0);
            current.pixel.y = sensor(1);

            intersections.push_back(current);
        }
    }

    return intersections;
}


using Solutions = std::vector<iris::ChessSolution>;


class SolutionCreator
{
public:
    SolutionCreator(
        double squareSize_mm,
        const iris::Intrinsics<double> &intrinsics)
        :
        squareSize_mm_(squareSize_mm),
        intrinsics_(intrinsics)
    {

    }

    iris::ChessSolution CreateSolution(
        double x_deg,
        double y_deg,
        double z_deg,
        double virtualZ_m)
    {
        // Create the pose of the camera in the world.
        // The camera is rotated relative to the space where the virtual
        // intersections will be placed.
        // Compute the translation of the camera such that the chess board
        // remains centered in the projected view.
        iris::Pose<double> pose({x_deg, y_deg, z_deg}, 0_d, 0_d, 0_d);

        // Positive rotation about y makes the camera look down.
        // Raise the camera to keep the intersections in view.
        pose.z_m = virtualZ_m * std::tan(tau::ToRadians(pose.rotation.pitch));

        // Positive rotation about z makes the camera look left.
        // Translate to the right (-y) to compensate.
        pose.y_m = -std::tan(tau::ToRadians(pose.rotation.yaw)) * virtualZ_m;

        iris::ChessSolution solution;

        solution.intersections =
            CreateIntersections(
                virtualZ_m,
                this->squareSize_mm_,
                this->intrinsics_,
                pose);

        return solution;
    }

    double squareSize_mm_;
    iris::Intrinsics<double> intrinsics_;
};


Solutions CreateSolutions(
    double squareSize_mm,
    const iris::Intrinsics<double> &intrinsics)
{
    Solutions solutions;

    SolutionCreator creator(squareSize_mm, intrinsics);

    solutions.push_back(
        creator.CreateSolution(0, 0, 0, 2));

    solutions.push_back(
        creator.CreateSolution(0, 0, 15, 1.9));

    solutions.push_back(
        creator.CreateSolution(0, 0, -17, 2.1));

    solutions.push_back(
        creator.CreateSolution(0, 0, -10, 2.05));

    solutions.push_back(
        creator.CreateSolution(0, 0, 11, 1.95));

    return solutions;
}


TEST_CASE("HomographyMatrix round trip", "[homography]")
{
    iris::Intrinsics<double> intrinsics{{
        10_d,
        25_d,
        25_d,
        1920.0_d / 2.0_d,
        1080.0_d / 2.0_d,
        0_d}};

    double squareSize_mm = 25;
    tau::Size<double> sensorSize_pixels{{1920, 1080}};

    auto solution =
        SolutionCreator(squareSize_mm, intrinsics).CreateSolution(0, 0, 0, 2);

    auto homography = iris::Homography(squareSize_mm, sensorSize_pixels);

    iris::HomographyMatrix homographyMatrix
        = homography.GetHomographyMatrix(solution.intersections);

    auto normalize = iris::Normalize(sensorSize_pixels);
    auto world = iris::World(squareSize_mm);

    for (auto &intersection: solution.intersections)
    {
        auto worldPoint = world(intersection.logical);
        auto pixel = normalize(intersection.pixel);

        iris::Vector3<double> pixelH(pixel.x, pixel.y, 1);
        iris::Vector3<double> worldH(worldPoint.x, worldPoint.y, 1);
        iris::Vector3<double> projected = homographyMatrix * worldH;
        projected.array() /= projected(2);

        if (!projected.isApprox(pixelH))
        {
            std::cout << "projected:\n" << projected << "\n!=\n" << pixelH << std::endl;
        }

        REQUIRE(projected.isApprox(pixelH));
    }
}


#if 0
template<int Options>
void RunSvdReconstructTest()
{
    iris::Intrinsics<double> intrinsics{{
        10_d,
        25_d,
        25_d,
        1920.0_d / 2.0_d,
        1080.0_d / 2.0_d,
        0_d}};

    double squareSize_mm = 25;
    tau::Size<double> sensorSize_pixels{{1920, 1080}};

    auto solution =
        SolutionCreator(squareSize_mm, intrinsics).CreateSolution(0, 0, 0, 2);

    auto homography = iris::Homography(squareSize_mm, sensorSize_pixels);
    auto factors = homography.CombineHomographyFactors(solution.intersections);

#ifdef __GNUG__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#endif

    using Svd = Eigen::JacobiSVD<iris::Homography::Factors>;

    Svd svd;

    svd.compute(
        factors,
        Options
            | Eigen::ComputeFullU
            | Eigen::ComputeFullV);

#ifdef __GNUG__
#pragma GCC diagnostic pop
#endif

    Eigen::VectorX<double> singularValues = svd.singularValues();
    Eigen::MatrixX<double> singular = singularValues.asDiagonal();
    // std::cout << "U:\n" << svd.matrixU() << std::endl;
    // std::cout << "S:\n" << singular << std::endl;
    // std::cout << "V:\n" << svd.matrixV() << std::endl;

    iris::Homography::Factors reconstruct =
        svd.matrixU() * singularValues.asDiagonal() * svd.matrixV().transpose();

    // std::cout << "factors:\n" << std::endl;
    // std::cout << factors << std::endl;

    // std::cout << "reconstruct:\n" << std::endl;
    // std::cout << reconstruct << std::endl;

    iris::Homography::Factors errors = reconstruct - factors;
    // std::cout << "errors:\n" << errors << std::endl;
    std::cout << "max error: " << errors.array().abs().maxCoeff() << std::endl;
}
#endif

#if 0
template<int Options>
void RunSvdNullspaceTest()
{
    iris::Intrinsics<double> intrinsics{{
        10_d,
        25_d,
        25_d,
        1920.0_d / 2.0_d,
        1080.0_d / 2.0_d,
        0_d}};

    double squareSize_mm = 25;
    tau::Size<double> sensorSize_pixels{{1920, 1080}};

    auto solution =
        SolutionCreator(squareSize_mm, intrinsics).CreateSolution(0, 0, 0, 2);

    auto homography = iris::Homography(squareSize_mm, sensorSize_pixels);
    auto factors = homography.CombineHomographyFactors(solution.intersections);

#ifdef __GNUG__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#endif

    using Svd = Eigen::JacobiSVD<iris::Homography::Factors>;

    Svd svd;

    svd.compute(
        factors,
        Options
            | Eigen::ComputeFullU
            | Eigen::ComputeFullV);

#ifdef __GNUG__
#pragma GCC diagnostic pop
#endif

    Eigen::VectorX<double> singularValues = svd.singularValues();

    for (auto i: jive::Range<Eigen::Index>(0, singularValues.size()))
    {
        std::cout << "sigma(" << i << "): " << singularValues(i) << std::endl;
        Eigen::VectorX<double> p = svd.matrixV()(Eigen::all, i);

        auto pHat =
            p.transpose() * factors.transpose() * factors * p;

        std::cout << "pHat: " << pHat << std::endl;
        auto mag = p.transpose() * p;
        std::cout << "mag: " << mag << std::endl;
    }
}


TEST_CASE("SVD null space", "[homography]")
{
    std::cout << "SVD ColPivHouseholderQRPreconditioner:\n";
    RunSvdNullspaceTest<Eigen::ColPivHouseholderQRPreconditioner>();
}

#endif


#if 0
TEST_CASE("SVD", "[homography]")
{
    std::cout << "SVD ColPivHouseholderQRPreconditioner:\n";
    RunSvdReconstructTest<Eigen::ColPivHouseholderQRPreconditioner>();

    std::cout << "SVD FullPivHouseholderQRPreconditioner:\n";
    RunSvdReconstructTest<Eigen::FullPivHouseholderQRPreconditioner>();

    std::cout << "SVD HouseholderQRPreconditioner:\n";
    RunSvdReconstructTest<Eigen::HouseholderQRPreconditioner>();

    std::cout << "SVD NoQRPreconditioner:\n";
    RunSvdReconstructTest<Eigen::NoQRPreconditioner>();
}
#endif


TEST_CASE("Solve for intrinsics", "[homography]")
{
    iris::Intrinsics<double> intrinsics{{
        10_d,
        25_d,
        25_d,
        1920.0_d / 2.0_d,
        1080.0_d / 2.0_d,
        5_d}};

    double squareSize_mm = 25;
    tau::Size<double> sensorSize_pixels{{1920, 1080}};

    auto solutions = CreateSolutions(squareSize_mm, intrinsics);
    auto homography = iris::Homography(squareSize_mm, sensorSize_pixels);

    iris::Homography::Intrinsics result =
        homography.ComputeIntrinsics(solutions);

    std::cout << "Invented:\n" << intrinsics << std::endl;
    std::cout << "\nComputed:\n" << result << std::endl;

    std::cout << iris::Intrinsics<double>::FromArray(10_d, result)
        << std::endl;
}
