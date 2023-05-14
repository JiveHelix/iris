add_subdirectory(common)


if (MSVC)
    set(windowed WIN32)
elseif (APPLE)
    set(windowed MACOSX_BUNDLE)
endif ()

# add_executable(gaussian_demo gaussian_demo.cpp)
# 
# target_link_libraries(
#     gaussian_demo
#     PRIVATE
#     project_warnings
#     project_options
#     iris
#     common)
# 
# 
# add_executable(gradient_demo gradient_demo.cpp)
# 
# target_link_libraries(
#     gradient_demo
#     PRIVATE
#     project_warnings
#     project_options
#     iris
#     common)
# 
# 
# add_executable(canny_demo canny_demo.cpp)
# 
# target_link_libraries(
#     canny_demo
#     PRIVATE
#     project_warnings
#     project_options
#     iris
#     common)


add_executable(hough_demo hough_demo.cpp)

target_link_libraries(
    hough_demo
    PRIVATE
    project_warnings
    project_options
    iris
    common)


add_executable(shapes_demo shapes_demo.cpp)

target_link_libraries(
    shapes_demo
    PRIVATE
    project_warnings
    project_options
    iris
    common)


# add_executable(level_adjust_demo level_adjust_demo.cpp)

# target_link_libraries(
#     level_adjust_demo
#     PRIVATE
#     project_warnings
#     project_options
#     iris
#     common)


add_executable(rotation rotation.cpp)

target_link_libraries(
    rotation
    PRIVATE
    project_warnings
    project_options
    iris
    common)
