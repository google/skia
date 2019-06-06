# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# configure import libs
set(distribution_DIR  ${CMAKE_SOURCE_DIR}/../../../../../..)
set(libs_DIR ${CMAKE_SOURCE_DIR}/../../../libs)

# shared lib will also be tucked into APK and sent to target
# refer to app/build.gradle, jniLibs section for that purpose.
# ${ANDROID_ABI} is handy for our purpose here. Probably this ${ANDROID_ABI} is
# the most valuable thing of this sample, the rest are pretty much normal cmake
add_library(libskia SHARED IMPORTED)
set_target_properties(libskia PROPERTIES IMPORTED_LOCATION
        ${libs_DIR}/${ANDROID_ABI}/libskia.so)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
        skiajni

        # Sets the library as a shared library.
        SHARED

        # JNI generator files - these could be taken out, because they are not needed to run the code
        generator/generator.cpp
        generator/jnigenerator.cpp
        generator/skia_api_idl.cpp

        # Provides a relative path to your source file(s).
        sk_facade.cpp
        org.skia_jni.cpp
        native-lib.cpp)

target_include_directories(skiajni PRIVATE
        ${distribution_DIR}
        ${distribution_DIR}/include
        ${distribution_DIR}/third_party/skcms)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        skiajni

        libskia

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})