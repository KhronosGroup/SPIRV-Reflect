cmake_minimum_required(VERSION 3.0)

project(spirv-reflect)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

if (NOT VULKAN_DIR)
  set(VULKAN_DIR $ENV{VULKAN_SDK})
endif()

if ("${VULKAN_DIR}" STREQUAL "")
  message(FATAL_ERROR "VULKAN_DIR not set, use -DVULKAN_DIR=<path> or set VULKAN_SDK environment variable")
endif()

message("Using Vulkan found at ${VULKAN_DIR}")

include_directories(${VULKAN_DIR}/include)

add_executable(spirv-reflect ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/spirv_reflect.h
                             ${CMAKE_CURRENT_SOURCE_DIR}/spirv_reflect.c
                             ${CMAKE_CURRENT_SOURCE_DIR}/examples/arg_parser.h
                             ${CMAKE_CURRENT_SOURCE_DIR}/examples/arg_parser.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/examples/common.h
                             ${CMAKE_CURRENT_SOURCE_DIR}/examples/common.cpp)
target_compile_options(spirv-reflect PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Werror>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Werror>
    $<$<CXX_COMPILER_ID:AppleClang>:-Wall -Werror>)
target_include_directories(spirv-reflect PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(examples)
add_subdirectory(util/stripper)

install(TARGETS spirv-reflect RUNTIME DESTINATION bin)
