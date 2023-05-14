#include <catch2/catch.hpp>

#include <iris/harris.h>
#include <iris/suppression.h>
#include <iris/gradient.h>


TEST_CASE("Create Harris corner detection class", "[harris]")
{
    using Matrix = Eigen::MatrixX<float>;

    Matrix m{
        { 0,  0,  0,  0,  0, 10, 10, 10, 10, 10},
        { 0,  0,  0,  0,  0, 10, 10, 10, 10, 10},
        { 0,  0,  0,  0,  0, 10, 10, 10, 10, 10},
        { 0,  0,  0,  0,  0, 10, 10, 10, 10, 10},
        { 0,  0,  0,  0,  0, 10, 10, 10, 10, 10},
        {10, 10, 10, 10, 10,  0,  0,  0,  0,  0},
        {10, 10, 10, 10, 10,  0,  0,  0,  0,  0},
        {10, 10, 10, 10, 10,  0,  0,  0,  0,  0},
        {10, 10, 10, 10, 10,  0,  0,  0,  0,  0},
        {10, 10, 10, 10, 10,  0,  0,  0,  0,  0}};

    m.array() += 10;

    auto differentiate =
        iris::Differentiate<float>(30, 1, iris::DerivativeSize::Size::three);

    auto gradient = iris::Gradient<Matrix>(differentiate);
    auto settings = iris::HarrisSettings<float>::Default();
    settings.threads = 1;
    auto harris = iris::Harris<float>(settings);

    auto response = harris.Filter(*gradient.Filter(m));
    REQUIRE(!!response);
    REQUIRE(response->cols() == m.cols());
    REQUIRE(response->rows() == m.rows());
}


TEST_CASE("Use suppression filter with count = 1", "[harris]")
{
    using Matrix = Eigen::MatrixX<float>;

    Matrix m{
        { 1,  0,  0,  0,  0},
        { 0,  2,  0,  0,  0},
        { 0,  0,  3,  0,  0},
        { 0,  0,  0,  4,  0},
        { 0,  0,  0,  0,  5}};

    Matrix filtered = iris::Suppression(1, 3, m);
}
