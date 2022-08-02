# Bazel Project Exporter

Skia's authoritative build system is moving to Bazel. For users needing to
use other build system, this tool will export a subset of the Bazel build
targets to other build systems.

This is not meant for any purpose beyond development.

# Bazel to CMake

At the root level of the Skia workspace:

```sh
bazel build //bazel/exporter_tool
bazel-bin/bazel/exporter_tool/exporter_tool_/exporter_tool -proj_name=Skia -rule='//:skia_public'
```

This will write to a single `CMakeLists.txt` file a valid CMake project with
targets to build the artifacts covered by the Bazel //:skia_public target
and all dependent targets.

## Current limitations

* External dependencies are not supported.
* Only the `//:skia_public` rule is supported. Other rules *may* work.