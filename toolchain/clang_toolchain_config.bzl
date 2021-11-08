"""
This file specifies a clang toolchain that can run on a Linux host which does depend on any
installed packages from the host machine.

See build_toolchain.bzl for more details on the creation of the toolchain.

It uses the usr subfolder of the built toolchain as a sysroot

It follows the example of:
 - https://docs.bazel.build/versions/4.2.1/tutorial/cc-toolchain-config.html
 - https://github.com/emscripten-core/emsdk/blob/7f39d100d8cd207094decea907121df72065517e/bazel/emscripten_toolchain/crosstool.bzl
"""

load(
    "@bazel_tools//tools/cpp:cc_toolchain_config_lib.bzl",
    "action_config",
    "feature",
    "flag_group",
    "flag_set",
    "tool",
    "variable_with_value",
)
load("@bazel_tools//tools/build_defs/cc:action_names.bzl", "ACTION_NAMES")

# The location of the created clang toolchain.
EXTERNAL_TOOLCHAIN = "external/clang_linux_amd64_musl"

def _clang_impl(ctx):
    action_configs = _make_action_configs()
    features = []
    features += _make_default_flags()
    features += _make_diagnostic_flags()

    # https://docs.bazel.build/versions/main/skylark/lib/cc_common.html#create_cc_toolchain_config_info
    # Note, this rule is defined in Java code, not Starlark
    # https://cs.opensource.google/bazel/bazel/+/master:src/main/java/com/google/devtools/build/lib/starlarkbuildapi/cpp/CcModuleApi.java
    return cc_common.create_cc_toolchain_config_info(
        ctx = ctx,
        features = features,
        abi_libc_version = "unknown",
        abi_version = "unknown",
        action_configs = action_configs,
        builtin_sysroot = EXTERNAL_TOOLCHAIN + "/usr",
        compiler = "clang",
        host_system_name = "local",
        target_cpu = "k8",
        target_libc = "musl",
        target_system_name = "local",
        toolchain_identifier = "clang-toolchain",
    )

provide_clang_toolchain_config = rule(
    attrs = {},
    provides = [CcToolchainConfigInfo],
    implementation = _clang_impl,
)

def _make_action_configs():
    """
    This function sets up the tools needed to perform the various compile/link actions.

    Bazel normally restricts us to referring to (and therefore running) executables/scripts
    that are in this directory (That is EXEC_ROOT/toolchain). However, the executables we want
    to run are brought in via WORKSPACE.bazel and are located in EXEC_ROOT/external/clang....
    Therefore, we make use of "trampoline scripts" that will call the binaries from the
    toolchain directory.

    These action_configs also let us dynamically specify arguments from the Bazel
    environment if necessary (see cpp_link_static_library_action).
    """

    # https://cs.opensource.google/bazel/bazel/+/master:tools/cpp/cc_toolchain_config_lib.bzl;l=435;drc=3b9e6f201a9a3465720aad8712ab7bcdeaf2e5da
    clang_tool = tool(path = "clang_trampoline.sh")
    lld_tool = tool(path = "lld_trampoline.sh")
    ar_tool = tool(path = "ar_trampoline.sh")

    # https://cs.opensource.google/bazel/bazel/+/master:tools/cpp/cc_toolchain_config_lib.bzl;l=488;drc=3b9e6f201a9a3465720aad8712ab7bcdeaf2e5da
    assemble_action = action_config(
        action_name = ACTION_NAMES.assemble,
        tools = [clang_tool],
    )
    c_compile_action = action_config(
        action_name = ACTION_NAMES.c_compile,
        tools = [clang_tool],
    )
    cpp_compile_action = action_config(
        action_name = ACTION_NAMES.cpp_compile,
        tools = [clang_tool],
    )
    linkstamp_compile_action = action_config(
        action_name = ACTION_NAMES.linkstamp_compile,
        tools = [clang_tool],
    )
    preprocess_assemble_action = action_config(
        action_name = ACTION_NAMES.preprocess_assemble,
        tools = [clang_tool],
    )

    cpp_link_dynamic_library_action = action_config(
        action_name = ACTION_NAMES.cpp_link_dynamic_library,
        tools = [lld_tool],
    )
    cpp_link_executable_action = action_config(
        action_name = ACTION_NAMES.cpp_link_executable,
        # Bazel assumes it is talking to clang when building an executable. There are
        # "-Wl" flags on the command: https://releases.llvm.org/6.0.1/tools/clang/docs/ClangCommandLineReference.html#cmdoption-clang-Wl
        tools = [clang_tool],
    )
    cpp_link_nodeps_dynamic_library_action = action_config(
        action_name = ACTION_NAMES.cpp_link_nodeps_dynamic_library,
        tools = [lld_tool],
    )

    # This is the same rule as
    # https://github.com/emscripten-core/emsdk/blob/7f39d100d8cd207094decea907121df72065517e/bazel/emscripten_toolchain/crosstool.bzl#L143
    # By default, there are no flags or libraries passed to the llvm-ar tool, so
    # we need to specify them. The variables mentioned by expand_if_available are defined
    # https://docs.bazel.build/versions/main/cc-toolchain-config-reference.html#cctoolchainconfiginfo-build-variables
    cpp_link_static_library_action = action_config(
        action_name = ACTION_NAMES.cpp_link_static_library,
        flag_sets = [
            flag_set(
                flag_groups = [
                    flag_group(
                        # https://llvm.org/docs/CommandGuide/llvm-ar.html
                        # replace existing files or insert them if they already exist,
                        # create the file if it doesn't already exist
                        # symbol table should be added
                        # Deterministic timestamps should be used
                        flags = ["rcsD", "%{output_execpath}"],
                        # Despite the name, output_execpath just refers to linker output,
                        # e.g. libFoo.a
                        expand_if_available = "output_execpath",
                    ),
                ],
            ),
            flag_set(
                flag_groups = [
                    flag_group(
                        iterate_over = "libraries_to_link",
                        flag_groups = [
                            flag_group(
                                flags = ["%{libraries_to_link.name}"],
                                expand_if_equal = variable_with_value(
                                    name = "libraries_to_link.type",
                                    value = "object_file",
                                ),
                            ),
                            flag_group(
                                flags = ["%{libraries_to_link.object_files}"],
                                iterate_over = "libraries_to_link.object_files",
                                expand_if_equal = variable_with_value(
                                    name = "libraries_to_link.type",
                                    value = "object_file_group",
                                ),
                            ),
                        ],
                        expand_if_available = "libraries_to_link",
                    ),
                ],
            ),
            flag_set(
                flag_groups = [
                    flag_group(
                        flags = ["@%{linker_param_file}"],
                        expand_if_available = "linker_param_file",
                    ),
                ],
            ),
        ],
        tools = [ar_tool],
    )

    action_configs = [
        assemble_action,
        c_compile_action,
        cpp_compile_action,
        cpp_link_dynamic_library_action,
        cpp_link_executable_action,
        cpp_link_nodeps_dynamic_library_action,
        cpp_link_static_library_action,
        linkstamp_compile_action,
        preprocess_assemble_action,
    ]
    return action_configs

def _make_default_flags():
    """Here we define the flags for certain actions that are always applied."""
    cxx_compile_includes = flag_set(
        actions = [
            ACTION_NAMES.c_compile,
            ACTION_NAMES.cpp_compile,
        ],
        flag_groups = [
            flag_group(
                flags = [
                    # THIS ORDER MATTERS GREATLY. If these are in the wrong order, the
                    # #include_next directives will walk off the end of the specified system
                    # folders here and look in the absolute path that happens to contain
                    # the clang executable. Because that looks like an absolute path to
                    # Bazel, it will declare the Build is not properly specified (that is,
                    # it appears to use headers outside of Bazel's control/view) and fail.
                    "-isystem",
                    EXTERNAL_TOOLCHAIN + "/include/c++/v1",
                    "-isystem",
                    EXTERNAL_TOOLCHAIN + "/lib/clang/13.0.0/include",
                    "-isystem",
                    EXTERNAL_TOOLCHAIN + "/usr/include/x86_64-linux-musl",
                ],
            ),
        ],
    )

    cpp_compile_includes = flag_set(
        actions = [
            ACTION_NAMES.cpp_compile,
        ],
        flag_groups = [
            flag_group(
                flags = [
                    # http://g/skia-staff/bhPPBV4YdeU/5oyG5GRgBQAJ
                    "-std=c++14",
                    "-Wno-c++17-extensions",
                    "-Wno-psabi",  # noisy
                    # This define allows libc++ to work with musl. They were discovered by
                    # trying to compile without them, reading errors and source code, e.g.
                    # https://github.com/llvm/llvm-project/blob/f4c1258d5633fcf06385ff3fd1f4bf57ab971964/libcxx/include/__locale#L513
                    # https://github.com/llvm/llvm-project/blob/f4c1258d5633fcf06385ff3fd1f4bf57ab971964/libcxx/include/__config#L1155
                    # https://github.com/llvm/llvm-project/blob/f4c1258d5633fcf06385ff3fd1f4bf57ab971964/libcxx/include/__support/musl/xlocale.h
                    "-D_LIBCPP_HAS_MUSL_LIBC",
                ],
            ),
        ],
    )

    link_exe_flags = flag_set(
        actions = [ACTION_NAMES.cpp_link_executable],
        flag_groups = [
            flag_group(
                flags = [
                    "-fuse-ld=lld",
                    # This is the path to libc.a and other libraries.
                    "-L" + EXTERNAL_TOOLCHAIN + "/usr/lib/x86_64-linux-musl",
                    "-L" + EXTERNAL_TOOLCHAIN + "/lib",
                    # We chose to use the llvm runtime, not the gcc one because it is already
                    # included in the clang binary
                    "--rtlib=compiler-rt",
                    # In order to run our executables, they need to be statically linked,
                    # otherwise, the libc++.so and friends will not be found if they are
                    # not installed on the user's machine.
                    "-static",
                    "-std=c++14",
                    "-lc++abi",
                    "-lunwind",
                    "-lc++",
                ],
            ),
        ],
    )
    return [feature(
        "default_flags",
        enabled = True,
        flag_sets = [
            cxx_compile_includes,
            cpp_compile_includes,
            link_exe_flags,
        ],
    )]

def _make_diagnostic_flags():
    """Here we define the flags that can be turned on via features to yield debug info."""
    cxx_diagnostic = flag_set(
        actions = [
            ACTION_NAMES.c_compile,
            ACTION_NAMES.cpp_compile,
        ],
        flag_groups = [
            flag_group(
                flags = [
                    "--trace-includes",
                    "-v",
                ],
            ),
        ],
    )

    link_diagnostic = flag_set(
        actions = [ACTION_NAMES.cpp_link_executable],
        flag_groups = [
            flag_group(
                flags = [
                    "-Wl,--verbose",
                    "-v",
                ],
            ),
        ],
    )

    link_search_dirs = flag_set(
        actions = [ACTION_NAMES.cpp_link_executable],
        flag_groups = [
            flag_group(
                flags = [
                    "--print-search-dirs",
                ],
            ),
        ],
    )
    return [
        # Running a Bazel command with --features diagnostic will cause the compilation and
        # link steps to be more verbose.
        feature(
            "diagnostic",
            enabled = False,
            flag_sets = [
                cxx_diagnostic,
                link_diagnostic,
            ],
        ),
        # Running a Bazel command with --features print_search_dirs will cause the link to fail
        # but directories searched for libraries, etc will be displayed.
        feature(
            "print_search_dirs",
            enabled = False,
            flag_sets = [
                link_search_dirs,
            ],
        ),
    ]
