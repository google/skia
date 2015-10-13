file(MAKE_DIRECTORY "${bin_dir}")
message(STATUS "running bootstrap: ${bootstrap}")
execute_process(
  COMMAND ${bootstrap}
  WORKING_DIRECTORY "${bin_dir}"
  RESULT_VARIABLE result
  )
if(result)
  message(FATAL_ERROR "bootstrap failed: ${result}")
endif()
