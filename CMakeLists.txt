cmake_minimum_required(VERSION 3.16.3)
project(simIGL)

find_package(Boost REQUIRED COMPONENTS system)

list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(LIBIGL_INSTALL OFF)
include(libigl)
igl_include(copyleft cgal)
igl_include(copyleft tetgen)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MACOSX_RPATH 1)

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

include_directories(${CMAKE_CURRENT_BINARY_DIR}/generated)

coppeliasim_generate_stubs(${CMAKE_CURRENT_BINARY_DIR}/generated XML_FILE ${CMAKE_CURRENT_SOURCE_DIR}/sourceCode/callbacks.xml LUA_FILE ${CMAKE_CURRENT_SOURCE_DIR}/lua/simIGL.lua)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sourceCode/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

coppeliasim_add_plugin(simIGL SOURCES sourceCode/plugin.cpp)
target_link_libraries(simIGL PRIVATE igl_copyleft::cgal igl_copyleft::tetgen)
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
