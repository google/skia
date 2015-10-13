#

#=============================================================================
# Copyright 2002-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

# This is an implementation detail for using VTK 4.0 with the
# FindVTK.cmake module.  Do not include directly.

# Hard-code the version number since it isn't provided by VTK 4.0.
set(VTK_MAJOR_VERSION 4)
set(VTK_MINOR_VERSION 0)
set(VTK_BUILD_VERSION 2)

# Provide a new UseVTK file that doesn't do a full LOAD_CACHE.
set(VTK_USE_FILE ${CMAKE_ROOT}/Modules/UseVTK40.cmake)

# Provide a build settings file.
set(VTK_BUILD_SETTINGS_FILE ${CMAKE_ROOT}/Modules/UseVTKBuildSettings40.cmake)

# There are no CMake extensions for VTK 4.0.
set(VTK_CMAKE_EXTENSIONS_DIR "")

# grep "VTK40_" UseVTKConfig40.cmake |sed 's/.*VTK40_\([A-Za-z0-9_]*\).*/  \1/'
load_cache(${VTK_DIR} READ_WITH_PREFIX VTK40_
  BUILD_SHARED_LIBS
  CMAKE_BUILD_TOOL
  CMAKE_BUILD_TYPE
  CMAKE_CACHE_MAJOR_VERSION
  CMAKE_CACHE_MINOR_VERSION
  CMAKE_CXX_COMPILER
  CMAKE_CXX_FLAGS
  CMAKE_CXX_FLAGS_DEBUG
  CMAKE_CXX_FLAGS_MINSIZEREL
  CMAKE_CXX_FLAGS_RELEASE
  CMAKE_CXX_FLAGS_RELWITHDEBINFO
  CMAKE_C_COMPILER
  CMAKE_C_FLAGS
  CMAKE_C_FLAGS_DEBUG
  CMAKE_C_FLAGS_MINSIZEREL
  CMAKE_C_FLAGS_RELEASE
  CMAKE_C_FLAGS_RELWITHDEBINFO
  CMAKE_INSTALL_PREFIX
  CMAKE_Xutil_INCLUDE_PATH
  EXECUTABLE_OUTPUT_PATH
  JAVA_INCLUDE_PATH2
  LIBRARY_OUTPUT_PATH
  MPIRUN
  MPI_INCLUDE_PATH
  MPI_POSTFLAGS
  MPI_PREFLAGS
  OPENGL_INCLUDE_DIR
  OSMESA_INCLUDE_PATH
  PYTHON_INCLUDE_PATH
  TCL_INCLUDE_PATH
  VLI_INCLUDE_PATH_FOR_VG500
  VLI_INCLUDE_PATH_FOR_VP1000
  VTK_BINARY_DIR
  VTK_DEBUG_LEAKS
  VTK_HAVE_VG500
  VTK_HAVE_VP1000
  VTK_MANGLE_MESA
  VTK_OPENGL_HAS_OSMESA
  VTK_PARSE_JAVA_EXE
  VTK_SOURCE_DIR
  VTK_USE_64BIT_IDS
  VTK_USE_ANSI_STDLIB
  VTK_USE_HYBRID
  VTK_USE_MATROX_IMAGING
  VTK_USE_MPI
  VTK_USE_PARALLEL
  VTK_USE_PATENTED
  VTK_USE_RENDERING
  VTK_USE_VIDEO_FOR_WINDOWS
  VTK_USE_VOLUMEPRO
  VTK_USE_X
  VTK_WRAP_JAVA
  VTK_WRAP_JAVA_EXE
  VTK_WRAP_PYTHON
  VTK_WRAP_PYTHON_EXE
  VTK_WRAP_TCL
  VTK_WRAP_TCL_EXE
  vtkCommonJava_LIB_DEPENDS
  vtkCommonPython_LIB_DEPENDS
  vtkCommonTCL_LIB_DEPENDS
  vtkCommon_LIB_DEPENDS
  vtkFilteringJava_LIB_DEPENDS
  vtkFilteringPython_LIB_DEPENDS
  vtkFilteringTCL_LIB_DEPENDS
  vtkFiltering_LIB_DEPENDS
  vtkGraphicsJava_LIB_DEPENDS
  vtkGraphicsPython_LIB_DEPENDS
  vtkGraphicsTCL_LIB_DEPENDS
  vtkGraphics_LIB_DEPENDS
  vtkHybridJava_LIB_DEPENDS
  vtkHybridPython_LIB_DEPENDS
  vtkHybridTCL_LIB_DEPENDS
  vtkHybrid_LIB_DEPENDS
  vtkIOJava_LIB_DEPENDS
  vtkIOPython_LIB_DEPENDS
  vtkIOTCL_LIB_DEPENDS
  vtkIO_LIB_DEPENDS
  vtkImagingJava_LIB_DEPENDS
  vtkImagingPython_LIB_DEPENDS
  vtkImagingTCL_LIB_DEPENDS
  vtkImaging_LIB_DEPENDS
  vtkParallelJava_LIB_DEPENDS
  vtkParallelPython_LIB_DEPENDS
  vtkParallelTCL_LIB_DEPENDS
  vtkParallel_LIB_DEPENDS
  vtkPatentedJava_LIB_DEPENDS
  vtkPatentedPython_LIB_DEPENDS
  vtkPatentedTCL_LIB_DEPENDS
  vtkPatented_LIB_DEPENDS
  vtkRenderingJava_LIB_DEPENDS
  vtkRenderingPythonTkWidgets_LIB_DEPENDS
  vtkRenderingPython_LIB_DEPENDS
  vtkRenderingTCL_LIB_DEPENDS
  vtkRendering_LIB_DEPENDS
  vtkjpeg_LIB_DEPENDS
  vtkpng_LIB_DEPENDS
  vtkzlib_LIB_DEPENDS
)

# Copy needed settings from the VTK 4.0 cache.
set(VTK_BUILD_SHARED ${VTK40_BUILD_SHARED_LIBS})
set(VTK_DEBUG_LEAKS ${VTK40_VTK_DEBUG_LEAKS})
set(VTK_HAVE_VG500 ${VTK40_VTK_HAVE_VG500})
set(VTK_HAVE_VP1000 ${VTK40_VTK_HAVE_VP1000})
set(VTK_USE_MANGLED_MESA ${VTK40_VTK_MANGLE_MESA})
set(VTK_MPIRUN_EXE ${VTK40_MPIRUN})
set(VTK_MPI_POSTFLAGS ${VTK40_MPI_POSTFLAGS})
set(VTK_MPI_PREFLAGS ${VTK40_MPI_PREFLAGS})
set(VTK_OPENGL_HAS_OSMESA ${VTK40_VTK_OPENGL_HAS_OSMESA})
set(VTK_USE_64BIT_IDS ${VTK40_VTK_USE_64BIT_IDS})
set(VTK_USE_ANSI_STDLIB ${VTK40_VTK_USE_ANSI_STDLIB})
set(VTK_USE_HYBRID ${VTK40_VTK_USE_HYBRID})
set(VTK_USE_MATROX_IMAGING ${VTK40_VTK_USE_MATROX_IMAGING})
set(VTK_USE_MPI ${VTK40_VTK_USE_MPI})
set(VTK_USE_PARALLEL ${VTK40_VTK_USE_PARALLEL})
set(VTK_USE_PATENTED ${VTK40_VTK_USE_PATENTED})
set(VTK_USE_RENDERING ${VTK40_VTK_USE_RENDERING})
set(VTK_USE_VIDEO_FOR_WINDOWS ${VTK40_VTK_USE_VIDEO_FOR_WINDOWS})
set(VTK_USE_VOLUMEPRO ${VTK40_VTK_USE_VOLUMEPRO})
set(VTK_USE_X ${VTK40_VTK_USE_X})
set(VTK_WRAP_JAVA ${VTK40_VTK_WRAP_JAVA})
set(VTK_WRAP_PYTHON ${VTK40_VTK_WRAP_PYTHON})
set(VTK_WRAP_TCL ${VTK40_VTK_WRAP_TCL})

# Create the list of available kits.
set(VTK_KITS COMMON FILTERING GRAPHICS IMAGING IO)
if(VTK_USE_RENDERING)
  set(VTK_KITS ${VTK_KITS} RENDERING)
endif()
if(VTK_USE_HYBRID)
  set(VTK_KITS ${VTK_KITS} HYBRID)
endif()
if(VTK_USE_PARALLEL)
  set(VTK_KITS ${VTK_KITS} PARALLEL)
endif()
if(VTK_USE_PATENTED)
  set(VTK_KITS ${VTK_KITS} PATENTED)
endif()

# Create the list of available languages.
set(VTK_LANGUAGES "")
if(VTK_WRAP_TCL)
  set(VTK_LANGUAGES ${VTK_LANGUAGES} TCL)
endif()
if(VTK_WRAP_PYTHON)
  set(VTK_LANGUAGES ${VTK_LANGUAGES} PYTHON)
endif()
if(VTK_WRAP_JAVA)
  set(VTK_LANGUAGES ${VTK_LANGUAGES} JAVA)
endif()

# Include directories for other projects installed on the system and
# used by VTK.
set(VTK_INCLUDE_DIRS_SYS "")
if(VTK_USE_RENDERING)
  set(VTK_INCLUDE_DIRS_SYS ${VTK_INCLUDE_DIRS_SYS}
      ${VTK40_OPENGL_INCLUDE_PATH} ${VTK40_OPENGL_INCLUDE_DIR})
  if(VTK_USE_X)
    set(VTK_INCLUDE_DIRS_SYS ${VTK_INCLUDE_DIRS_SYS}
        ${VTK40_CMAKE_Xlib_INCLUDE_PATH} ${VTK40_CMAKE_Xutil_INCLUDE_PATH})
  endif()
endif()

if(VTK_OPENGL_HAS_OSMESA)
  set(VTK_INCLUDE_DIRS_SYS ${VTK_INCLUDE_DIRS_SYS}
      ${VTK40_OSMESA_INCLUDE_PATH})
endif()

if(VTK_USE_MPI)
  set(VTK_INCLUDE_DIRS_SYS ${VTK_INCLUDE_DIRS_SYS} ${VTK40_MPI_INCLUDE_PATH})
endif()

if(VTK_WRAP_TCL)
  set(VTK_INCLUDE_DIRS_SYS ${VTK_INCLUDE_DIRS_SYS} ${VTK40_TCL_INCLUDE_PATH})
endif()

if(VTK_WRAP_PYTHON)
  set(VTK_INCLUDE_DIRS_SYS ${VTK_INCLUDE_DIRS_SYS} ${VTK40_PYTHON_INCLUDE_PATH})
endif()

if(VTK_WRAP_JAVA)
  set(VTK_INCLUDE_DIRS_SYS ${VTK_INCLUDE_DIRS_SYS}
      ${VTK40_JAVA_INCLUDE_PATH} ${VTK40_JAVA_INCLUDE_PATH2})
endif()

if(VTK_HAVE_VG500)
  set(VTK_INCLUDE_DIRS_SYS ${VTK_INCLUDE_DIRS_SYS}
      ${VTK40_VLI_INCLUDE_PATH_FOR_VG500})
endif()

if(VTK_HAVE_VP1000)
  set(VTK_INCLUDE_DIRS_SYS ${VTK_INCLUDE_DIRS_SYS}
      ${VTK40_VLI_INCLUDE_PATH_FOR_VP1000})
endif()

# See if this is a build tree or install tree.
if(EXISTS ${VTK_DIR}/Common)
  # This is a VTK 4.0 build tree.

  set(VTK_LIBRARY_DIRS ${VTK40_LIBRARY_OUTPUT_PATH})

  # Determine the include directories needed.
  set(VTK_INCLUDE_DIRS ${VTK40_VTK_BINARY_DIR})
  if(VTK_USE_PARALLEL)
    set(VTK_INCLUDE_DIRS ${VTK_INCLUDE_DIRS} ${VTK40_VTK_SOURCE_DIR}/Parallel)
  endif()
  if(VTK_USE_HYBRID)
    set(VTK_INCLUDE_DIRS ${VTK_INCLUDE_DIRS} ${VTK40_VTK_SOURCE_DIR}/Hybrid)
  endif()
  if(VTK_USE_PATENTED)
    set(VTK_INCLUDE_DIRS ${VTK_INCLUDE_DIRS} ${VTK40_VTK_SOURCE_DIR}/Patented)
  endif()
  if(VTK_USE_RENDERING)
    set(VTK_INCLUDE_DIRS ${VTK_INCLUDE_DIRS} ${VTK40_VTK_SOURCE_DIR}/Rendering)
  endif()

  # These directories are always needed.
  set(VTK_INCLUDE_DIRS ${VTK_INCLUDE_DIRS}
    ${VTK40_VTK_SOURCE_DIR}/IO
    ${VTK40_VTK_SOURCE_DIR}/Imaging
    ${VTK40_VTK_SOURCE_DIR}/Graphics
    ${VTK40_VTK_SOURCE_DIR}/Filtering
    ${VTK40_VTK_SOURCE_DIR}/Common)

  # Give access to a few utilities.
  set(VTK_INCLUDE_DIRS ${VTK_INCLUDE_DIRS}
    ${VTK40_VTK_BINARY_DIR}/Utilities/png
    ${VTK40_VTK_SOURCE_DIR}/Utilities/png
    ${VTK40_VTK_BINARY_DIR}/Utilities/zlib
    ${VTK40_VTK_SOURCE_DIR}/Utilities/zlib)

  # Executable locations.
  if(VTK_WRAP_TCL)
    set(VTK_TCL_EXE ${VTK40_EXECUTABLE_OUTPUT_PATH}/vtk)
    set(VTK_WRAP_TCL_EXE ${VTK40_VTK_WRAP_TCL_EXE})
    set(VTK_TCL_HOME ${VTK40_VTK_SOURCE_DIR}/Wrapping/Tcl)
  endif()
  if(VTK_WRAP_PYTHON)
    set(VTK_WRAP_PYTHON_EXE ${VTK40_VTK_WRAP_PYTHON_EXE})
  endif()
  if(VTK_WRAP_JAVA)
    set(VTK_PARSE_JAVA_EXE ${VTK40_VTK_PARSE_JAVA_EXE})
    set(VTK_WRAP_JAVA_EXE ${VTK40_VTK_WRAP_JAVA_EXE})
  endif()

else()
  # This is a VTK 4.0 install tree.

  set(VTK_INCLUDE_DIRS ${VTK_DIR})
  set(VTK_LIBRARY_DIRS ${VTK40_CMAKE_INSTALL_PREFIX}/lib/vtk)

  # Executable locations.
  if(VTK_WRAP_TCL)
    set(VTK_TCL_EXE ${VTK40_CMAKE_INSTALL_PREFIX}/bin/vtk)
    set(VTK_WRAP_TCL_EXE ${VTK40_CMAKE_INSTALL_PREFIX}/bin/vtkWrapTcl)
    set(VTK_TCL_HOME ${VTK40_CMAKE_INSTALL_PREFIX}/lib/vtk/tcl)
  endif()
  if(VTK_WRAP_PYTHON)
    set(VTK_WRAP_PYTHON_EXE ${VTK40_CMAKE_INSTALL_PREFIX}/bin/vtkWrapPython)
  endif()
  if(VTK_WRAP_JAVA)
    set(VTK_PARSE_JAVA_EXE ${VTK40_CMAKE_INSTALL_PREFIX}/bin/vtkParseJava)
    set(VTK_WRAP_JAVA_EXE ${VTK40_CMAKE_INSTALL_PREFIX}/bin/vtkWrapJava)
  endif()
endif()

# Add the system include directories last.
set(VTK_INCLUDE_DIRS ${VTK_INCLUDE_DIRS} ${VTK_INCLUDE_DIRS_SYS})

# Find the required C and C++ compiler flags.
if(CMAKE_COMPILER_IS_GNUCXX)
  if(WIN32)
    # The platform is gcc on cygwin.
    set(VTK_REQUIRED_CXX_FLAGS "${VTK_REQUIRED_CXX_FLAGS} -mwin32")
    set(VTK_REQUIRED_C_FLAGS "${VTK_REQUIRED_C_FLAGS} -mwin32")
  endif()
else()
  if(CMAKE_ANSI_CFLAGS)
    set(VTK_REQUIRED_C_FLAGS "${VTK_REQUIRED_C_FLAGS} ${CMAKE_ANSI_CFLAGS}")
  endif()
  if(CMAKE_SYSTEM MATCHES "OSF1-V")
     set(VTK_REQUIRED_CXX_FLAGS
         "${VTK_REQUIRED_CXX_FLAGS} -timplicit_local -no_implicit_include")
  endif()
endif()

if(VTK_USE_X)
  if(CMAKE_X_CFLAGS)
    set(VTK_REQUIRED_C_FLAGS "${VTK_REQUIRED_C_FLAGS} ${CMAKE_X_CFLAGS}")
    set(VTK_REQUIRED_CXX_FLAGS "${VTK_REQUIRED_CXX_FLAGS} ${CMAKE_X_CFLAGS}")
  endif()
endif()

# Copy library dependencies.
set(vtkCommonJava_LIB_DEPENDS "${VTK40_vtkCommonJava_LIB_DEPENDS}")
set(vtkCommonPython_LIB_DEPENDS "${VTK40_vtkCommonPython_LIB_DEPENDS}")
set(vtkCommonTCL_LIB_DEPENDS "${VTK40_vtkCommonTCL_LIB_DEPENDS}")
set(vtkCommon_LIB_DEPENDS "${VTK40_vtkCommon_LIB_DEPENDS}")
set(vtkFilteringJava_LIB_DEPENDS "${VTK40_vtkFilteringJava_LIB_DEPENDS}")
set(vtkFilteringPython_LIB_DEPENDS "${VTK40_vtkFilteringPython_LIB_DEPENDS}")
set(vtkFilteringTCL_LIB_DEPENDS "${VTK40_vtkFilteringTCL_LIB_DEPENDS}")
set(vtkFiltering_LIB_DEPENDS "${VTK40_vtkFiltering_LIB_DEPENDS}")
set(vtkGraphicsJava_LIB_DEPENDS "${VTK40_vtkGraphicsJava_LIB_DEPENDS}")
set(vtkGraphicsPython_LIB_DEPENDS "${VTK40_vtkGraphicsPython_LIB_DEPENDS}")
set(vtkGraphicsTCL_LIB_DEPENDS "${VTK40_vtkGraphicsTCL_LIB_DEPENDS}")
set(vtkGraphics_LIB_DEPENDS "${VTK40_vtkGraphics_LIB_DEPENDS}")
set(vtkHybridJava_LIB_DEPENDS "${VTK40_vtkHybridJava_LIB_DEPENDS}")
set(vtkHybridPython_LIB_DEPENDS "${VTK40_vtkHybridPython_LIB_DEPENDS}")
set(vtkHybridTCL_LIB_DEPENDS "${VTK40_vtkHybridTCL_LIB_DEPENDS}")
set(vtkHybrid_LIB_DEPENDS "${VTK40_vtkHybrid_LIB_DEPENDS}")
set(vtkIOJava_LIB_DEPENDS "${VTK40_vtkIOJava_LIB_DEPENDS}")
set(vtkIOPython_LIB_DEPENDS "${VTK40_vtkIOPython_LIB_DEPENDS}")
set(vtkIOTCL_LIB_DEPENDS "${VTK40_vtkIOTCL_LIB_DEPENDS}")
set(vtkIO_LIB_DEPENDS "${VTK40_vtkIO_LIB_DEPENDS}")
set(vtkImagingJava_LIB_DEPENDS "${VTK40_vtkImagingJava_LIB_DEPENDS}")
set(vtkImagingPython_LIB_DEPENDS "${VTK40_vtkImagingPython_LIB_DEPENDS}")
set(vtkImagingTCL_LIB_DEPENDS "${VTK40_vtkImagingTCL_LIB_DEPENDS}")
set(vtkImaging_LIB_DEPENDS "${VTK40_vtkImaging_LIB_DEPENDS}")
set(vtkParallelJava_LIB_DEPENDS "${VTK40_vtkParallelJava_LIB_DEPENDS}")
set(vtkParallelPython_LIB_DEPENDS "${VTK40_vtkParallelPython_LIB_DEPENDS}")
set(vtkParallelTCL_LIB_DEPENDS "${VTK40_vtkParallelTCL_LIB_DEPENDS}")
set(vtkParallel_LIB_DEPENDS "${VTK40_vtkParallel_LIB_DEPENDS}")
set(vtkPatentedJava_LIB_DEPENDS "${VTK40_vtkPatentedJava_LIB_DEPENDS}")
set(vtkPatentedPython_LIB_DEPENDS "${VTK40_vtkPatentedPython_LIB_DEPENDS}")
set(vtkPatentedTCL_LIB_DEPENDS "${VTK40_vtkPatentedTCL_LIB_DEPENDS}")
set(vtkPatented_LIB_DEPENDS "${VTK40_vtkPatented_LIB_DEPENDS}")
set(vtkRenderingJava_LIB_DEPENDS "${VTK40_vtkRenderingJava_LIB_DEPENDS}")
set(vtkRenderingPythonTkWidgets_LIB_DEPENDS "${VTK40_vtkRenderingPythonTkWidgets_LIB_DEPENDS}")
set(vtkRenderingPython_LIB_DEPENDS "${VTK40_vtkRenderingPython_LIB_DEPENDS}")
set(vtkRenderingTCL_LIB_DEPENDS "${VTK40_vtkRenderingTCL_LIB_DEPENDS}")
set(vtkRendering_LIB_DEPENDS "${VTK40_vtkRendering_LIB_DEPENDS}")
set(vtkjpeg_LIB_DEPENDS "${VTK40_vtkjpeg_LIB_DEPENDS}")
set(vtkpng_LIB_DEPENDS "${VTK40_vtkpng_LIB_DEPENDS}")
set(vtkzlib_LIB_DEPENDS "${VTK40_vtkzlib_LIB_DEPENDS}")

# List of VTK configuration variables set above.
# grep "^[ ]*set(VTK" UseVTKConfig40.cmake |sed 's/[ ]*set(\([^ ]*\) .*/  \1/'
set(VTK_SETTINGS
  VTK_BUILD_SHARED
  VTK_BUILD_VERSION
  VTK_DEBUG_LEAKS
  VTK_HAVE_VG500
  VTK_HAVE_VP1000
  VTK_INCLUDE_DIRS
  VTK_KITS
  VTK_LANGUAGES
  VTK_LIBRARY_DIRS
  VTK_MAJOR_VERSION
  VTK_MANGLE_MESA
  VTK_MINOR_VERSION
  VTK_MPIRUN_EXE
  VTK_MPI_POSTFLAGS
  VTK_MPI_PREFLAGS
  VTK_OPENGL_HAS_OSMESA
  VTK_PARSE_JAVA_EXE
  VTK_TCL_EXE
  VTK_TCL_HOME
  VTK_USE_64BIT_IDS
  VTK_USE_ANSI_STDLIB
  VTK_USE_HYBRID
  VTK_USE_MATROX_IMAGING
  VTK_USE_MPI
  VTK_USE_PARALLEL
  VTK_USE_PATENTED
  VTK_USE_RENDERING
  VTK_USE_VIDEO_FOR_WINDOWS
  VTK_USE_VOLUMEPRO
  VTK_USE_X
  VTK_WRAP_JAVA
  VTK_WRAP_JAVA_EXE
  VTK_WRAP_PYTHON
  VTK_WRAP_PYTHON_EXE
  VTK_WRAP_TCL
  VTK_WRAP_TCL_EXE
)
