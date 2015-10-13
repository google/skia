CMAKE_COMPILER_IS_GNU<LANG>
---------------------------

True if the compiler is GNU.

If the selected <LANG> compiler is the GNU compiler then this is TRUE,
if not it is FALSE.  Unlike the other per-language variables, this
uses the GNU syntax for identifying languages instead of the CMake
syntax.  Recognized values of the <LANG> suffix are:

::

  CC = C compiler
  CXX = C++ compiler
  G77 = Fortran compiler
