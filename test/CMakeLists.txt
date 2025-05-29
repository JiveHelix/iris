
add_catch2_test(
    NAME iris_tests
    SOURCES
        gradient_test.cpp
        harris_tests.cpp
        homography_tests.cpp
        suppression_tests.cpp
    LINK
        iris)
