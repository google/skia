#.rst:
# TestBigEndian
# -------------
#
# Define macro to determine endian type
#
# Check if the system is big endian or little endian
#
# ::
#
#   TEST_BIG_ENDIAN(VARIABLE)
#   VARIABLE - variable to store the result to

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

macro(TEST_BIG_ENDIAN VARIABLE)
  if(NOT DEFINED HAVE_${VARIABLE})
    message(STATUS "Check if the system is big endian")
    message(STATUS "Searching 16 bit integer")

    include(CheckTypeSize)

    CHECK_TYPE_SIZE("unsigned short" CMAKE_SIZEOF_UNSIGNED_SHORT)
    if(CMAKE_SIZEOF_UNSIGNED_SHORT EQUAL 2)
      message(STATUS "Using unsigned short")
      set(CMAKE_16BIT_TYPE "unsigned short")
    else()
      CHECK_TYPE_SIZE("unsigned int"   CMAKE_SIZEOF_UNSIGNED_INT)
      if(CMAKE_SIZEOF_UNSIGNED_INT)
        message(STATUS "Using unsigned int")
        set(CMAKE_16BIT_TYPE "unsigned int")

      else()

        CHECK_TYPE_SIZE("unsigned long"  CMAKE_SIZEOF_UNSIGNED_LONG)
        if(CMAKE_SIZEOF_UNSIGNED_LONG)
          message(STATUS "Using unsigned long")
          set(CMAKE_16BIT_TYPE "unsigned long")
        else()
          message(FATAL_ERROR "no suitable type found")
        endif()

      endif()

    endif()


    configure_file("${CMAKE_ROOT}/Modules/TestEndianess.c.in"
                   "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/TestEndianess.c"
                   @ONLY)

     file(READ "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/TestEndianess.c"
          TEST_ENDIANESS_FILE_CONTENT)

     try_compile(HAVE_${VARIABLE}
      "${CMAKE_BINARY_DIR}"
      "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/TestEndianess.c"
      OUTPUT_VARIABLE OUTPUT
      COPY_FILE "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/TestEndianess.bin" )

      if(HAVE_${VARIABLE})

        file(STRINGS "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/TestEndianess.bin"
            CMAKE_TEST_ENDIANESS_STRINGS_LE LIMIT_COUNT 1 REGEX "THIS IS LITTLE ENDIAN")

        file(STRINGS "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/TestEndianess.bin"
            CMAKE_TEST_ENDIANESS_STRINGS_BE LIMIT_COUNT 1 REGEX "THIS IS BIG ENDIAN")

        # on mac, if there are universal binaries built both will be true
        # return the result depending on the machine on which cmake runs
        if(CMAKE_TEST_ENDIANESS_STRINGS_BE  AND  CMAKE_TEST_ENDIANESS_STRINGS_LE)
          if(CMAKE_SYSTEM_PROCESSOR MATCHES powerpc)
            set(CMAKE_TEST_ENDIANESS_STRINGS_BE TRUE)
            set(CMAKE_TEST_ENDIANESS_STRINGS_LE FALSE)
          else()
            set(CMAKE_TEST_ENDIANESS_STRINGS_BE FALSE)
            set(CMAKE_TEST_ENDIANESS_STRINGS_LE TRUE)
          endif()
          message(STATUS "TEST_BIG_ENDIAN found different results, consider setting CMAKE_OSX_ARCHITECTURES or CMAKE_TRY_COMPILE_OSX_ARCHITECTURES to one or no architecture !")
        endif()

        if(CMAKE_TEST_ENDIANESS_STRINGS_LE)
          set(${VARIABLE} 0 CACHE INTERNAL "Result of TEST_BIG_ENDIAN" FORCE)
          message(STATUS "Check if the system is big endian - little endian")
        endif()

        if(CMAKE_TEST_ENDIANESS_STRINGS_BE)
          set(${VARIABLE} 1 CACHE INTERNAL "Result of TEST_BIG_ENDIAN" FORCE)
          message(STATUS "Check if the system is big endian - big endian")
        endif()

        if(NOT CMAKE_TEST_ENDIANESS_STRINGS_BE  AND  NOT CMAKE_TEST_ENDIANESS_STRINGS_LE)
          message(SEND_ERROR "TEST_BIG_ENDIAN found no result!")
        endif()

        file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
          "Determining if the system is big endian passed with the following output:\n${OUTPUT}\nTestEndianess.c:\n${TEST_ENDIANESS_FILE_CONTENT}\n\n")

      else()
        message(STATUS "Check if the system is big endian - failed")
        file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
          "Determining if the system is big endian failed with the following output:\n${OUTPUT}\nTestEndianess.c:\n${TEST_ENDIANESS_FILE_CONTENT}\n\n")
        set(${VARIABLE})
      endif()
  endif()
endmacro()


