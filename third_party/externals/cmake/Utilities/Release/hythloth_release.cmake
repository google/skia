set(PROCESSORS 2)
set(HOST hythloth)
set(MAKE_PROGRAM "make")
set(MAKE "${MAKE_PROGRAM} -j2")
set(INITIAL_CACHE "
CMAKE_BUILD_TYPE:STRING=Release
CMAKE_SKIP_BOOTSTRAP_TEST:STRING=TRUE
")
get_filename_component(path "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(${path}/release_cmake.cmake)
