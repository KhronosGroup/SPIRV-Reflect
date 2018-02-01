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

################################################################################
# descriptors
################################################################################
add_executable(descriptors ${CMAKE_CURRENT_SOURCE_DIR}/main_descriptors.cpp ${COMMON_FILES} ${SPIRV_REFLECT_FILES})
target_include_directories(descriptors PRIVATE ${CMAKE_SOURCE_DIR})

if(WIN32)
    set_target_properties(descriptors PROPERTIES FOLDER "examples")
endif()

################################################################################
# io_variables
################################################################################
add_executable(io_variables ${CMAKE_CURRENT_SOURCE_DIR}/main_io_variables.cpp ${COMMON_FILES} ${SPIRV_REFLECT_FILES})
target_include_directories(io_variables PRIVATE ${CMAKE_SOURCE_DIR})

if(WIN32)
    set_target_properties(io_variables PROPERTIES FOLDER "examples")
endif()

################################################################################
# ubo_offsets
################################################################################
add_executable(ubo_offsets ${CMAKE_CURRENT_SOURCE_DIR}/main_ubo_offsets.cpp ${COMMON_FILES} ${SPIRV_REFLECT_FILES})
target_include_directories(ubo_offsets PRIVATE ${CMAKE_SOURCE_DIR})

if(WIN32)
    set_target_properties(ubo_offsets PROPERTIES FOLDER "examples")
endif()