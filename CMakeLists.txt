cmake_minimum_required(VERSION 3.16.3)
project(simIGL)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MACOSX_RPATH 1)

if(MSVC)
    # fix C1128: number of sections exceeded object file format limit
    add_compile_options(/bigobj)
endif()

find_package(Boost REQUIRED COMPONENTS system)

if(NOT COPPELIASIM_INCLUDE_DIR)
    if(DEFINED ENV{COPPELIASIM_ROOT_DIR})
        set(COPPELIASIM_INCLUDE_DIR $ENV{COPPELIASIM_ROOT_DIR}/programming/include)
    else()
        set(COPPELIASIM_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../include)
    endif()
endif()
list(APPEND CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
    ${COPPELIASIM_INCLUDE_DIR}/cmake)
find_package(CoppeliaSim 4.5.1.7 REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GMP REQUIRED IMPORTED_TARGET gmp)
pkg_check_modules(GMPXX REQUIRED IMPORTED_TARGET gmpxx)
pkg_check_modules(MPFR REQUIRED IMPORTED_TARGET mpfr)

include(ExternalProject)
ExternalProject_Add(libigl-build
    GIT_REPOSITORY https://github.com/libigl/libigl
    GIT_TAG b1fe6ba49c06075372ef90a70c785e96cd044772
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/libigl
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/libigl-build
    # macOS: error: no member named 'rayStreamFilterFuncs' in
    #               namespace 'embree::sse41'; did you mean
    #               simply 'rayStreamFilterFuncs'?
    # workaround: -DLIBIGL_EMBREE=OFF
    # https://github.com/libigl/libigl/issues/1302
    CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
        -DCMAKE_BUILD_TYPE=Release
        -DBUILD_TESTING=OFF
        -DLIBIGL_BUILD_TESTS=OFF
        -DLIBIGL_BUILD_TUTORIALS=OFF
        -DLIBIGL_COPYLEFT_CGAL=ON
        -DLIBIGL_COPYLEFT_TETGEN=ON
        -DLIBIGL_EMBREE=OFF
        -DLIBIGL_GLFW=OFF
        -DLIBIGL_IMGUI=OFF
        -DLIBIGL_OPENGL=OFF
        -DLIBIGL_RESTRICTED_MATLAB=OFF
        -DLIBIGL_RESTRICTED_MOSEK=OFF
        -DLIBIGL_RESTRICTED_TRIANGLE=OFF
    INSTALL_COMMAND ""
)

include_directories(${CMAKE_CURRENT_BINARY_DIR}/generated)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/libigl/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/libigl-build/_deps/tetgen-src)
link_directories(${CMAKE_CURRENT_BINARY_DIR}/libigl-build/lib)

coppeliasim_generate_stubs(${CMAKE_CURRENT_BINARY_DIR}/generated XML_FILE ${CMAKE_CURRENT_SOURCE_DIR}/sourceCode/callbacks.xml LUA_FILE ${CMAKE_CURRENT_SOURCE_DIR}/lua/simIGL.lua)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sourceCode/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

coppeliasim_add_plugin(simIGL SOURCES sourceCode/plugin.cpp)
add_dependencies(simIGL libigl-build)
target_link_libraries(simIGL PRIVATE Boost::system)
target_link_libraries(simIGL PRIVATE Eigen3::Eigen)
target_link_libraries(simIGL PRIVATE PkgConfig::GMP PkgConfig::GMPXX PkgConfig::MPFR)
target_link_libraries(simIGL PRIVATE igl igl_copyleft_cgal igl_copyleft_tetgen tetgen)
coppeliasim_add_lua("lua/addOns/booleanMesh.lua" SUBDIR addOns)
coppeliasim_add_addon("addOns/Create dummies on mesh surface.lua")
coppeliasim_add_addon("addOns/Find midpoint.lua")
coppeliasim_add_addon("addOns/Measure distance and direction.lua")
coppeliasim_add_addon("addOns/Mesh difference.lua")
coppeliasim_add_addon("addOns/Mesh intersection.lua")
coppeliasim_add_addon("addOns/Mesh subdivision.lua")
coppeliasim_add_addon("addOns/Mesh symmetric difference.lua")
coppeliasim_add_addon("addOns/Mesh union.lua")
coppeliasim_add_addon("addOns/Point sampler interactive.lua")
coppeliasim_add_addon("addOns/Point sampler service.lua")
coppeliasim_add_addon("addOns/Subdivide large triangles.lua")
coppeliasim_add_addon("addOns/Translate to align vertex.lua")
