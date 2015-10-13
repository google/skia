macro(REGEX_ESCAPE_STRING _OUT _IN)
    # Escape special regex metacharacters with a backslash
    string(REGEX REPLACE "([$^.[|*+?()]|])" "\\\\\\1" ${_OUT} "${_IN}")
endmacro()
