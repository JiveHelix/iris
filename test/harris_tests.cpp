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
    settings.sigma = 1.0;

    // TODO: For small inputs, it is possible that the default windowSize of
    // 6 may be larger than the size of the input as it is in this case.
    // Add automatic validation to the settings model.
    settings.window = 4;

    auto harris = iris::Harris<float>(settings);

    iris::GradientResult<float> gradientResult(255, m.rows(), m.cols());

    REQUIRE(gradient.Filter(m, gradientResult));

    using HarrisResult = typename iris::Harris<float>::Result;
    using Data = typename HarrisResult::Data;

    HarrisResult harrisResult{};
    harrisResult.data = Data::Zero(m.rows(), m.cols());

    REQUIRE(harris.Filter(gradientResult, harrisResult, tau::Margins{}));
    REQUIRE(harrisResult.data.cols() == m.cols());
    REQUIRE(harrisResult.data.rows() == m.rows());
}


TEST_CASE("Use suppression filter with count = 1", "[harris]")
{
    using Matrix = tau::RowMajorMatrix<float>;

    Matrix m{
        { 1,  0,  0,  0,  0},
        { 0,  2,  0,  0,  0},
        { 0,  0,  3,  0,  0},
        { 0,  0,  0,  4,  0},
        { 0,  0,  0,  0,  5}};

    Matrix filtered(5, 5);
    iris::Suppression<float>(3, m, filtered);

    std::cout << "filtered: " << filtered << std::endl;
}
