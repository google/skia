# Try to identify the current development source version.
set(CMake_VERSION_SOURCE "")
if(EXISTS ${CMake_SOURCE_DIR}/.git/HEAD)
  find_program(GIT_EXECUTABLE NAMES git git.cmd)
  mark_as_advanced(GIT_EXECUTABLE)
  if(GIT_EXECUTABLE)
    execute_process(
      COMMAND ${GIT_EXECUTABLE} rev-parse --verify -q --short=4 HEAD
      OUTPUT_VARIABLE head
      OUTPUT_STRIP_TRAILING_WHITESPACE
      WORKING_DIRECTORY ${CMake_SOURCE_DIR}
      )
    if(head)
      set(CMake_VERSION_SOURCE "g${head}")
      execute_process(
        COMMAND ${GIT_EXECUTABLE} update-index -q --refresh
        WORKING_DIRECTORY ${CMake_SOURCE_DIR}
        )
      execute_process(
        COMMAND ${GIT_EXECUTABLE} diff-index --name-only HEAD --
        OUTPUT_VARIABLE dirty
        OUTPUT_STRIP_TRAILING_WHITESPACE
        WORKING_DIRECTORY ${CMake_SOURCE_DIR}
        )
      if(dirty)
        set(CMake_VERSION_SOURCE "${CMake_VERSION_SOURCE}-dirty")
      endif()
    endif()
  endif()
elseif(EXISTS ${CMake_SOURCE_DIR}/CVS/Repository)
  file(READ ${CMake_SOURCE_DIR}/CVS/Repository repo)
  set(branch "")
  if("${repo}" MATCHES "\\.git/([^\r\n]*)")
    set(branch "${CMAKE_MATCH_1}")
  endif()
  set(CMake_VERSION_SOURCE "cvs${branch}")
endif()
