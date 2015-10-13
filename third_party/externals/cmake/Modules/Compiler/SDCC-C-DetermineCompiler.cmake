
# sdcc, the small devices C compiler for embedded systems,
#   http://sdcc.sourceforge.net  */
set(_compiler_id_pp_test "defined(SDCC)")

set(_compiler_id_version_compute "
  /* SDCC = VRP */
#  define COMPILER_VERSION_MAJOR @MACRO_DEC@(SDCC/100)
#  define COMPILER_VERSION_MINOR @MACRO_DEC@(SDCC/10 % 10)
#  define COMPILER_VERSION_PATCH @MACRO_DEC@(SDCC    % 10)")
