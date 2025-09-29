#include <catch2/catch.hpp>
#include <jive/equal.h>

#include <tau/literals.h>
#include <tau/intrinsics.h>
#include <tau/pose.h>
#include <tau/projection.h>
#include <iris/homography.h>


using namespace tau::literals;


iris::NamedVertices CreateNamedVertices(
    double x_m,
    double squareSize_mm,
    const tau::Intrinsics<double> &intrinsics,
    const tau::Pose<double> &pose)
{
    tau::Projection projection(intrinsics, pose);

    iris::NamedVertices vertices;
    iris::NamedVertex current{};

    double squareSize_m = squareSize_mm * 1e-3;

    double startingY = 0.1;
    double startingZ = 0.075;

    for (size_t i = 0; i < 8; ++i)
    {
        current.logical.x = i;

        for (size_t j = 0; j < 6; ++j)
        {
            current.logical.y = j;

            tau::Vector3d<double> world(
                x_m,
                startingY - static_cast<double>(i) * squareSize_m,
                startingZ - static_cast<double>(j) * squareSize_m);

            tau::Vector3<double> sensor = projection.WorldToCamera(world);
            current.pixel.x = sensor(0);
            current.pixel.y = sensor(1);

            vertices.push_back(current);
        }
    }

    return vertices;
}


using Solutions = std::vector<iris::ChessSolution>;


class SolutionCreator
{
public:
    SolutionCreator(
        double squareSize_mm,
        const tau::Intrinsics<double> &intrinsics)
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
        // vertices will be placed.
        // Compute the translation of the camera such that the chess board
        // remains centered in the projected view.
        tau::Pose<double> pose(
            {x_deg, y_deg, z_deg},
            0_d,
            0_d,
            0_d);

        // Positive rotation about y makes the camera look down.
        // Raise the camera to keep the vertices in view.
        pose.point_m.z =
            virtualZ_m * std::tan(tau::ToRadians(pose.rotation.pitch_deg));

        // Positive rotation about z makes the camera look left.
        // Translate to the right (-y) to compensate.
        pose.point_m.y =
            -std::tan(tau::ToRadians(pose.rotation.yaw_deg)) * virtualZ_m;

        iris::ChessSolution solution;

        solution.vertices =
            CreateNamedVertices(
                virtualZ_m,
                this->squareSize_mm_,
                this->intrinsics_,
                pose);

        return solution;
    }

    double squareSize_mm_;
    tau::Intrinsics<double> intrinsics_;
};


Solutions CreateSolutions(
    double squareSize_mm,
    const tau::Intrinsics<double> &intrinsics)
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
    tau::Intrinsics<double> intrinsics{{
        10_d,
        25_d,
        25_d,
        1920.0_d / 2.0_d,
        1080.0_d / 2.0_d,
        0_d}};

    auto homographySettings = iris::HomographySettings{};

    auto solution =
        SolutionCreator(
            homographySettings.squareSize_mm,
            intrinsics).CreateSolution(0, 0, 0, 2);

    auto homography = iris::Homography(homographySettings);

    iris::HomographyMatrix homographyMatrix
        = homography.GetHomographyMatrix(solution.vertices);

    auto normalize = tau::NormalizePixel(homographySettings.sensorSize_pixels);
    auto world = iris::World(homographySettings.squareSize_mm);

    for (auto &vertex: solution.vertices)
    {
        auto worldPoint = world(vertex.logical);
        auto pixel = normalize(vertex.pixel);

        tau::Vector3<double> pixelH(pixel.x, pixel.y, 1);
        tau::Vector3<double> worldH(worldPoint.x, worldPoint.y, 1);
        tau::Vector3<double> projected = homographyMatrix * worldH;
        projected.array() /= projected(2);

        if (!projected.isApprox(pixelH))
        {
            std::cout << "projected:\n" << projected << "\n!=\n" << pixelH << std::endl;
        }

        REQUIRE(projected.isApprox(pixelH));
    }
}


TEST_CASE("Solve for intrinsics", "[homography]")
{
    tau::Intrinsics<double> intrinsics{{
        10_d,
        25_d,
        25_d,
        1920.0_d / 2.0_d,
        1080.0_d / 2.0_d,
        0_d}};

    auto homographySettings = iris::HomographySettings{};

    auto solutions =
        CreateSolutions(homographySettings.squareSize_mm, intrinsics);

    auto homography = iris::Homography(homographySettings);

    iris::Homography::Intrinsics result =
        homography.ComputeIntrinsics(solutions);

    std::cout << "Invented:\n" << intrinsics << std::endl;
    std::cout << "\nComputed:\n" << result << std::endl;

    std::cout << tau::Intrinsics<double>::FromArray_pixels(10_d, result)
        << std::endl;
}
