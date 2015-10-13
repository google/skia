project(hello Java)

cmake_minimum_required (VERSION 2.6)
set(CMAKE_VERBOSE_MAKEFILE 1)

find_package(Java COMPONENTS Development)
include (UseJava)

add_jar(hello A.java HelloWorld.java)
