cmake_minimum_required(VERSION 3.0)

project(spirv-reflect)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

set(VULKAN_DIR_FOUND FALSE)
if (NOT VULKAN_DIR)
  set(VULKAN_DIR $ENV{VULKAN_SDK})
endif()
if (NOT "${VULKAN_DIR}" STREQUAL "")
  message(STATUS "Using Vulkan found at ${VULKAN_DIR}")
  set(VULKAN_DIR_FOUND TRUE)
endif()

# Figure out where the spirv.h header is located. Prefer the unified1 dir in the SPIRV-Headers submodule,
# and allow parent proejcts to override the location if they wish, and fall back on the Vulkan SDK if all
# else fails.
if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/SPIRV-Headers)
  add_subdirectory(third_party/SPIRV-Headers)
endif()
if (DEFINED SPIRV-Headers_SOURCE_DIR)
  set(SPIRV_REFLECT_SPIRV_HEADERS_DIR ${SPIRV-Headers_SOURCE_DIR}/include/spirv/unified1)
else()
  message(STATUS "SPIRV-Headers directory not found; falling back on Vulkan SDK")
  if(NOT ${VULKAN_DIR_FOUND})
    message(FATAL_ERROR "VULKAN_DIR not set, use -DVULKAN_DIR=<path> or set VULKAN_SDK environment variable")
  endif()
  set(SPIRV_REFLECT_SPIRV_HEADERS_DIR ${VULKAN_DIR}/include/vulkan)
endif()
message(STATUS "Building with spirv.h from ${SPIRV_REFLECT_SPIRV_HEADERS_DIR}")
include_directories(${SPIRV_REFLECT_SPIRV_HEADERS_DIR})

add_executable(spirv-reflect ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/spirv_reflect.h
                             ${CMAKE_CURRENT_SOURCE_DIR}/spirv_reflect.c
                             ${CMAKE_CURRENT_SOURCE_DIR}/examples/arg_parser.h
                             ${CMAKE_CURRENT_SOURCE_DIR}/examples/arg_parser.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/examples/common.h
                             ${CMAKE_CURRENT_SOURCE_DIR}/examples/common.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/common/output_stream.h
                             ${CMAKE_CURRENT_SOURCE_DIR}/common/output_stream.cpp)
target_compile_options(spirv-reflect PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Werror>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Werror>
    $<$<CXX_COMPILER_ID:AppleClang>:-Wall -Werror>)
target_include_directories(spirv-reflect PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(examples)
add_subdirectory(util/stripper)

install(TARGETS spirv-reflect RUNTIME DESTINATION bin)
