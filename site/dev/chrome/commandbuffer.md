Chromium Command Buffer
==========================

It is possible to run Skia's correctness tool, dm, and benchmarking tool,
nanobench, on top of the GL ES interface provided by Chromium's command
buffer.

The Skia tools are always built with this support. They dynamically load
the command buffer as a shared library and thus no GYP/GN flags are
required.

The command buffer standalone shared library is built in a Chromium checkout
by building the `command_buffer_gles2` target. The command buffer should be
built with the `is_component_build` in GN set to false. This will produce a .so,
.dylib, or .dll depending on the target OS. This should be copied alongside
the dm or nanobench executable built from a Skia repository.

Both tools have a `commandbuffer` config which can be used with the `--config`
option to the tool and will run the tests or benchmarks using the command buffer
library. Unit tests in dm always run on all appropriate and available backends
regardless of the `--config` flag.

