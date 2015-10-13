CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(jsoncpp)
ENABLE_TESTING()

OPTION(JSONCPP_WITH_TESTS "Compile and run JsonCpp test executables" ON)
OPTION(JSONCPP_WITH_POST_BUILD_UNITTEST "Automatically run unit-tests as a post build step" ON)
OPTION(JSONCPP_WITH_WARNING_AS_ERROR "Force compilation to fail if a warning occurs" OFF)

# Ensures that CMAKE_BUILD_TYPE is visible in cmake-gui on Unix
IF(NOT WIN32)
    IF(NOT CMAKE_BUILD_TYPE)
        SET(CMAKE_BUILD_TYPE Release CACHE STRING
            "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel Coverage."
            FORCE)
    ENDIF(NOT CMAKE_BUILD_TYPE)
ENDIF(NOT WIN32) 

# This ensures shared DLL are in the same dir as executable on Windows.
# Put all executables / libraries are in a project global directory.
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
    CACHE PATH "Single directory for all static libraries.")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
    CACHE PATH "Single directory for all dynamic libraries on Unix.")
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin
    CACHE PATH "Single directory for all executable and dynamic libraries on Windows.")
MARK_AS_ADVANCED( CMAKE_RUNTIME_OUTPUT_DIRECTORY CMAKE_LIBRARY_OUTPUT_DIRECTORY CMAKE_ARCHIVE_OUTPUT_DIRECTORY )

# Set variable named ${VAR_NAME} to value ${VALUE}
FUNCTION(set_using_dynamic_name VAR_NAME VALUE)
    SET( "${VAR_NAME}" "${VALUE}" PARENT_SCOPE)
ENDFUNCTION(set_using_dynamic_name) 

# Extract major, minor, patch and qualifier from version text
# Parse a version string "X.Y.Z[-qualifier]" and outputs
# version parts in ${OUPUT_PREFIX}_MAJOR, _MINOR, _PATCH, _QUALIFIER.
# If parse succed then ${OUPUT_PREFIX}_FOUND is TRUE.
MACRO(jsoncpp_parse_version VERSION_TEXT OUPUT_PREFIX)
    SET(VERSION_REGEX "[0-9]+\\.[0-9]+\\.[0-9]+(-[a-zA-Z0-9_]+)?")
    IF( ${VERSION_TEXT} MATCHES ${VERSION_REGEX} )
        STRING(REGEX MATCHALL "[0-9]+|-([A-Za-z0-9_]+)" VERSION_PARTS ${VERSION_TEXT})
        list(APPEND VERSION_PARTS "") # empty qualifier to handle no qualifier case
        LIST(GET VERSION_PARTS 0 ${OUPUT_PREFIX}_MAJOR)
        LIST(GET VERSION_PARTS 1 ${OUPUT_PREFIX}_MINOR)
        LIST(GET VERSION_PARTS 2 ${OUPUT_PREFIX}_PATCH)
        LIST(GET VERSION_PARTS 3 ${OUPUT_PREFIX}_QUALIFIER)
        set_using_dynamic_name( "${OUPUT_PREFIX}_FOUND" TRUE ) 
    ELSE( ${VERSION_TEXT} MATCHES ${VERSION_REGEX} )
        set_using_dynamic_name( "${OUPUT_PREFIX}_FOUND" FALSE ) 
    ENDIF( ${VERSION_TEXT} MATCHES ${VERSION_REGEX} )
ENDMACRO(jsoncpp_parse_version)

# Read out version from "version" file
FILE(STRINGS "version" JSONCPP_VERSION)

jsoncpp_parse_version( ${JSONCPP_VERSION} JSONCPP_VERSION )
IF(NOT JSONCPP_VERSION_FOUND)
    MESSAGE(FATAL_ERROR "Failed to parse version string properly. Expect X.Y.Z[-qualifier]")
ENDIF(NOT JSONCPP_VERSION_FOUND)

MESSAGE(STATUS "JsonCpp Version: ${JSONCPP_VERSION_MAJOR}.${JSONCPP_VERSION_MINOR}.${JSONCPP_VERSION_PATCH}${JSONCPP_VERSION_QUALIFIER}")
# File version.h is only regenerated on CMake configure step
CONFIGURE_FILE( "${PROJECT_SOURCE_DIR}/src/lib_json/version.h.in"
                "${PROJECT_SOURCE_DIR}/include/json/version.h" )

macro(UseCompilationWarningAsError)
	if ( MSVC )
        # Only enabled in debug because some old versions of VS STL generate
        # warnings when compiled in release configuration.
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /WX ")
	endif( MSVC )
endmacro() 

# Include our configuration header
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/include )

if ( MSVC )
    # Only enabled in debug because some old versions of VS STL generate
    # unreachable code warning when compiled in release configuration.
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W4 ")
endif( MSVC )

IF(JSONCPP_WITH_WARNING_AS_ERROR)
    UseCompilationWarningAsError()
ENDIF(JSONCPP_WITH_WARNING_AS_ERROR)

# Build the different applications
ADD_SUBDIRECTORY( src )

#install the includes
ADD_SUBDIRECTORY( include )
