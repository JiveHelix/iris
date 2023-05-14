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
    CONAN_PKG::fmt
    CONAN_PKG::nlohmann_json)

target_sources(
    iris
    PRIVATE
    chess.cpp
    corner.cpp
    homography.cpp
    png.cpp
    waveform.cpp
    views/chess_settings_view.cpp
    views/corner_settings_view.cpp
    views/draw_points.cpp
    views/pixel_info_view.cpp
    views/pixel_view.cpp
    views/shape_view.cpp
    views/view_settings.cpp
    views/waveform_settings_view.cpp
    views/waveform_view.cpp)

install(TARGETS iris DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/iris
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
