cmake_minimum_required(VERSION 2.8.12.2 FATAL_ERROR)
project(plotstuff)

#=====constants=====#
set(output plotstuff)
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	set(sfmlDir "../deps/SFML-2.2")
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
	set(sfmlDir "../deps/SFML-2.2-windows-vc12-32-bit")
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Windows" AND CMAKE_BUILD_TYPE MATCHES "Debug")
	set(sfmlSystem sfml-system-d)
	set(sfmlWindow sfml-window-d)
	set(sfmlGraphics sfml-graphics-d)
else()
	set(sfmlSystem sfml-system)
	set(sfmlWindow sfml-window)
	set(sfmlGraphics sfml-graphics)
endif()

#=====include=====#
include_directories(
		"../src"
		"${sfmlDir}/include"
)

#=====source=====#
set(
	src
	"../src/main.cpp"
)

#=====products=====#
add_executable(${output} ${src})

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	set(
		sfmlDlls
		"${sfmlDir}/bin/${sfmlGraphics}-2.dll"
		"${sfmlDir}/bin/${sfmlSystem}-2.dll"
		"${sfmlDir}/bin/${sfmlWindow}-2.dll"
	)
	file(COPY ${sfmlDlls} DESTINATION "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
endif()

#=====linking=====#
set(CMAKE_LIBRARY_PATH "${sfmlDir}/lib")

find_library(SFML_SYSTEM ${sfmlSystem})
find_library(SFML_WINDOW ${sfmlWindow})
find_library(SFML_GRAPHICS ${sfmlGraphics})

target_link_libraries(
	${output}
	${SFML_SYSTEM}
	${SFML_WINDOW}
	${SFML_GRAPHICS}
)

#=====c++ standard=====#
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	set(CMAKE_CXX_FLAGS "-std=c++11")
endif()
