# Keep formatting here consistent with bootstrap script expectations.
if(BEOS)
  set(CMAKE_DATA_DIR_DEFAULT "share/cmake-${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}") # HAIKU
  set(CMAKE_MAN_DIR_DEFAULT "documentation/man") # HAIKU
  set(CMAKE_DOC_DIR_DEFAULT "documentation/doc/cmake-${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}") # HAIKU
elseif(CYGWIN)
  set(CMAKE_DATA_DIR_DEFAULT "share/cmake-${CMake_VERSION}") # CYGWIN
  set(CMAKE_DOC_DIR_DEFAULT "share/doc/cmake-${CMake_VERSION}") # CYGWIN
  set(CMAKE_MAN_DIR_DEFAULT "share/man") # CYGWIN
else()
  set(CMAKE_DATA_DIR_DEFAULT "share/cmake-${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}") # OTHER
  set(CMAKE_DOC_DIR_DEFAULT "doc/cmake-${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}") # OTHER
  set(CMAKE_MAN_DIR_DEFAULT "man") # OTHER
endif()

set(CMAKE_DATA_DIR_DESC "data")
set(CMAKE_DOC_DIR_DESC "docs")
set(CMAKE_MAN_DIR_DESC "man pages")

foreach(v
    CMAKE_DATA_DIR
    CMAKE_DOC_DIR
    CMAKE_MAN_DIR
    )
  # Populate the cache with empty values so we know when the user sets them.
  set(${v} "" CACHE STRING "")
  set_property(CACHE ${v} PROPERTY HELPSTRING
    "Location under install prefix for ${${v}_DESC} (default \"${${v}_DEFAULT}\")"
    )
  set_property(CACHE ${v} PROPERTY ADVANCED 1)

  # Use the default when the user did not set this variable.
  if(NOT ${v})
    set(${v} "${${v}_DEFAULT}")
  endif()
  # Remove leading slash to treat as relative to install prefix.
  string(REGEX REPLACE "^/" "" ${v} "${${v}}")
endforeach()
