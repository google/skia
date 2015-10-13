#=============================================================================
# CMake - Cross Platform Makefile Generator
# Copyright 2000-2009 Kitware, Inc., Insight Software Consortium
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================

set( CURSES_SRCS
              CursesDialog/cmCursesOptionsWidget
              CursesDialog/cmCursesBoolWidget
              CursesDialog/cmCursesCacheEntryComposite
              CursesDialog/cmCursesDummyWidget
              CursesDialog/cmCursesFilePathWidget
              CursesDialog/cmCursesForm
              CursesDialog/cmCursesLabelWidget
              CursesDialog/cmCursesLongMessageForm
              CursesDialog/cmCursesMainForm
              CursesDialog/cmCursesPathWidget
              CursesDialog/cmCursesStringWidget
              CursesDialog/cmCursesWidget
              CursesDialog/ccmake
   )

if( NOT CMAKE_USE_SYSTEM_FORM )
  include_directories(${CMake_SOURCE_DIR}/Source/CursesDialog/form
                      ${CMake_BINARY_DIR}/Source/CursesDialog/form)
endif()
include_directories(${CURSES_INCLUDE_PATH})


add_executable(ccmake ${CURSES_SRCS} )
target_link_libraries(ccmake CMakeLib)
if(CMAKE_USE_SYSTEM_FORM)
  target_link_libraries(ccmake
    ${CURSES_FORM_LIBRARY}
    ${CURSES_LIBRARY}
  )
  if(CURSES_EXTRA_LIBRARY)
    target_link_libraries(ccmake ${CURSES_EXTRA_LIBRARY})
  endif()
else()
  target_link_libraries(ccmake cmForm)
endif()

install(TARGETS ccmake DESTINATION bin)
