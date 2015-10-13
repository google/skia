# Needed for CMAKE_SYSTEM_NAME, CMAKE_LIBRARY_ARCHITECTURE and FIND_LIBRARY_USE_LIB64_PATHS
enable_language(C)

# Prepare environment and variables
set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH TRUE)
set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/pc-foo")
if(WIN32)
    set(PKG_CONFIG_EXECUTABLE "${CMAKE_CURRENT_SOURCE_DIR}\\dummy-pkg-config.bat")
    set(ENV{CMAKE_PREFIX_PATH} "${CMAKE_CURRENT_SOURCE_DIR}\\pc-bar;X:\\this\\directory\\should\\not\\exist\\in\\the\\filesystem")
    set(ENV{PKG_CONFIG_PATH} "C:\\baz")
else()
    set(PKG_CONFIG_EXECUTABLE "${CMAKE_CURRENT_SOURCE_DIR}/dummy-pkg-config.sh")
    set(ENV{CMAKE_PREFIX_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/pc-bar:/this/directory/should/not/exist/in/the/filesystem")
    set(ENV{PKG_CONFIG_PATH} "/baz")
endif()


find_package(PkgConfig)


if(NOT DEFINED CMAKE_SYSTEM_NAME
    OR (CMAKE_SYSTEM_NAME MATCHES "^(Linux|kFreeBSD|GNU)$"
    AND NOT CMAKE_CROSSCOMPILING))
  if(EXISTS "/etc/debian_version") # is this a debian system ?
    if(CMAKE_LIBRARY_ARCHITECTURE MATCHES "^(i386-linux-gnu|x86_64-linux-gnu)$")
      # Cannot create directories for all the existing architectures...
      set(expected_path "/baz:${CMAKE_CURRENT_SOURCE_DIR}/pc-foo/lib/${CMAKE_LIBRARY_ARCHITECTURE}/pkgconfig:${CMAKE_CURRENT_SOURCE_DIR}/pc-foo/lib/pkgconfig")
    else()
      set(expected_path "/baz:${CMAKE_CURRENT_SOURCE_DIR}/pc-foo/lib/pkgconfig")
    endif()
  else()
    # not debian, chech the FIND_LIBRARY_USE_LIB64_PATHS property
    get_property(uselib64 GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS)
    if(uselib64)
      set(expected_path "/baz:${CMAKE_CURRENT_SOURCE_DIR}/pc-foo/lib64/pkgconfig:${CMAKE_CURRENT_SOURCE_DIR}/pc-foo/lib/pkgconfig")
    endif()
  endif()
else()
  if(WIN32)
    set(expected_path "C:\\baz;${CMAKE_CURRENT_SOURCE_DIR}\\pc-foo\\lib\\pkgconfig")
  else()
    set(expected_path "/baz:${CMAKE_CURRENT_SOURCE_DIR}/pc-foo/lib/pkgconfig")
  endif()
endif()


pkg_check_modules(FOO "${expected_path}" NO_CMAKE_ENVIRONMENT_PATH)

if(NOT FOO_FOUND)
  message(FATAL_ERROR "Expected PKG_CONFIG_PATH: \"${expected_path}\".")
endif()
