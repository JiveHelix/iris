add_library(iris_catch_main STATIC catch_main.cpp)
target_link_libraries(iris_catch_main PUBLIC CONAN_PKG::catch2)
target_link_libraries(iris_catch_main PRIVATE project_options)

add_executable(
    iris_tests
    extrinsics_tests.cpp
    gradient_test.cpp
    harris_tests.cpp
    homography_tests.cpp
    intrinsics_tests.cpp
    projection_tests.cpp
    rotation_tests.cpp
    suppression_tests.cpp)

target_link_libraries(
    iris_tests
    PRIVATE
    project_warnings
    project_options
    iris_catch_main
    iris)

add_test(NAME IrisTests COMMAND ${CMAKE_BINARY_DIR}/bin/iris_tests)
