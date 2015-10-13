set(CMAKE_RELEASE_DIRECTORY "c:/cygwin/home/dashboard/CMakeReleaseCygwin")
set(PROCESSORS 9)
set(MAKE_PROGRAM "make")
set(MAKE "${MAKE_PROGRAM} -j8")
set(HOST dash2win64)
set(CPACK_BINARY_GENERATORS "CygwinBinary")
set(CPACK_SOURCE_GENERATORS "CygwinSource")
set(MAKE_PROGRAM "make")
set(INITIAL_CACHE "CMAKE_BUILD_TYPE:STRING=Release
CMAKE_Fortran_COMPILER_FULLPATH:FILEPATH=FALSE
CTEST_TEST_TIMEOUT:STRING=7200
DART_TESTING_TIMEOUT:STRING=7200
SPHINX_HTML:BOOL=ON
SPHINX_MAN:BOOL=ON
")
set(CXX g++)
set(CC  gcc)
set(SCRIPT_NAME dash2win64cygwin)
set(GIT_EXTRA "git config core.autocrlf false")
get_filename_component(path "${CMAKE_CURRENT_LIST_FILE}" PATH)

# WARNING: Temporary fix!! This exclusion of the ExternalProject test
# is temporary until we can set up a new cygwin build machine.
# It only fails because of cygwin/non-cygwin "svn" mismatches in this
# particular environment. This is less than ideal, but at least it
# allows us to produce cygwin builds in the short term.
set(EXTRA_CTEST_ARGS "-E ExternalProject")

include(${path}/release_cmake.cmake)
