cmake_minimum_required(VERSION 3.10)
project(skia CXX)
set(CMAKE_CXX_STANDARD 17)

add_library(skcms "third_party/skcms/skcms.cc")
target_include_directories(skcms PRIVATE "include/third_party/skcms")

file(GLOB_RECURSE skia_sources CONFIGURE_DEPENDS
     src/codec/*.cpp
     src/core/*.cpp
     src/effects/*.cpp
     src/gpu/*.cpp
     src/image/*.cpp
     src/images/*.cpp
     src/lazy/*.cpp
     src/opts/*.cpp
     src/pathops/*.cpp
     src/pdf/*.cpp
     src/ports/*.cpp
     src/sfnt/*.cpp
     src/shaders/*.cpp
     src/sksl/*.cpp
    #src/svg/*.cpp
     src/utils/*.cpp
     third_party/gif/*.cpp)

list(FILTER skia_sources EXCLUDE REGEX SkFontMgr_custom.*.cpp)
list(FILTER skia_sources EXCLUDE REGEX SkFontMgr_empty_factory.cpp)
list(FILTER skia_sources EXCLUDE REGEX SkImageGeneratorWIC.cpp)
list(FILTER skia_sources EXCLUDE REGEX SkImageGenerator_none.cpp)
list(FILTER skia_sources EXCLUDE REGEX SkMemory_mozalloc.cpp)
list(FILTER skia_sources EXCLUDE REGEX SkRawCodec.cpp)
list(FILTER skia_sources EXCLUDE REGEX SkTLS_none.cpp)
list(FILTER skia_sources EXCLUDE REGEX SkWebpCodec.cpp)
list(FILTER skia_sources EXCLUDE REGEX SkWuffsCodec.cpp)
list(FILTER skia_sources EXCLUDE REGEX android)
list(FILTER skia_sources EXCLUDE REGEX dawn)
list(FILTER skia_sources EXCLUDE REGEX egl)
list(FILTER skia_sources EXCLUDE REGEX fuchsia)
list(FILTER skia_sources EXCLUDE REGEX glfw)
list(FILTER skia_sources EXCLUDE REGEX glx)
list(FILTER skia_sources EXCLUDE REGEX src/sksl/SkSLMain.cpp)
list(FILTER skia_sources EXCLUDE REGEX src/sksl/lex/Main.cpp)
list(FILTER skia_sources EXCLUDE REGEX vk)

find_package(Fontconfig)
if (NOT Fontconfig_FOUND)
    list(FILTER skia_sources EXCLUDE REGEX [fF]ont[cC]onfig)
endif()

find_package(Freetype)
if (NOT Freetype_FOUND)
    list(FILTER skia_sources EXCLUDE REGEX FreeType)
endif()

find_package(PNG)
if (NOT PNG_FOUND)
    list(FILTER skia_sources EXCLUDE REGEX SkIcoCodec.cpp)
    list(FILTER skia_sources EXCLUDE REGEX SkPngCodec.cpp)
endif()

set(LibJpegTurbo_FOUND FALSE) #find_package(LibJpegTurbo)
if (NOT LibJpegTurbo_FOUND)
    list(FILTER skia_sources EXCLUDE REGEX src/codec/SkJ[pP][eE][gG])
    list(FILTER skia_sources EXCLUDE REGEX src/images/SkJ[pP][eE][gG])
endif()

find_package(Lua)
if (NOT Lua_FOUND)
    list(FILTER skia_sources EXCLUDE REGEX Lua)
endif()

find_package(ZLIB)
if (ZLIB_FOUND)
    list(FILTER skia_sources EXCLUDE REGEX SkDocument_PDF_None.cpp)
else()
    list(FILTER skia_soures EXCLUDE REGEX pdf)
endif()

add_library(skia ${skia_sources})
target_include_directories(skia PUBLIC ".")
target_link_libraries(skia PRIVATE skcms ${ZLIB_LIBRARIES})

if (APPLE)
    target_link_libraries(skia PRIVATE "-framework CoreFoundation"
                                       "-framework CoreGraphics"
                                       "-framework CoreText"
                                       "-framework CoreServices"
                                       "-framework ImageIO")
endif()

