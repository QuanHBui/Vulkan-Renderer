cmake_minimum_required(VERSION 3.10)
include("${CMAKE_SOURCE_DIR}/utils.cmake")

project(VulkanRenderer)

set(CMAKE_CXX_STANDARD_REQUIRED 11)
set(CMAKE_CXX_STANDARD 11)

# For the header and resource files to show up in IDEs
file(GLOB_RECURSE SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE HEADERS "${PROJECT_SOURCE_DIR}/include/*.h")

add_executable(${CMAKE_PROJECT_NAME} ${SOURCES} ${HEADERS} ${GLSL})
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/include")

setBuildProperties(${CMAKE_PROJECT_NAME})

set(VULKAN_API_VERSION "VK_API_VERSION_1_0" CACHE STRING "Vulkan api version in the format of the Vulkan api version preprocessor constants i.e 'VK_API_VERSION_1_)'")
add_definitions("-DVULKAN_BASE_VK_API_VERSION=${VULKAN_API_VERSION}")