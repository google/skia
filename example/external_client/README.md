This folder demonstrates how an external client would depend on and build Skia
using their own C++ toolchain.

Look first in the `WORKSPACE.bazel` to see the setup part (with a quick detour to
`./custom_skia_config`) and then `BUILD.bazel` for the actual rules which use
Skia's modular build rules to assemble the components necessary for a particular task.