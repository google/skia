
enable_language(CXX)

cmake_policy(SET CMP0026 OLD)

add_library(somelib empty.cpp)
get_target_property(_loc somelib LOCATION_Debug)
