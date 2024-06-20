if(TARGET igl::core)
    return()
endif()

include(FetchContent)
FetchContent_Declare(
    libigl
    GIT_REPOSITORY https://github.com/libigl/libigl.git
    GIT_TAG v2.5.0
    #EXCLUDE_FROM_ALL # only available in CMake 3.28+
)
#FetchContent_MakeAvailable(libigl) # CMake 3.28+

# workaround for CMake < 3.28, to add EXCLUDE_FROM_ALL:
FetchContent_GetProperties(libigl)
if(NOT libigl_POPULATED)
  FetchContent_Populate(libigl)
  add_subdirectory(${libigl_SOURCE_DIR} ${libigl_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()
