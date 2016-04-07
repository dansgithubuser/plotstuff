cmake_minimum_required(VERSION 2.8.12.2 FATAL_ERROR)
project(plotstuff)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "static")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../deps/sfml" "${CMAKE_CURRENT_BINARY_DIR}/sfml")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
	string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR})
endforeach()

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../deps/sfml/include")

add_definitions(-DSFML_STATIC)

set(output plotstuff)

set(src "${CMAKE_CURRENT_SOURCE_DIR}/../src/main.cpp")

add_executable(${output} ${src})

target_link_libraries(${output} sfml-system sfml-window sfml-graphics)

set_property(TARGET ${output} PROPERTY CXX_STANDARD 11)
