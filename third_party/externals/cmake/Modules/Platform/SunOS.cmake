if(CMAKE_SYSTEM MATCHES "SunOS-4")
   set(CMAKE_C_COMPILE_OPTIONS_PIC "-PIC")
   set(CMAKE_C_COMPILE_OPTIONS_PIE "-PIE")
   set(CMAKE_SHARED_LIBRARY_C_FLAGS "-PIC")
   set(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "-shared -Wl,-r")
   set(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG "-Wl,-R")
   set(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG_SEP ":")
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
  if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_CREATE_SHARED_LIBRARY
        "<CMAKE_C_COMPILER> <CMAKE_SHARED_LIBRARY_CXX_FLAGS> <LINK_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS>  <SONAME_FLAG><TARGET_SONAME> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>")
  else()
    # Take default rule from CMakeDefaultMakeRuleVariables.cmake.
  endif()
endif()
include(Platform/UnixPaths)

# Add the compiler's implicit link directories.
if("${CMAKE_C_COMPILER_ID} ${CMAKE_CXX_COMPILER_ID}" MATCHES SunPro)
  list(APPEND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES
    /opt/SUNWspro/lib /opt/SUNWspro/prod/lib /usr/ccs/lib)
endif()

# The Sun linker needs to find transitive shared library dependencies
# in the -L path.
set(CMAKE_LINK_DEPENDENT_LIBRARY_DIRS 1)

# Shared libraries with no builtin soname may not be linked safely by
# specifying the file path.
set(CMAKE_PLATFORM_USES_PATH_WHEN_NO_SONAME 1)
