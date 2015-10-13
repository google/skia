function (check_directory_property dir prop)
  get_directory_property(gdp_val DIRECTORY "${dir}" "${prop}")
  get_property(gp_val
    DIRECTORY "${dir}"
    PROPERTY "${prop}")

  message("get_directory_property: -->${gdp_val}<--")
  message("get_property: -->${gp_val}<--")
endfunction ()

set_directory_properties(PROPERTIES empty "" custom value)

check_directory_property("${CMAKE_CURRENT_SOURCE_DIR}" empty)
check_directory_property("${CMAKE_CURRENT_SOURCE_DIR}" custom)
check_directory_property("${CMAKE_CURRENT_SOURCE_DIR}" noexist)
