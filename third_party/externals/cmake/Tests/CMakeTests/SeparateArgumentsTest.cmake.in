set(old_out "a b  c")
separate_arguments(old_out)
set(old_exp "a;b;;c")

set(unix_cmd "a \"b c\" 'd e' \";\" \\ \\'\\\" '\\'' \"\\\"\"")
set(unix_exp "a;b c;d e;\;; '\";';\"")
separate_arguments(unix_out UNIX_COMMAND "${unix_cmd}")

set(windows_cmd "a \"b c\" 'd e' \";\" \\ \"c:\\windows\\path\\\\\" \\\"")
set(windows_exp "a;b c;'d;e';\;;\\;c:\\windows\\path\\;\"")
separate_arguments(windows_out WINDOWS_COMMAND "${windows_cmd}")

foreach(mode old unix windows)
  if(NOT "${${mode}_out}" STREQUAL "${${mode}_exp}")
    message(FATAL_ERROR "separate_arguments ${mode}-style failed.  "
      "Expected\n  [${${mode}_exp}]\nbut got\n  [${${mode}_out}]\n")
  endif()
endforeach()

set(nothing)
separate_arguments(nothing)
if(DEFINED nothing)
  message(FATAL_ERROR "separate_arguments null-case failed: "
    "nothing=[${nothing}]")
endif()
