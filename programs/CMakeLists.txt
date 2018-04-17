cmake_minimum_required(VERSION 3.0)

project(examples)

list(APPEND SPIRV_REFLECT_FILES
  ${CMAKE_SOURCE_DIR}/spirv_reflect.h
  ${CMAKE_SOURCE_DIR}/spirv_reflect.c
)

list(APPEND COMMON_FILES
  ${CMAKE_SOURCE_DIR}/common/arg_parser.h
  ${CMAKE_SOURCE_DIR}/common/arg_parser.cpp
  ${CMAKE_SOURCE_DIR}/common/to_string.h
  ${CMAKE_SOURCE_DIR}/common/to_string.cpp
)

################################################################################
# spirv-diff
################################################################################
add_executable(spirv-diff ${CMAKE_CURRENT_SOURCE_DIR}/main_spirv_diff.cpp ${COMMON_FILES} ${SPIRV_REFLECT_FILES})
target_include_directories(spirv-diff PRIVATE ${CMAKE_SOURCE_DIR})
if (${VULKAN_DIR_FOUND})
    target_compile_definitions(spirv-diff PRIVATE SPIRV_REFLECT_HAS_VULKAN_H)
    target_include_directories(spirv-diff PRIVATE ${VULKAN_DIR}/include)
endif()
if(WIN32)
    set_target_properties(spirv-diff PROPERTIES FOLDER "programs")
endif()


install(TARGETS spirv-diff RUNTIME DESTINATION bin)