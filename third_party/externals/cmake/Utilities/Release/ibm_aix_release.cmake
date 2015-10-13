set(CMAKE_RELEASE_DIRECTORY "/bench1/noibm34/CMakeReleaseDirectory")
set(PROCESSORS 64)
set(HOST "ibm-backend")
set(SCRIPT_NAME aix)
set(MAKE_PROGRAM "make")
set(CC "xlc_r")
set(CXX "xlC_r")
set(FC "xlf")
set(LDFLAGS "-Wl,-bmaxdata:0x80000000") # Push "Segmentation fault in extend_brk" over horizon
set(INITIAL_CACHE "
CMAKE_BUILD_TYPE:STRING=Release
CMAKE_SKIP_BOOTSTRAP_TEST:STRING=TRUE
")
get_filename_component(path "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(${path}/release_cmake.cmake)
