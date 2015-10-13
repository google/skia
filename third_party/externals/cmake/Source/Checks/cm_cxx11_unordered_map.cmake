
if(CMAKE_CXX_STANDARD AND NOT DEFINED CMake_HAVE_CXX11_UNORDERED_MAP)
  message(STATUS "Checking if compiler supports C++11 unordered_map")
  try_compile(CMake_HAVE_CXX11_UNORDERED_MAP
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/cm_cxx11_unordered_map.cpp
    CMAKE_FLAGS -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
    OUTPUT_VARIABLE OUTPUT
    )
  if(CMake_HAVE_CXX11_UNORDERED_MAP)
    message(STATUS "Checking if compiler supports C++11 unordered_map - yes")
    file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
      "Determining if compiler supports C++11 unordered_map passed with the following output:\n"
      "${OUTPUT}\n"
      "\n"
      )
  else()
    message(STATUS "Checking if compiler supports C++11 unordered_map - no")
    file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
      "Determining if compiler supports C++11 unordered_map failed with the following output:\n"
      "${OUTPUT}\n"
      "\n"
      )
  endif()
endif()
