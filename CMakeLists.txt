# Minimum CMake version required
cmake_minimum_required(VERSION 3.18...3.27)

# Project name
project(fastgdal2tiles)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
    message(STATUS "Build type not specified, using Release by default")
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
else()
    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-reorder")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -pg -DLLVM_ENABLE_ASSERTIONS=ON")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
endif()


set(Python_FIND_VIRTUALENV ONLY)
find_package(Python 3.10 COMPONENTS Interpreter Development REQUIRED)
message(STATUS "python3: ${Python_EXECUTABLE}")

execute_process(
        COMMAND "${Python_EXECUTABLE}" -m nanobind --cmake_dir
        OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE NB_DIR)
message(STATUS "nanobind cmake dir: ${NB_DIR}")
list(APPEND CMAKE_PREFIX_PATH "${NB_DIR}")

find_package(nanobind CONFIG REQUIRED)
find_package(GDAL REQUIRED)
#find_package(unofficial-sqlite3 CONFIG REQUIRED)

message(STATUS "gdal include dirs: ${GDAL_INCLUDE_DIRS} ")
include_directories(${GDAL_INCLUDE_DIRS})

include_directories(SYSTEM src/third_party)
include_directories(src/include)

# Set installation directory
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/src)
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)
#
## Add subdirectories
#add_subdirectory(src/)
add_subdirectory(sandbox/)
add_executable(fastgdal2tiles src/fastgdal2tiles.cpp)
