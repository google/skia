cmake_minimum_required(VERSION 3.1)

project(QtAutogen)

# Tell find_package(Qt5) where to find Qt.
if(QT_QMAKE_EXECUTABLE)
  get_filename_component(Qt_BIN_DIR "${QT_QMAKE_EXECUTABLE}" PATH)
  get_filename_component(Qt_PREFIX_DIR "${Qt_BIN_DIR}" PATH)
  set(CMAKE_PREFIX_PATH ${Qt_PREFIX_DIR})
endif()

if (QT_TEST_VERSION STREQUAL 4)
  find_package(Qt4 REQUIRED)

  # Include this directory before using the UseQt4 file.
  add_subdirectory(defines_test)

  include(UseQt4)

  set(QT_QTCORE_TARGET Qt4::QtCore)

  macro(qtx_wrap_cpp)
    qt4_wrap_cpp(${ARGN})
  endmacro()

else()
  if (NOT QT_TEST_VERSION STREQUAL 5)
    message(SEND_ERROR "Invalid Qt version specified.")
  endif()
  find_package(Qt5Widgets REQUIRED)

  set(QT_QTCORE_TARGET Qt5::Core)

  include_directories(${Qt5Widgets_INCLUDE_DIRS})
  set(QT_LIBRARIES Qt5::Widgets)

  if(Qt5_POSITION_INDEPENDENT_CODE)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
  endif()

  macro(qtx_wrap_cpp)
    qt5_wrap_cpp(${ARGN})
  endmacro()

endif()

add_executable(rcconly rcconly.cpp second_resource.qrc)
set_property(TARGET rcconly PROPERTY AUTORCC ON)
target_link_libraries(rcconly ${QT_QTCORE_TARGET})

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_definitions(-DFOO -DSomeDefine="Barx")

# enable relaxed mode so automoc can handle all the special cases:
set(CMAKE_AUTOMOC_RELAXED_MODE TRUE)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# create an executable and two library targets, each requiring automoc:
add_library(codeeditorLib STATIC codeeditor.cpp)

add_library(privateSlot OBJECT private_slot.cpp)

configure_file(generated_resource.qrc.in generated_resource.qrc @ONLY)
add_custom_command(
  OUTPUT generated.txt
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/generated.txt.in" "${CMAKE_CURRENT_BINARY_DIR}/generated.txt"
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/generated.txt.in"
  )

add_custom_target(generate_moc_input
  DEPENDS generated.txt
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/myinterface.h.in" "${CMAKE_CURRENT_BINARY_DIR}"
  COMMAND ${CMAKE_COMMAND} -E rename "${CMAKE_CURRENT_BINARY_DIR}/myinterface.h.in" "${CMAKE_CURRENT_BINARY_DIR}/myinterface.h"
)

add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/myotherinterface.h"
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/myotherinterface.h.in" "${CMAKE_CURRENT_BINARY_DIR}/myotherinterface.h"
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/myotherinterface.h.in"
)

if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_GENERATOR STREQUAL Ninja)
  set(debug_srcs "$<$<CONFIG:Debug>:debug_class.cpp>" $<$<CONFIG:Debug>:debug_resource.qrc>)
  set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS $<$<CONFIG:Debug>:TEST_DEBUG_CLASS>)
endif()

# The -no-protection option disables the generation of include guards.  Verify
# that setting the source file property has an effect by using this and
# issue an error in the preprocessor in calwidget.cpp if the include guard
# is defined.
set_source_files_properties(calwidget.ui PROPERTIES AUTOUIC_OPTIONS "-no-protection")

add_executable(QtAutogen main.cpp calwidget.cpp second_widget.cpp foo.cpp blub.cpp bar.cpp abc.cpp
               multiplewidgets.cpp
               xyz.cpp yaf.cpp gadget.cpp $<TARGET_OBJECTS:privateSlot>
               test.qrc second_resource.qrc resourcetester.cpp generated.cpp ${debug_srcs}
               ${CMAKE_CURRENT_BINARY_DIR}/generated_resource.qrc
)
set_property(TARGET QtAutogen APPEND PROPERTY AUTOGEN_TARGET_DEPENDS generate_moc_input "${CMAKE_CURRENT_BINARY_DIR}/myotherinterface.h")

set_target_properties(QtAutogen codeeditorLib privateSlot PROPERTIES AUTOMOC TRUE)

include(GenerateExportHeader)
# The order is relevant here. B depends on A, and B headers depend on A
# headers both subdirectories use CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE and we
# test that CMAKE_AUTOMOC successfully reads the include directories
# for the build interface from those targets. There has previously been
# a bug where caching of the include directories happened before
# extracting the includes to pass to moc.
add_subdirectory(Bdir)
add_subdirectory(Adir)
add_library(libC SHARED libC.cpp)
set_target_properties(libC PROPERTIES AUTOMOC TRUE)
generate_export_header(libC)
target_link_libraries(libC LINK_PUBLIC libB)

target_link_libraries(QtAutogen codeeditorLib ${QT_LIBRARIES} libC)

# Add not_generated_file.qrc to the source list to get the file-level
# dependency, but don't generate a c++ file from it.  Disable the AUTORCC
# feature for this target.  This tests that qrc files in the sources don't
# have an effect on generation if AUTORCC is off.
add_library(empty STATIC empty.cpp not_generated_file.qrc)
set_target_properties(empty PROPERTIES AUTORCC OFF)

set_target_properties(empty PROPERTIES AUTOMOC TRUE)
target_link_libraries(empty no_link_language)
add_library(no_link_language STATIC empty.h)
set_target_properties(no_link_language PROPERTIES AUTOMOC TRUE)

qtx_wrap_cpp(uicOnlyMoc sub/uiconly.h)
add_executable(uiconly sub/uiconly.cpp ${uicOnlyMoc})
target_link_libraries(uiconly ${QT_LIBRARIES})

try_compile(RCC_DEPENDS
  "${CMAKE_CURRENT_BINARY_DIR}/autorcc_depends"
  "${CMAKE_CURRENT_SOURCE_DIR}/autorcc_depends"
  autorcc_depends
  CMAKE_FLAGS "-DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}" "-DQT_TEST_VERSION=${QT_TEST_VERSION}"
              "-DCMAKE_PREFIX_PATH=${Qt_PREFIX_DIR}"
  OUTPUT_VARIABLE output
)
if (NOT RCC_DEPENDS)
  message(SEND_ERROR "Initial build of autorcc_depends failed. Output: ${output}")
endif()

file(STRINGS "${CMAKE_CURRENT_BINARY_DIR}/autorcc_depends/info_file.txt" qrc_files)

list(GET qrc_files 0 qrc_file1)

set(timeformat "%Y%j%H%M%S")

file(TIMESTAMP "${qrc_file1}" file1_before "${timeformat}")

execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep 1) # Ensure that the timestamp will change.
execute_process(COMMAND "${CMAKE_COMMAND}" -E touch "${CMAKE_CURRENT_BINARY_DIR}/autorcc_depends/res1/input.txt")

execute_process(COMMAND "${CMAKE_COMMAND}" --build .
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/autorcc_depends"
)

file(TIMESTAMP "${qrc_file1}" file1_step1 "${timeformat}")

if (NOT file1_step1 GREATER file1_before)
  message(SEND_ERROR "file1 (${qrc_file1}) should have changed in the first step!")
endif()
