include(Compiler/XL)
__compiler_xl(C)
set(CMAKE_C_FLAGS_RELEASE_INIT "${CMAKE_C_FLAGS_RELEASE_INIT} -DNDEBUG")
set(CMAKE_C_FLAGS_MINSIZEREL_INIT "${CMAKE_C_FLAGS_MINSIZEREL_INIT} -DNDEBUG")

# -qthreaded     = Ensures that all optimizations will be thread-safe
# -qalias=noansi = Turns off type-based aliasing completely (safer optimizer)
# -qhalt=e       = Halt on error messages (rather than just severe errors)
set(CMAKE_C_FLAGS_INIT "-qthreaded -qalias=noansi -qhalt=e")
