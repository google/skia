
#=============================================================================
# Copyright 2013 Stephen Kelly <steveire@gmail.com>
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

function(cmake_determine_compile_features lang)

  if(lang STREQUAL C AND COMMAND cmake_record_c_compile_features)
    message(STATUS "Detecting ${lang} compile features")

    set(CMAKE_C90_COMPILE_FEATURES)
    set(CMAKE_C99_COMPILE_FEATURES)
    set(CMAKE_C11_COMPILE_FEATURES)

    include("${CMAKE_ROOT}/Modules/Internal/FeatureTesting.cmake")

    cmake_record_c_compile_features()

    if(NOT _result EQUAL 0)
      message(STATUS "Detecting ${lang} compile features - failed")
      return()
    endif()

    if (CMAKE_C99_COMPILE_FEATURES AND CMAKE_C11_COMPILE_FEATURES)
      list(REMOVE_ITEM CMAKE_C11_COMPILE_FEATURES ${CMAKE_C99_COMPILE_FEATURES})
    endif()
    if (CMAKE_C90_COMPILE_FEATURES AND CMAKE_C99_COMPILE_FEATURES)
      list(REMOVE_ITEM CMAKE_C99_COMPILE_FEATURES ${CMAKE_C90_COMPILE_FEATURES})
    endif()

    if(NOT CMAKE_C_COMPILE_FEATURES)
      set(CMAKE_C_COMPILE_FEATURES
        ${CMAKE_C90_COMPILE_FEATURES}
        ${CMAKE_C99_COMPILE_FEATURES}
        ${CMAKE_C11_COMPILE_FEATURES}
      )
    endif()

    set(CMAKE_C_COMPILE_FEATURES ${CMAKE_C_COMPILE_FEATURES} PARENT_SCOPE)
    set(CMAKE_C90_COMPILE_FEATURES ${CMAKE_C90_COMPILE_FEATURES} PARENT_SCOPE)
    set(CMAKE_C99_COMPILE_FEATURES ${CMAKE_C99_COMPILE_FEATURES} PARENT_SCOPE)
    set(CMAKE_C11_COMPILE_FEATURES ${CMAKE_C11_COMPILE_FEATURES} PARENT_SCOPE)

    message(STATUS "Detecting ${lang} compile features - done")

  elseif(lang STREQUAL CXX AND COMMAND cmake_record_cxx_compile_features)
    message(STATUS "Detecting ${lang} compile features")

    set(CMAKE_CXX98_COMPILE_FEATURES)
    set(CMAKE_CXX11_COMPILE_FEATURES)
    set(CMAKE_CXX14_COMPILE_FEATURES)

    include("${CMAKE_ROOT}/Modules/Internal/FeatureTesting.cmake")

    cmake_record_cxx_compile_features()

    if(NOT _result EQUAL 0)
      message(STATUS "Detecting ${lang} compile features - failed")
      return()
    endif()

    if (CMAKE_CXX11_COMPILE_FEATURES AND CMAKE_CXX14_COMPILE_FEATURES)
      list(REMOVE_ITEM CMAKE_CXX14_COMPILE_FEATURES ${CMAKE_CXX11_COMPILE_FEATURES})
    endif()
    if (CMAKE_CXX98_COMPILE_FEATURES AND CMAKE_CXX11_COMPILE_FEATURES)
      list(REMOVE_ITEM CMAKE_CXX11_COMPILE_FEATURES ${CMAKE_CXX98_COMPILE_FEATURES})
    endif()

    if(NOT CMAKE_CXX_COMPILE_FEATURES)
      set(CMAKE_CXX_COMPILE_FEATURES
        ${CMAKE_CXX98_COMPILE_FEATURES}
        ${CMAKE_CXX11_COMPILE_FEATURES}
        ${CMAKE_CXX14_COMPILE_FEATURES}
      )
    endif()

    set(CMAKE_CXX_COMPILE_FEATURES ${CMAKE_CXX_COMPILE_FEATURES} PARENT_SCOPE)
    set(CMAKE_CXX98_COMPILE_FEATURES ${CMAKE_CXX98_COMPILE_FEATURES} PARENT_SCOPE)
    set(CMAKE_CXX11_COMPILE_FEATURES ${CMAKE_CXX11_COMPILE_FEATURES} PARENT_SCOPE)
    set(CMAKE_CXX14_COMPILE_FEATURES ${CMAKE_CXX14_COMPILE_FEATURES} PARENT_SCOPE)

    message(STATUS "Detecting ${lang} compile features - done")
  endif()

endfunction()
