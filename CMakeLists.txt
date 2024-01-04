# Minimum CMake version required
cmake_minimum_required(VERSION 3.17...3.27)

# Project name
project(fastgdal2tiles)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set (Python_FIND_VIRTUALENV ONLY)
find_package(Python 3.10 COMPONENTS Interpreter Development
        REQUIRED)

execute_process(
        COMMAND "${Python_EXECUTABLE}" -m nanobind --cmake_dir
        OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE NB_DIR)
message(STATUS "python: ${Python_EXECUTABLE}")
#message(STATUS "nanobind cmake dir: ${NB_DIR}")
list(APPEND CMAKE_PREFIX_PATH "${NB_DIR}")

find_package(nanobind CONFIG REQUIRED)
find_package(GDAL REQUIRED)
message(STATUS "gdal include dirs: ${GDAL_INCLUDE_DIRS} ")
include_directories(${GDAL_INCLUDE_DIRS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/third_party)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/include)

# Set installation directory
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/src)
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)

# Add subdirectories
add_subdirectory(src/)
