#include <catch2/catch.hpp>

#include <iris/suppression.h>


TEST_CASE("Column-major threaded suppression", "[suppression]")
{
    using Matrix = Eigen::MatrixX<double>;

    Matrix m = Matrix::Zero(10, 10);
    m(4, 4) = 2.0;
    m(5, 5) = 3.0;
    m(0, 4) = 42.9;
    m(0, 5) = 42.8;

    Matrix result = iris::Suppression(2, 3, m);

    REQUIRE(result(4, 4) == 0);
    REQUIRE(result(5, 5) == 3.0);
    REQUIRE(result(0, 5) == 0.0);
    REQUIRE(result(0, 4) == 42.9);

    m(3, 3) = 4.0;
    m(0, 3) = 42.9;
    m(0, 5) = 42.8;
    m(5, 5) = 3.0;

    result = iris::Suppression(2, 3, m);

    REQUIRE(result(3, 3) == 4.0);
    REQUIRE(result(5, 5) == 0.0);
    REQUIRE(result(0, 5) == 0.0);
    REQUIRE(result(0, 3) == 42.9);
}


TEST_CASE("Row-major threaded suppression", "[suppression]")
{
    using Matrix =
        Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

    Matrix m = Matrix::Zero(10, 10);
    m(4, 4) = 2.0;
    m(5, 5) = 3.0;
    m(4, 0) = 42.9;
    m(5, 0) = 42.8;

    Matrix result = iris::Suppression(2, 3, m);

    REQUIRE(result(4, 4) == 0);
    REQUIRE(result(5, 5) == 3.0);
    REQUIRE(result(5, 0) == 0.0);
    REQUIRE(result(4, 0) == 42.9);

    m(3, 3) = 4.0;
    m(3, 0) = 42.9;
    m(5, 0) = 42.8;
    m(5, 5) = 3.0;

    result = iris::Suppression(2, 3, m);

    REQUIRE(result(3, 3) == 4.0);
    REQUIRE(result(5, 5) == 0.0);
    REQUIRE(result(5, 0) == 0.0);
    REQUIRE(result(3, 0) == 42.9);
}


TEST_CASE("Closely-spaced identical values", "[suppression]")
{
    using Matrix =
        Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

    Matrix m = Matrix::Zero(16, 16);
    m(7, 7) = 16;
    m(8, 8) = 16;

    Matrix result = iris::Suppression(2, 8, m);

    // Exactly one of them better be 0.
    REQUIRE(((result(7, 7) > 0) != (result(8, 8) > 0)));
}

