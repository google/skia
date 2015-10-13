cmake_minimum_required(VERSION 2.8)

project(gtkmm CXX)

find_package(GTK2 COMPONENTS gtk gtkmm REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(gtkmm-target WIN32 main.cpp helloworld.cpp helloworld.h)
target_link_libraries(gtkmm-target GTK2::gtkmm)

add_executable(gtkmm-all-libs WIN32 main.cpp helloworld.cpp helloworld.h)
target_link_libraries(gtkmm-all-libs ${GTK2_LIBRARIES})
target_include_directories(gtkmm-all-libs PRIVATE ${GTK2_INCLUDE_DIRS})
