cmake_minimum_required(VERSION 3.1)

project(spirv-reflect)

OPTION(SPIRV_REFLECT_BUILD_TESTS "Build the SPIRV-Reflect test suite" OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

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
set_target_properties(spirv-reflect PROPERTIES CXX_STANDARD 11)
target_include_directories(spirv-reflect PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(examples)
add_subdirectory(util/stripper)

install(TARGETS spirv-reflect RUNTIME DESTINATION bin)

if (SPIRV_REFLECT_BUILD_TESTS)
	message("Tests are enabled!")
  SET(BUILD_GMOCK OFF CACHE BOOL "Builds the googlemock subproject" FORCE)
  SET(BUILD_GTEST ON CACHE BOOL "Builds the googletest subproject" FORCE)
  SET(INSTALL_GTEST OFF CACHE BOOL "Enable installation of googletest. (Projects embedding googletest generally turn this OFF.)" FORCE)
  SET(gtest_force_shared_crt ON CACHE BOOL "Enable DLL for C runtime libraries on Windows" FORCE)

  add_subdirectory(third_party/googletest)
	add_executable(test-spirv-reflect ${CMAKE_CURRENT_SOURCE_DIR}/tests/test-spirv-reflect.cpp
                                    ${CMAKE_CURRENT_SOURCE_DIR}/spirv_reflect.h
                                    ${CMAKE_CURRENT_SOURCE_DIR}/spirv_reflect.c
                                    ${CMAKE_CURRENT_SOURCE_DIR}/common/output_stream.h
                                    ${CMAKE_CURRENT_SOURCE_DIR}/common/output_stream.cpp)
  set_target_properties(test-spirv-reflect PROPERTIES
                        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                        CXX_STANDARD 11)
  target_compile_definitions(test-spirv-reflect PRIVATE
                             $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
  target_link_libraries(test-spirv-reflect gtest_main)
  target_include_directories(test-spirv-reflect PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  add_custom_command(TARGET test-spirv-reflect POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy_directory
                     ${CMAKE_CURRENT_SOURCE_DIR}/tests ${CMAKE_CURRENT_BINARY_DIR}/tests)
endif()
