"""
This file contains logic related to enforcing public API relationships, also known as
layering checks.

See also https://maskray.me/blog/2022-09-25-layering-check-with-clang and go/layering_check

"""

# https://github.com/bazelbuild/bazel/blob/master/tools/build_defs/cc/action_names.bzl
load("@bazel_tools//tools/build_defs/cc:action_names.bzl", "ACTION_NAMES")

# https://github.com/bazelbuild/bazel/blob/master/tools/cpp/cc_toolchain_config_lib.bzl
load(
    "@bazel_tools//tools/cpp:cc_toolchain_config_lib.bzl",
    "feature",
    "feature_set",
    "flag_group",
    "flag_set",
)

def make_layering_check_features():
    """Returns a list of features which enforce "layering checks".

    Layering checks catch two types of problems:
      1) A cc_library using private headers from another cc_library.
      2) A cc_library using public headers from a transitive dependency instead of
         directly depending on that library.

    This is implemented using Clang module maps, which are generated for each cc_library
    as it is being built.

    This implementation is very similar to the one in the default Bazel C++ toolchain
    (which is not inherited by custom toolchains).
    https://github.com/bazelbuild/bazel/commit/8b9f74649512ee17ac52815468bf3d7e5e71c9fa

    Returns:
        A list of Bazel "features", the primary one being one called "layering_check".
    """
    return [
        feature(
            name = "use_module_maps",
            enabled = False,
            requires = [feature_set(features = ["module_maps"])],
            flag_sets = [
                flag_set(
                    actions = [
                        ACTION_NAMES.c_compile,
                        ACTION_NAMES.cpp_compile,
                    ],
                    flag_groups = [
                        flag_group(
                            flags = [
                                "-fmodule-name=%{module_name}",
                                "-fmodule-map-file=%{module_map_file}",
                            ],
                        ),
                    ],
                ),
            ],
        ),
        # This feature name is baked into Bazel
        # https://github.com/bazelbuild/bazel/blob/8f5b626acea0086be8a314d5efbf6bc6d3473cd2/src/main/java/com/google/devtools/build/lib/rules/cpp/CompileBuildVariables.java#L471
        feature(name = "module_maps", enabled = True),
        feature(
            name = "layering_check",
            # This is currently disabled by default (although we aim to enable it by default)
            # because our current skia_public build does not pass the fmodules-strict-decluse
            # options with its current deps implementation (which was designed to pass these along).
            enabled = False,
            implies = ["use_module_maps"],
            flag_sets = [
                flag_set(
                    actions = [
                        ACTION_NAMES.c_compile,
                        ACTION_NAMES.cpp_compile,
                    ],
                    flag_groups = [
                        flag_group(flags = [
                            # Identify issue #1 (see docstring)
                            "-Wprivate-header",
                            # Identify issue #2
                            "-fmodules-strict-decluse",
                        ]),
                        flag_group(
                            iterate_over = "dependent_module_map_files",
                            flags = [
                                "-fmodule-map-file=%{dependent_module_map_files}",
                            ],
                        ),
                    ],
                ),
            ],
        ),
    ]

def generate_system_module_map(ctx, module_file, folders):
    """Generates a module map [1] for all the "system" headers in the toolchain.

    The generated map looks something like:
        module "crosstool" [system] {
            textual header "lib/clang/15.0.1/include/__clang_cuda_builtin_vars.h"
            textual header "lib/clang/15.0.1/include/__clang_cuda_cmath.h"
            ...
            textual header "include/c++/v1/climits"
            textual header "include/c++/v1/clocale"
            textual header "include/c++/v1/cmath"
            textual header "symlinks/xcode/MacSDK/usr/share/man/mann/zip.n"
        }
    Notice how all the file paths are relative to *this* directory, where
    the toolchain_system_headers.modulemap. Annoyingly, Clang will silently
    ignore a file that is declared if it does not actually exist on disk.

    [1] https://clang.llvm.org/docs/Modules.html#module-map-language

    Args:
        ctx: A repository_ctx (https://bazel.build/rules/lib/repository_ctx)
        module_file: The name of the modulemap file to create.
        folders: List of strings corresponding to paths in the toolchain with system headers.

    """

    # https://github.com/bazelbuild/bazel/blob/8f5b626acea0086be8a314d5efbf6bc6d3473cd2/tools/cpp/generate_system_module_map.sh
    script_path = ctx.path(Label("@bazel_tools//tools/cpp:generate_system_module_map.sh"))

    # https://bazel.build/rules/lib/repository_ctx#execute
    res = ctx.execute([script_path] + folders)
    if res.return_code != 0:
        fail("Could not generate module map")

    # https://bazel.build/rules/lib/repository_ctx#file
    ctx.file(
        module_file,
        content = res.stdout,
        executable = False,
    )
