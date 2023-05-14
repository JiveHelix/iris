cmake_minimum_required(VERSION 3.15)

project(iris CXX)

include(${CMAKE_CURRENT_LIST_DIR}/cmake_includes/setup_project.cmake)
setup_project()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftemplate-backtrace-limit=0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmessage-length=120")

add_subdirectory(iris)


if (${PROJECT_IS_TOP_LEVEL})
    option(
        BUILD_EXAMPLES
        "Build the example targets"
        ${ENABLES_DEFAULT})

    option(
        BUILD_TESTS
        "Build the tests"
        ${ENABLES_DEFAULT})
endif ()


if (${PROJECT_IS_TOP_LEVEL})

    if (${BUILD_EXAMPLES})
        add_subdirectory(examples)
    endif ()

    if (${BUILD_TESTS})
        add_subdirectory(test)
    endif ()

endif ()
