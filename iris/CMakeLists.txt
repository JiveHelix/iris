add_library(iris)

if (${fPIC})
    set_property(TARGET iris PROPERTY POSITION_INDEPENDENT_CODE ON)
endif ()

# Projects that include this project must #include "iris/<header-name>"
target_include_directories(iris PUBLIC ${PROJECT_SOURCE_DIR})

target_link_libraries(
    iris
    PUBLIC
    CONAN_PKG::jive
    CONAN_PKG::fields
    CONAN_PKG::tau
    CONAN_PKG::pex
    CONAN_PKG::libpng
    CONAN_PKG::wxpex
    CONAN_PKG::draw
    CONAN_PKG::fmt
    CONAN_PKG::nlohmann_json)

target_sources(
    iris
    PRIVATE
    canny.cpp
    canny_chain.cpp
    canny_chain_settings.cpp
    canny_settings.cpp
    chess/axis_groups.cpp
    chess/chess_solution.cpp
    chess/find_vertex.cpp
    chess/group_functions.cpp
    chess/named_vertex.cpp
    chess/line_from_points.cpp
    chess/line_group.cpp
    chess.cpp
    chess_chain.cpp
    chess_chain_results.cpp
    chess_chain_settings.cpp
    chess_settings.cpp
    color.cpp
    color_settings.cpp
    vertex.cpp
    vertex_chain.cpp
    vertex_chain_settings.cpp
    vertex_settings.cpp
    gaussian_node.cpp
    gaussian_settings.cpp
    gradient.cpp
    gradient_settings.cpp
    harris_settings.cpp
    homography.cpp
    homography_settings.cpp
    hough_settings.cpp
    level_adjust.cpp
    level_settings.cpp
    lines_chain.cpp
    lines_chain_settings.cpp
    mask.cpp
    mask_settings.cpp
    node.cpp
    views/canny_settings_view.cpp
    views/canny_chain_settings_view.cpp
    views/chess_chain_settings_view.cpp
    views/chess_settings_view.cpp
    views/chess_shape.cpp
    views/chess_shape_view.cpp
    views/vertex_chain_settings_view.cpp
    views/vertex_settings_view.cpp
    views/gaussian_settings_view.cpp
    views/gradient_settings_view.cpp
    views/homography_settings_view.cpp
    views/intrinsics_view.cpp
    views/level_settings_view.cpp
    views/lines_chain_settings_view.cpp
    views/mask_brain.cpp
    views/mask_settings_view.cpp
    views/pixel_info_view.cpp)

install(TARGETS iris DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/iris
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
