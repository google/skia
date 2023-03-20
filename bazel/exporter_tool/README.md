# Bazel Project Exporter

Skia's authoritative build system is moving to Bazel. For users needing to
use other build system, this tool will export a subset of the Bazel build
targets to other build systems.

# Bazel to CMake

**Note:** This is not meant for any purpose beyond development.

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

Generating the predefined `*.gni` files is done by running the following
at the root level of the Skia workspace:

```sh
make -C bazel generate_gni
```

This will update the `*.gni` files that reside in `//gn` that contain
file lists necessary for a GN build. The exporter tool is hardcoded
with the Bazel rules to be exported, and to which GNI file and
file list they should be mapped. As Bazel project rules are
refactored it may be necessary to update the exporter tool to reflect
those changes.

## Bazel Rule to GNI File List Mapping

The GNI export process is platform agnostic and generates the GNI files
with the same file lists on all platforms. Let's describe the mapping
process using a fictitious example program:

In `//include/example/BUILD.bazel` exists a rule defining the header
file:

```bazel
filegroup(
    name = "public_hdrs",
    srcs = [ "example.h" ],
)
```

**Note:** Bazel **visibility rules are ignored**. The exporter
tool can export private files.

In `//src/example/BUILD.bazel` a rule to define the example
sources:

```bazel
filegroup(
    name = "example_srcs",
    srcs = [
        "main.cpp",
        "draw.cpp",
        select({
            ":is_windows": [ "draw_win.cpp" ]
        }).
    ],
)
```

The rule → file list mapping in the exporter tool looks like:

```go
var gniExportDescs = []exporter.GNIExportDesc{
    // ... Other GNI definitions.
    {GNI: "gn/example.gni", Vars: []exporter.GNIFileListExportDesc{
		{Var: "example_headers",
			Rules: []string{"//include/example:public_hdrs"}},
		{Var: "example_sources",
			Rules: []string{"//src/example:example_srcs"}}},
	},
    // ... Other GNI definitions.
}
```

When the exporter tool is run, it will create the following
definitions in `//gn/example.gni`:

```gn
# DO NOT EDIT: This is a generated file.

_src = get_path_info("../src", "abspath")
_include = get_path_info("../include", "abspath")

example_headers = [ "$_include/example/example.h" ]

example_sources = [
    "$_src/example/main.cpp",
    "$_src/example/draw.cpp",
    "$_src/example/draw_win.cpp",
]
```

**Note:** The exporter always includes the contents of all `select()`
calls. This may be desired -- if not the solution is to pull
the files in a select into a new Bazel filegroup. For example:

```bazel
filegroup(
    name = "win_example_srcs",
    srcs = [ "draw_win.cpp" ],
)

filegroup(
    name = "example_srcs",
    srcs = [
        "main.cpp",
        "draw.cpp",
        srcs = select({
            ":is_windows": [ ":win_example_srcs" ]
        }).
    ],
)
```

Or alternatively:

```bazel
filegroup(
    name = "win_example_srcs",
    srcs = select({
        ":is_windows": [ "draw_win.cpp" ]
    }).
)

filegroup(
    name = "example_srcs",
    srcs = [
        "main.cpp",
        "draw.cpp",
        ":win_example_srcs", # Not recursively followed.
    ],
)
```

In each case the referenced rule (`win_example_srcs`) is not
followed and **only files directly listed in a rule are exported**
to a GNI file.
