cmake_minimum_required(VERSION 3.0)

project(examples)

list(APPEND SPIRV_REFLECT_FILES
  ${CMAKE_SOURCE_DIR}/spirv_reflect.h
  ${CMAKE_SOURCE_DIR}/spirv_reflect.c
)

list(APPEND COMMON_FILES
  common.h
  common.cpp
  sample_spv.h
)

add_executable(descriptors ${CMAKE_CURRENT_SOURCE_DIR}/descriptors.cpp ${COMMON_FILES} ${SPIRV_REFLECT_FILES})
target_include_directories(descriptors PRIVATE ${CMAKE_SOURCE_DIR})

if(WIN32)
    set_target_properties(descriptors PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup /SUBSYSTEM:Windows /INCREMENTAL:NO")
    set_target_properties(descriptors PROPERTIES FOLDER "examples")
endif()
