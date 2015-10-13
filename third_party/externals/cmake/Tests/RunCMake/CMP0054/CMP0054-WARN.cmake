set(FOO "BAR")

if(NOT "FOO" STREQUAL "BAR")
  message(FATAL_ERROR "The given literals should match")
endif()
