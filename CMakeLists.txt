cmake_minimum_required (VERSION 3.1)
project(CG1Examples)

FUNCTION(PREPEND var prefix)
   SET(listVar "")
   FOREACH(f ${ARGN})
      LIST(APPEND listVar "${prefix}/${f}")
   ENDFOREACH(f)
   SET(${var} "${listVar}" PARENT_SCOPE)
ENDFUNCTION(PREPEND)

FUNCTION(JOIN VALUES GLUE OUTPUT)
  string (REGEX REPLACE "([^\\]|^);" "\\1${GLUE}" _TMP_STR "${VALUES}")
  string (REGEX REPLACE "[\\](.)" "\\1" _TMP_STR "${_TMP_STR}") #fixes escaping
  set (${OUTPUT} "${_TMP_STR}" PARENT_SCOPE)
ENDFUNCTION()

set(LIBS)

# Enable folders for projects in Visual Studio
if (CMAKE_GENERATOR MATCHES "Visual Studio")
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

# Sanitize build environment for static build with C++14
if (MSVC)
  add_definitions (/D "_CRT_SECURE_NO_WARNINGS")  

  add_definitions (-DNOMINMAX )
  add_definitions(/D_USE_MATH_DEFINES)

  # Parallel build on MSVC (all targets)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")

  if (NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE2")

    # Disable Eigen vectorization for Windows 32 bit builds (issues with unaligned access segfaults)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DEIGEN_DONT_ALIGN")
  endif()

  # Static build
  set(CompilerFlags
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO
        CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
  foreach(CompilerFlag ${CompilerFlags})
    string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
  endforeach()
elseif(APPLE)
  # Try to auto-detect a suitable SDK
  execute_process(COMMAND bash -c "xcodebuild -version -sdk | grep MacOSX | grep Path | head -n 1 | cut -f 2 -d ' '" OUTPUT_VARIABLE CMAKE_OSX_SYSROOT)
  string(REGEX REPLACE "(\r?\n)+$" "" CMAKE_OSX_SYSROOT "${CMAKE_OSX_SYSROOT}")
  string(REGEX REPLACE "^.*X([0-9.]*).sdk$" "\\1" CMAKE_OSX_DEPLOYMENT_TARGET "${CMAKE_OSX_SYSROOT}")
endif()

set (CMAKE_CXX_STANDARD 14)

# Build NanoGUI
set(NANOGUI_BUILD_EXAMPLE OFF CACHE BOOL " " FORCE)
set(NANOGUI_BUILD_SHARED  OFF CACHE BOOL " " FORCE)
set(NANOGUI_BUILD_PYTHON  OFF CACHE BOOL " " FORCE)
set(NANOGUI_USE_GLAD ON CACHE BOOL " " FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ext/nanogui ext_build/nanogui)
set_property(TARGET nanogui glfw glfw_objects PROPERTY FOLDER "dependencies")
include_directories(ext/nanogui/include
					${NANOGUI_EXTRA_INCS})
set(LIBS ${LIBS} nanogui ${NANOGUI_EXTRA_LIBS})
set(NSE_TESSELLATION_SUPPORT ON CACHE BOOL " " FORCE)
set(NSE_WITH_NANOGUI ON CACHE BOOL " " FORCE)
set(NSE_WITH_EIGEN ON CACHE BOOL " " FORCE)
add_definitions(${NANOGUI_EXTRA_DEFS})

#NS Essentials
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ext/nsessentials ext_build/nsessentials)
set_property(TARGET nsessentials PROPERTY FOLDER "dependencies")
include_directories(ext/nsessentials/include)
set(LIBS nsessentials ${LIBS})
add_definitions(${NSE_EXTRA_DEFS})

#Compile GLSL files to CPP file
set(GLSL_FILES	simple.vert simple.frag
				mesh.vert meshInstanced.vert meshIllum.vert
				normal.vert normal.geom constantColor.frag
				blinnphong.frag orennayar.frag
				texturedmesh.vert texturedMesh.frag texturedMesh.tcs texturedMesh.tes)

PREPEND(GLSL_FILES "${CMAKE_CURRENT_SOURCE_DIR}/glsl/" ${GLSL_FILES})

source_group(glsl FILES ${GLSL_FILES})

JOIN("${GLSL_FILES}" "," glsl_string)
set(bin2c_cmdline
    -DOUTPUT_C=glsl.cpp
    -DOUTPUT_H=glsl.h
    "-DINPUT_FILES=${glsl_string}"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/ext/nanogui/resources/bin2c.cmake")

add_custom_command(
  OUTPUT glsl.cpp glsl.h
  COMMAND ${CMAKE_COMMAND} ARGS ${bin2c_cmdline}
  DEPENDS ${GLSL_FILES}
  COMMENT "Running bin2c"
  PRE_BUILD VERBATIM)

include_directories( ${CMAKE_CURRENT_BINARY_DIR}  )

SET(THIRD_PARTY 	ext/tinyobjloader/tiny_obj_loader.cc)
source_group(third_party FILES ${THIRD_PARTY})		
include_directories(ext)

add_executable(CG1Examples MACOSX_BUNDLE
	glsl.cpp
	src/main.cpp	
	src/Viewer.cpp src/Viewer.h	
	src/Content.h
	src/TriangleContent.h src/TriangleContent.cpp
	src/HeightFieldContent.h src/HeightFieldContent.cpp
	src/SimpleOBJContent.h src/SimpleOBJContent.cpp
	src/IlluminatedMeshContent.h src/IlluminatedMeshContent.cpp
	src/TexturedMeshContent.h src/TexturedMeshContent.cpp
	src/Mesh.h src/Mesh.cpp
	src/ShaderPool.h src/ShaderPool.cpp
	${GLSL_FILES}
	${THIRD_PARTY})

target_link_libraries(CG1Examples ${LIBS})