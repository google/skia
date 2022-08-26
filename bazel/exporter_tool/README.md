# Bazel Project Exporter

Skia's authoritative build system is moving to Bazel. For users needing to
use other build system, this tool will export a subset of the Bazel build
targets to other build systems.

This is not meant for any purpose beyond development.

# Bazel to CMake

At the root level of the Skia workspace:

```sh
make -C bazel generate_cmake
```

This will write to a single `CMakeLists.txt` file a valid CMake project with
targets to build the artifacts covered by the Bazel //:skia_public target
and all dependent targets.

## Current limitations

* External dependencies are not supported.
* Only the `//:skia_public` rule is supported. Other rules *may* work.

# Bazel to *.gni

At the root level of the Skia workspace:

```sh
make -C bazel generate_gni
```

This will update some `*.gni` files that reside in //gn that contain file lists.

# Out of Date Check
The exporter tool has a flag to identify all output files which are out of date.
This can be run as so:

```sh
exporter_tool -check_current ...
```

The will return a zero return code if all files are up to date.