#include <catch2/catch.hpp>

#include <iris/harris.h>
#include <iris/suppression.h>
#include <iris/gradient.h>


TEST_CASE("Create Harris vertex detection class", "[harris]")
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

    auto gradient = iris::Gradient<float>(differentiate);
    auto settings = iris::HarrisSettings<float>{};
    settings.threads = 1;
    settings.sigma = 1.0;
    auto harris = iris::Harris<float>(settings);

    iris::GradientResult<float> gradientResult(255, m.rows(), m.cols());

    REQUIRE(gradient.Filter(m, gradientResult));

    using HarrisResult = typename iris::Harris<float>::Result;

    HarrisResult harrisResult{};

    REQUIRE(harris.Filter(gradientResult, harrisResult));
    REQUIRE(harrisResult.cols() == m.cols());
    REQUIRE(harrisResult.rows() == m.rows());
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

    Matrix filtered;
    iris::Suppression(1, 3, m, filtered);

    std::cout << "filtered: " << filtered << std::endl;
}
