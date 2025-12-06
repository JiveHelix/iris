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
    m(6, 8) = 18;
    m(7, 8) = 18.1;
    m(8, 8) = 18.2;
    m(9, 8) = 18.3;

    m(9, 0) = 3.14;
    m(9, 1) = 3.15;
    m(9, 2) = 3.16;
    m(9, 3) = 3.17;

    Matrix result;

    iris::Suppression(2, 3, m, result);

    REQUIRE(result(4, 4) == 0);
    REQUIRE(result(5, 5) == 3.0);
    REQUIRE(result(0, 5) == 0.0);
    REQUIRE(result(0, 4) == 42.9);
    REQUIRE(result(6, 8) == 0.0);
    REQUIRE(result(7, 8) == 0.0);
    REQUIRE(result(8, 8) == 0.0);
    REQUIRE(result(9, 8) == 18.3);


    REQUIRE(result(9, 0) == 0.0);
    REQUIRE(result(9, 1) == 0.0);
    REQUIRE(result(9, 2) == 0.0);
    REQUIRE(result(9, 3) == 3.17);

    m(3, 3) = 4.0;
    m(0, 3) = 42.9;
    m(0, 5) = 42.8;
    m(5, 5) = 3.0;

    iris::Suppression(2, 3, m, result);

    REQUIRE(result(3, 3) == 4.0);
    REQUIRE(result(5, 5) == 0.0);
    REQUIRE(result(0, 5) == 0.0);
    REQUIRE(result(0, 3) == 42.9);
}


TEST_CASE("Column-major single-threaded suppression", "[suppression]")
{
    using Matrix = Eigen::MatrixX<double>;

    Matrix m = Matrix::Zero(10, 10);
    m(4, 4) = 2.0;
    m(5, 5) = 3.0;
    m(0, 4) = 42.9;
    m(0, 5) = 42.8;

    m(6, 8) = 18;
    m(7, 8) = 18;
    m(8, 8) = 18;
    m(9, 8) = 18;

    Matrix result;
    iris::Suppression(1, 3, m, result);

    REQUIRE(result(4, 4) == 0);
    REQUIRE(result(5, 5) == 3.0);
    REQUIRE(result(0, 5) == 0.0);
    REQUIRE(result(0, 4) == 42.9);
    REQUIRE(result(6, 8) == 18.0);
    REQUIRE(result(7, 8) == 0.0);
    REQUIRE(result(8, 8) == 0.0);
    REQUIRE(result(9, 8) == 18.0);

    m(3, 3) = 4.0;
    m(0, 3) = 42.9;
    m(0, 5) = 42.8;
    m(5, 5) = 3.0;

    iris::Suppression(2, 3, m, result);

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

    Matrix result;
    iris::Suppression(2, 3, m, result);

    REQUIRE(result(4, 4) == 0);
    REQUIRE(result(5, 5) == 3.0);
    REQUIRE(result(5, 0) == 0.0);
    REQUIRE(result(4, 0) == 42.9);

    m(3, 3) = 4.0;
    m(3, 0) = 42.9;
    m(5, 0) = 42.8;
    m(5, 5) = 3.0;

    iris::Suppression(2, 3, m, result);

    REQUIRE(result(3, 3) == 4.0);
    REQUIRE(result(5, 5) == 0.0);
    REQUIRE(result(5, 0) == 0.0);
    REQUIRE(result(3, 0) == 42.9);
}


TEST_CASE("Row-major single-threaded suppression", "[suppression]")
{
    using Matrix =
        Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

    Matrix m = Matrix::Zero(10, 10);
    m(4, 4) = 2.0;
    m(5, 5) = 3.0;
    m(4, 0) = 42.9;
    m(5, 0) = 42.8;

    m(6, 8) = 18;
    m(7, 8) = 18.1;
    m(8, 8) = 18.2;
    m(9, 8) = 18.3;

    m(9, 0) = 3.14;
    m(9, 1) = 3.15;
    m(9, 2) = 3.16;
    m(9, 3) = 3.17;

    Matrix result;
    iris::Suppression(1, 3, m, result);

    REQUIRE(result(4, 4) == 0);
    REQUIRE(result(5, 5) == 3.0);
    REQUIRE(result(5, 0) == 0.0);
    REQUIRE(result(4, 0) == 42.9);

    REQUIRE(result(6, 8) == 0.0);
    REQUIRE(result(7, 8) == 0.0);
    REQUIRE(result(8, 8) == 0.0);
    REQUIRE(result(9, 8) == 18.3);

    REQUIRE(result(9, 0) == 0.0);
    REQUIRE(result(9, 1) == 0.0);
    REQUIRE(result(9, 2) == 0.0);
    REQUIRE(result(9, 3) == 3.17);

    m(3, 3) = 4.0;
    m(3, 0) = 42.9;
    m(5, 0) = 42.8;
    m(5, 5) = 3.0;

    iris::Suppression(2, 3, m, result);

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

    Matrix result;
    iris::Suppression(2, 8, m, result);

    // Exactly one of them better be 0.
    REQUIRE(((result(7, 7) > 0) != (result(8, 8) > 0)));
}


TEST_CASE("Row-major increasing values within window", "[suppression]")
{
    using Matrix = Eigen::MatrixX<double>;

    Matrix m = Matrix::Zero(10, 10);
    m(6, 8) = 18;
    m(7, 8) = 18.1;
    m(8, 8) = 18.2;
    m(9, 8) = 18.3;

    m(9, 0) = 3.14;
    m(9, 1) = 3.15;
    m(9, 2) = 3.16;
    m(9, 3) = 3.17;

    Matrix result;
    iris::Suppression(1, 5, m, result);


    REQUIRE(result(6, 8) == 0.0);
    REQUIRE(result(7, 8) == 0.0);
    REQUIRE(result(8, 8) == 0.0);
    REQUIRE(result(9, 8) == 18.3);


    REQUIRE(result(9, 0) == 0.0);
    REQUIRE(result(9, 1) == 0.0);
    REQUIRE(result(9, 2) == 0.0);
    REQUIRE(result(9, 3) == 3.17);
}


TEST_CASE("Row-major decreasing values within window", "[suppression]")
{
    using Matrix = Eigen::MatrixX<double>;

    Matrix m = Matrix::Zero(10, 10);
    m(6, 8) = 18.3;
    m(7, 8) = 18.2;
    m(8, 8) = 18.1;
    m(9, 8) = 18.0;

    m(9, 0) = 3.17;
    m(9, 1) = 3.16;
    m(9, 2) = 3.15;
    m(9, 3) = 3.14;

    Matrix result;
    iris::Suppression(1, 5, m, result);

    REQUIRE(result(6, 8) == 18.3);
    REQUIRE(result(7, 8) == 0.0);
    REQUIRE(result(8, 8) == 0.0);
    REQUIRE(result(9, 8) == 0.0);


    REQUIRE(result(9, 0) == 3.17);
    REQUIRE(result(9, 1) == 0.0);
    REQUIRE(result(9, 2) == 0.0);
    REQUIRE(result(9, 3) == 0.0);
}
