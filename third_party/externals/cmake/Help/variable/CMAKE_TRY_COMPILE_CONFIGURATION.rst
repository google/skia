CMAKE_TRY_COMPILE_CONFIGURATION
-------------------------------

Build configuration used for try_compile and try_run projects.

Projects built by try_compile and try_run are built synchronously
during the CMake configuration step.  Therefore a specific build
configuration must be chosen even if the generated build system
supports multiple configurations.
