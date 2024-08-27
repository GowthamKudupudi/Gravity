# CMake entry point
cmake_minimum_required (VERSION 3.0)
project (Gravity)

find_package(OpenGL REQUIRED)


if( CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR )
    message( FATAL_ERROR "Please select another Build Directory ! (and give it a clever name, like bin_Visual2012_64bits/)" )
endif()
if( CMAKE_SOURCE_DIR MATCHES " " )
	message( "Your Source Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()
if( CMAKE_BINARY_DIR MATCHES " " )
	message( "Your Build Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()



# Compile external dependencies 
add_subdirectory (external)

# On Visual 2005 and above, this module can set the debug working directory
cmake_policy(SET CMP0026 OLD)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/external/rpavlik-cmake-modules-fe2273")
include(CreateLaunchers)
include(MSVCMultipleProcessCompile) # /MP

if(INCLUDE_DISTRIB)
	add_subdirectory(distrib)
endif(INCLUDE_DISTRIB)

set(CMAKE_MODULE_PATH
   "${CMAKE_CURRENT_SOURCE_DIR}/../ferrybase;${CMAKE_MODULE_PATH}"
)
message("CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")
include(TargetArch)
target_architecture(TARGET_ARCH)
message ("TARGET_ARCH: ${TARGET_ARCH}")

string (
   REPLACE ${CMAKE_CURRENT_SOURCE_DIR} "" SRC_RLTV_BLD_DIR
   ${PROJECT_BINARY_DIR}
)
message ("SRC_RLTV_BLD_DIR: ${SRC_RLTV_BLD_DIR}")
string (REPLACE "/Build" "" BUILD_TARGET ${SRC_RLTV_BLD_DIR})
if (${SRC_RLTV_BLD_DIR})
   string (REPLACE "/build" "" BUILD_TARGET ${BUILD_TARGET})
endif ()
message ("BUILD_TARGET: ${BUILD_TARGET}")

add_subdirectory (
   ${CMAKE_CURRENT_SOURCE_DIR}/../ferrybase
   ${CMAKE_CURRENT_SOURCE_DIR}/../ferrybase/${SRC_RLTV_BLD_DIR}
)

include_directories(
	external/glfw-3.1.2/include/GLFW/
	external/glm-0.9.7.1/
	external/glew-1.13.0/include/
   ${CMAKE_CURRENT_SOURCE_DIR}/../ferrybase
   ${CMAKE_CURRENT_SOURCE_DIR}/../ferrybase${BUILD_TARGET} 
   ${CMAKE_CURRENT_SOURCE_DIR}/..
	.
)
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   link_directories(
      /usr/local/lib/LASlib
   )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
   include_directories(
      external/las/inc
      ../thirdparty/CGAL-5.6.1/include
      ../thirdparty/boost_1_85_0
   )
   link_directories(
      external/las/lib
      ../thirdparty/boost_1_85_0/stage/lib
      ../thirdparty/CGAL-5.6.1/auxiliary/gmp/lib
   )
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
link_directories (
   "${CMAKE_CURRENT_SOURCE_DIR}/../ferrybase/${BLD_DIR}"
)

set(ALL_LIBS
	${OPENGL_LIBRARY}
	glfw
	GLEW_1130
   LASlib
   gmp
   ferrybase
)

add_definitions(
	-DTW_STATIC
	-DTW_NO_LIB_PRAGMA
	-DTW_NO_DIRECT3D
	-DGLEW_STATIC
	-D_CRT_SECURE_NO_WARNINGS
)

if (DEFINED _DEBUG)
   ADD_DEFINITIONS(-D_DEBUG=${_DEBUG})
   set (GCC_COVERAGE_COMPILE_FLAGS "${GCC_COVERAGE_COMPILE_FLAGS} -g -O0")
   set (GCC_COVERAGE_LINK_FLAGS "${GCC_COVERAGE_LINK_FLAGS} -g -O0")
endif ()
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")

# User playground
add_executable(playground 
	playground/playground.cpp
   playground/world.cpp
   playground/world.hpp
   playground/playgroundfragment.glsl
   playground/playgroundvertex.glsl
   playground/TextFragment.glsl
   playground/TextVertex.glsl
   playground/Holstein.DDS
   common/shader.cpp
	common/shader.hpp
   common/texture.cpp
   common/texture.hpp
   common/controls.cpp
   common/controls.hpp
   common/text2D.cpp
   common/text2D.hpp
)

target_link_libraries(playground
	${ALL_LIBS}
)
# Xcode and Visual working directories
set_target_properties(playground PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/playground/")
create_target_launcher(playground WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/playground/")

SOURCE_GROUP(common REGULAR_EXPRESSION ".*/common/.*" )
SOURCE_GROUP(shaders REGULAR_EXPRESSION ".*/.*shader$" )


if (NOT ${CMAKE_GENERATOR} MATCHES "Xcode" )
add_custom_command(
   TARGET playground POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/playground${CMAKE_EXECUTABLE_SUFFIX}" "${CMAKE_CURRENT_SOURCE_DIR}/playground/"
)

elseif (${CMAKE_GENERATOR} MATCHES "Xcode" )

endif (NOT ${CMAKE_GENERATOR} MATCHES "Xcode" )

