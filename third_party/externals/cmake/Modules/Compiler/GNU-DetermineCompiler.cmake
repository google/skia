
set(_compiler_id_pp_test "defined(__GNUC__)")

set(_compiler_id_version_compute "
# define @PREFIX@COMPILER_VERSION_MAJOR @MACRO_DEC@(__GNUC__)
# define @PREFIX@COMPILER_VERSION_MINOR @MACRO_DEC@(__GNUC_MINOR__)
# if defined(__GNUC_PATCHLEVEL__)
#  define @PREFIX@COMPILER_VERSION_PATCH @MACRO_DEC@(__GNUC_PATCHLEVEL__)
# endif")
