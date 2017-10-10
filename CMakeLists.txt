cmake_minimum_required(VERSION 3.0)

project(spirv-reflect)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

if (NOT VULKAN_DIR)
  set(VULKAN_DIR $ENV{VULKAN_SDK})
endif()

if ("${VULKAN_DIR}" STREQUAL "")
  message(FATAL_ERROR "VULKAN_DIR not set, use -DVULKAN_DIR=<path> or set VULKAN_SDK environment variable")
endif()

message("Using Vulkan found at ${VULKAN_DIR}")

include_directories(${VULKAN_DIR}/include/vulkan)

add_executable(spirv-reflect ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/spirv_reflect.h
                             ${CMAKE_CURRENT_SOURCE_DIR}/spirv_reflect.c)

if(WIN32)
    set_target_properties(spirv-reflect PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup /SUBSYSTEM:Windows /INCREMENTAL:NO")
endif()
