cmake_minimum_required(VERSION 3.13)
project(simExtIGL)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MACOSX_RPATH 1)

find_package(Boost REQUIRED COMPONENTS system)

if(NOT LIBPLUGIN_DIR)
    if(DEFINED ENV{COPPELIASIM_ROOT_DIR})
        set(LIBPLUGIN_DIR $ENV{COPPELIASIM_ROOT_DIR}/programming/libPlugin)
    else()
        set(LIBPLUGIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../libPlugin)
    endif()
endif()
list(APPEND CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
    ${LIBPLUGIN_DIR}/cmake)
find_package(CoppeliaSim 4.1.0.0 REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GMP REQUIRED IMPORTED_TARGET gmp)
pkg_check_modules(GMPXX REQUIRED IMPORTED_TARGET gmpxx)
pkg_check_modules(MPFR REQUIRED IMPORTED_TARGET mpfr)

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()

include(ExternalProject)
ExternalProject_Add(libigl-build
    DOWNLOAD_COMMAND ""
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/libigl
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/libigl-build
    CMAKE_ARGS -DBUILD_TESTING=OFF -DLIBIGL_WITH_CGAL=ON -DLIBIGL_WITH_CORK=OFF -DLIBIGL_WITH_OPENGL=OFF -DLIBIGL_WITH_OPENGL_GLFW=OFF -DLIBIGL_WITH_OPENGL_GLFW_IMGUI=OFF
    INSTALL_COMMAND ""
)

include_directories(${CMAKE_CURRENT_BINARY_DIR}/generated)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/libigl/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/libigl/external/eigen)
link_directories(${CMAKE_CURRENT_BINARY_DIR}/libigl-build)

coppeliasim_generate_stubs(${CMAKE_CURRENT_BINARY_DIR}/generated XML_FILE ${CMAKE_CURRENT_SOURCE_DIR}/callbacks.xml LUA_FILE ${CMAKE_CURRENT_SOURCE_DIR}/simExtIGL.lua)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

coppeliasim_add_plugin(simExtIGL SOURCES plugin.cpp)
add_dependencies(simExtIGL libigl-build)
target_link_libraries(simExtIGL Boost::system)
target_link_libraries(simExtIGL Eigen3::Eigen)
target_link_libraries(simExtIGL PkgConfig::GMP PkgConfig::GMPXX PkgConfig::MPFR)
target_link_libraries(simExtIGL -ligl -ligl_cgal)
