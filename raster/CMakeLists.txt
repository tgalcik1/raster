cmake_minimum_required(VERSION 3.12...3.25)
project(raster)
set(TARGET raster)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${TARGET})

# set C++ version to use
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set up config.h to find data directory
set(PROJECT_BASE_DIR "${PROJECT_SOURCE_DIR}")
set(PROJECT_DATA_DIR "${PROJECT_BASE_DIR}/data")
set(PROJECT_BUILD_DIR "${PROJECT_BINARY_DIR}")
configure_file(src/Config.hpp.in Config.hpp)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# I'm lazy here, and building with all cpp files in src
file(GLOB SOURCES  "src/*.cpp")
file(GLOB INCLUDES "src/*.hpp")
file(GLOB INLINES  "src/*.inl")
add_executable(${TARGET} ${SOURCES} ${INCLUDES} Config.hpp ${INLINES})

# add GLM
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(GLM REQUIRED)
include_directories(${GLM_INCLUDE_DIR})
