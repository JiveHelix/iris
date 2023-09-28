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


add_executable(
    lines_chain_demo
    lines_chain_demo/lines_chain_demo.cpp
    lines_chain_demo/filters.cpp
    lines_chain_demo/main_frame.cpp
    lines_chain_demo/demo_settings_view.cpp)

target_link_libraries(
    lines_chain_demo
    PRIVATE
    project_warnings
    project_options
    iris
    common)


add_executable(
    chess_demo
    chess_demo/chess_demo.cpp
    chess_demo/demo_brain.cpp
    chess_demo/demo_brain_process.cpp
    chess_demo/demo_brain_display_loop.cpp
    chess_demo/demo_brain_load_png.cpp
    chess_demo/demo_brain_ctor.cpp
    chess_demo/demo_brain_create_control_frame.cpp
    chess_demo/demo_brain_shutdown.cpp
    chess_demo/demo_brain_display.cpp
    chess_demo/filters.cpp
    chess_demo/main_frame.cpp
    chess_demo/demo_settings_view.cpp)

target_link_libraries(
    chess_demo
    PRIVATE
    project_warnings
    project_options
    iris
    common)


add_executable(
    vertex_chain_demo
    vertex_chain_demo/vertex_chain_demo.cpp
    vertex_chain_demo/filters.cpp
    vertex_chain_demo/main_frame.cpp
    vertex_chain_demo/demo_settings_view.cpp)

target_link_libraries(
    vertex_chain_demo
    PRIVATE
    project_warnings
    project_options
    iris
    common)


add_executable(mask_demo mask_demo.cpp)

target_link_libraries(
    mask_demo
    PRIVATE
    project_warnings
    project_options
    iris
    common)


add_executable(level_adjust_demo level_adjust_demo.cpp)

target_link_libraries(
    level_adjust_demo
    PRIVATE
    project_warnings
    project_options
    iris
    common)


add_executable(rotation rotation.cpp)

target_link_libraries(
    rotation
    PRIVATE
    project_warnings
    project_options
    CONAN_PKG::tau)
