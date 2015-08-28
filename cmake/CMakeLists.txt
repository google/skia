# Boilerplate.
cmake_minimum_required (VERSION 3.3)   # The latest version as I write this.  May be too strict.
project (skimake)
set (CMAKE_CXX_STANDARD 11)

# Default to Release mode.  We're mainly targeting Skia users, not Skia developers.
if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE Release)
endif ()

# To first approximation, the Skia library comprises all .cpp files under src/.
file (GLOB_RECURSE srcs ../src/*.cpp)

function (find_include_dirs out)
    file (GLOB_RECURSE headers ${ARGV})
    foreach (path ${headers})
        get_filename_component (dir ${path} PATH)
        list (APPEND include_dirs ${dir})
    endforeach()
    list (REMOVE_DUPLICATES include_dirs)
    set (${out} ${include_dirs} PARENT_SCOPE)
endfunction()

# We need src/ directories and include/private on our path when building Skia.
# Users should be able to use Skia with only include/ directories that are not include/private.
find_include_dirs(private_includes ../src/*.h ../include/private/*.h)
find_include_dirs(public_includes ../include/*.h)
list (REMOVE_ITEM public_includes ${private_includes})  # Easiest way to exclude private.

# These guys are third_party but provided by a Skia checkout.
list (APPEND srcs             ../third_party/etc1/etc1.cpp ../third_party/ktx/ktx.cpp)
list (APPEND private_includes ../third_party/etc1          ../third_party/ktx)

function (remove_srcs)
    file (GLOB_RECURSE to_remove ${ARGV})
    list (REMOVE_ITEM srcs ${to_remove})
    set (srcs ${srcs} PARENT_SCOPE)
endfunction()

# This file is empty and is only used to trick GYP.
remove_srcs (../src/core/SkForceCPlusPlusLinking.cpp)
# This file forces linking for all our supported image decoders.  We're more fine-grained.
remove_srcs (../src/images/SkForceLinking.cpp)

# Skia sure ships a lot of code no one uses.
remove_srcs (../src/animator/* ../src/*nacl* ../src/svg/* ../src/views/* ../src/xml/*)

# Some files only contain code in Debug mode.  This quiets down some linker warnings.
if (NOT CMAKE_BUILD_TYPE EQUAL Debug)
    remove_srcs (../src/core/SkDebug.cpp ../src/utils/SkDumpCanvas.cpp)
endif()

# Remove OS-specific source files.
if (NOT WIN32)
    remove_srcs(../src/utils/win/* ../src/*_win*.cpp ../src/*xps* ../src/gpu/gl/angle/* ../src/*WIC* ../src/*DWRITE*)
endif()
if (NOT LINUX)
    remove_srcs(../src/*linux* ../src/*FreeType* ../src/*FontConfig* ../src/*FontMgr_custom*)
endif()
if (NOT ANDROID)
    remove_srcs(../src/*android* ../src/*hwui*)
endif()

# Remove processor-specific source files.
if (NOT CMAKE_SYSTEM_PROCESSOR EQUAL ARM)
    remove_srcs(../src/*arm* ../src/*ARM* ../src/*neon* ../src/*NEON*)
endif()
if (NOT CMAKE_SYSTEM_PROCESSOR EQUAL MIPS)
    remove_srcs(../src/*mips* ../src/*MIPS*)
endif()

# Make our ports choices.
remove_srcs(
    ../src/*moz*   # We're probably not Mozilla.
    ../src/gpu/GrContextFactory.cpp                  # For internal testing only.
    ../src/gpu/gl/GrGLCreateNativeInterface_none.cpp
    ../src/gpu/gl/GrGLDefaultInterface_none.cpp
    ../src/gpu/gl/SkCreatePlatformGLContext*.cpp     # For internal testing only.
    ../src/gpu/gl/command_buffer/*
    ../src/gpu/gl/egl/*
    ../src/gpu/gl/glx/*
    ../src/gpu/gl/iOS/*
    ../src/gpu/gl/mesa/*
    ../src/images/SkImageDecoder_FactoryDefault.cpp
    ../src/opts/SkBitmapProcState_opts_none.cpp
    ../src/opts/SkBlitMask_opts_none.cpp
    ../src/opts/SkBlitRow_opts_none.cpp
    ../src/ports/SkFontMgr_empty_factory.cpp
    ../src/ports/SkGlobalInitialization_chromium.cpp
    ../src/ports/SkImageDecoder_empty.cpp
    ../src/ports/SkImageGenerator_none.cpp
    ../src/ports/SkTLS_none.cpp
    ../src/utils/SkThreadUtils_pthread_other.cpp
    )

remove_srcs(../src/codec/*)  # TODO: Requires Chromium's libjpeg-turbo, and incompatible giflib.

# Certain files must be compiled with support for SSSE3 or SSE4.1 intrinsics.
file (GLOB_RECURSE ssse3_srcs ../src/*ssse3*.cpp ../src/*SSSE3*.cpp)
file (GLOB_RECURSE sse41_srcs ../src/*sse4*.cpp ../src/*SSE4*.cpp)
set_source_files_properties(${ssse3_srcs} PROPERTIES COMPILE_FLAGS -mssse3)
set_source_files_properties(${sse41_srcs} PROPERTIES COMPILE_FLAGS -msse4.1)

# Detect our optional dependencies.
# If we can't find them, don't build the parts of Skia that use them.

find_package (GIF)
find_package (JPEG)
find_package (LUA)
find_package (PNG)
find_package (ZLIB)
# No find_package for libwebp as far as I can tell, so simulate it here.
find_path (WEBP_INCLUDE_DIRS "webp/decode.h")
find_library (WEBP_LIBRARIES webp)

if (NOT GIF_FOUND)
    remove_srcs(../src/images/*gif*)
endif()
if (NOT JPEG_FOUND)
    remove_srcs(../src/images/*jpeg*)
endif()
if (NOT LUA_FOUND)
    remove_srcs(../src/utils/*lua*)
endif()
if (NOT PNG_FOUND)
    remove_srcs(../src/images/*png*)
endif()
if (NOT ZLIB_FOUND)
    remove_srcs(../src/pdf/*.cpp ../src/doc/SkDocument_PDF.cpp)
else()
    remove_srcs(../src/doc/SkDocument_PDF_None.cpp)
endif()
if (NOT WEBP_INCLUDE_DIRS OR NOT WEBP_LIBRARIES)
    remove_srcs(../src/images/*webp*)
endif()

if (APPLE)
    find_library(APPLICATION_SERVICES_FRAMEWORK ApplicationServices)
endif()

# This is our main output, libskia.so.
# We mostly build an .so here because it helps test we've linked everything,
# not so much that we think Skia is a good candidate to ship as a shared library.
add_library (skia SHARED ${srcs})

target_compile_definitions(skia
    PUBLIC
    PRIVATE -DSKIA_DLL)

target_include_directories(skia
    PUBLIC  ${public_includes}
    PRIVATE ${private_includes}
            ${GIF_INCLUDE_DIRS}
            ${JPEG_INCLUDE_DIRS}
            ${LUA_INCLUDE_DIRS}
            ${PNG_INCLUDE_DIRS}
            ${WEBP_INCLUDE_DIRS}
            ${ZLIB_INCLUDE_DIRS})

target_link_libraries(skia
    PUBLIC
    PRIVATE ${APPLICATION_SERVICES_FRAMEWORK}
            ${GIF_LIBRARIES}
            ${JPEG_LIBRARIES}
            ${LUA_LIBRARIES}
            ${PNG_LIBRARIES}
            ${WEBP_LIBRARIES}
            ${ZLIB_LIBRARIES})

set_target_properties(skia PROPERTIES
    COMPILE_FLAGS "-fno-exceptions -fno-rtti -Wno-deprecated-declarations"
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN true)

# Now build a simple example app that uses Skia via libskia.so.
find_package(OpenGL REQUIRED)
add_executable(example example.cpp)
target_link_libraries(example skia ${OPENGL_LIBRARIES})
