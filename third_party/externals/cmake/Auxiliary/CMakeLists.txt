install(FILES cmake-help.vim cmake-indent.vim cmake-syntax.vim DESTINATION ${CMAKE_DATA_DIR}/editors/vim)
install(FILES cmake-mode.el DESTINATION ${CMAKE_DATA_DIR}/editors/emacs)
install(FILES cmake.m4 DESTINATION share/aclocal)
add_subdirectory (bash-completion)
