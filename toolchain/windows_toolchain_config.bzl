"""
This file specifies a clang toolchain that can run on a Windows host.

See download_windows_toolchain.bzl for more details on the creation of the toolchain.

It uses the usr subfolder of the built toolchain as a sysroot

It follows the example of:
 - linux_amd64_toolchain_config.bzl
"""

# https://github.com/bazelbuild/bazel/blob/master/tools/build_defs/cc/action_names.bzl
load("@bazel_tools//tools/build_defs/cc:action_names.bzl", "ACTION_NAMES")

# https://github.com/bazelbuild/bazel/blob/master/tools/cpp/cc_toolchain_config_lib.bzl
load(
    "@bazel_tools//tools/cpp:cc_toolchain_config_lib.bzl",
    "action_config",
    "feature",
    "flag_group",
    "flag_set",
    "tool",
    "variable_with_value",
)

# TODO(borenet): These variables were copied from the automatically-generated
# @clang_windows_amd64//:vars.bzl file. They are available to be directly
# used here, but in order to do so, Bazel needs access to both the Clang and
# MSVC archives, even when we aren't going to use this toolchain. Due to the
# time required to download and extract these archives, we've opted to hard-code
# the versions and paths here.
#load("@clang_windows_amd64//:vars.bzl", "MSVC_INCLUDE", "MSVC_LIB", "WIN_SDK_INCLUDE", "WIN_SDK_LIB")
MSVC_VERSION = "14.39.33519"
MSVC_INCLUDE = "VC/Tools/MSVC/" + MSVC_VERSION + "/include"
MSVC_LIB = "VC/Tools/MSVC/" + MSVC_VERSION + "/lib"
WIN_SDK_VERSION = "10.0.22621.0"
WIN_SDK_INCLUDE = "win_sdk/Include/" + WIN_SDK_VERSION
WIN_SDK_LIB = "win_sdk/Lib/" + WIN_SDK_VERSION

# The location of the downloaded clang toolchain.
CLANG_TOOLCHAIN = "external/clang_windows_amd64"

# Paths inside the win_toolchain CIPD package.
FULL_MSVC_INCLUDE = CLANG_TOOLCHAIN + "/" + MSVC_INCLUDE
FULL_MSVC_LIB = CLANG_TOOLCHAIN + "/" + MSVC_LIB
FULL_WIN_SDK_INCLUDE = CLANG_TOOLCHAIN + "/" + WIN_SDK_INCLUDE
FULL_WIN_SDK_LIB = CLANG_TOOLCHAIN + "/" + WIN_SDK_LIB

def _windows_amd64_toolchain_info(ctx):
    action_configs = _make_action_configs()
    features = [
        feature(
            name = "archive_param_file",
            enabled = True,
        ),
    ]
    features += _make_default_flags()
    features += _make_diagnostic_flags()

    # https://bazel.build/rules/lib/cc_common#create_cc_toolchain_config_info
    # Note, this rule is defined in Java code, not Starlark
    # https://cs.opensource.google/bazel/bazel/+/master:src/main/java/com/google/devtools/build/lib/starlarkbuildapi/cpp/CcModuleApi.java
    return cc_common.create_cc_toolchain_config_info(
        ctx = ctx,
        features = features,
        action_configs = action_configs,
        # These are required, but do nothing
        compiler = "",
        target_cpu = "",
        target_libc = "",
        target_system_name = "",
        toolchain_identifier = "",
    )

provide_windows_amd64_toolchain_config = rule(
    attrs = {},
    provides = [CcToolchainConfigInfo],
    implementation = _windows_amd64_toolchain_info,
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
    clang_tool = tool(path = "windows_trampolines/clang_trampoline_windows.bat")
    ar_tool = tool(path = "windows_trampolines/ar_trampoline_windows.bat")

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
        tools = [clang_tool],
    )
    cpp_link_executable_action = action_config(
        action_name = ACTION_NAMES.cpp_link_executable,
        # Bazel assumes it is talking to clang when building an executable. There are
        # "-Wl" flags on the command: https://releases.llvm.org/6.0.1/tools/clang/docs/ClangCommandLineReference.html#cmdoption-clang-Wl
        tools = [clang_tool],
    )
    cpp_link_nodeps_dynamic_library_action = action_config(
        action_name = ACTION_NAMES.cpp_link_nodeps_dynamic_library,
        tools = [clang_tool],
    )

    # By default, there are no flags or libraries passed to the llvm-ar tool, so
    # we need to specify them. The variables mentioned by expand_if_available are defined
    # https://bazel.build/docs/cc-toolchain-config-reference#cctoolchainconfiginfo-build-variables
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
                        flags = ["@%{archive_param_file}"],
                        expand_if_available = "archive_param_file",
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
    """Here we define the flags for certain actions that are always applied.

    For any flag that might be conditionally applied, it should be defined in //bazel/copts.bzl.

    Flags that are set here will be unconditionally applied to everything we compile with
    this toolchain, even third_party deps.
    """

    # Note: These values must be kept in sync with those defined in cmake_exporter.go.
    cxx_compile_includes = flag_set(
        actions = [
            ACTION_NAMES.c_compile,
            ACTION_NAMES.cpp_compile,
        ],
        flag_groups = [
            flag_group(
                flags = [
                    # THIS ORDER MATTERS GREATLY. If these are in the wrong order, the
                    # #include_next directives will fail to find the files, causing a compilation
                    # error (or, without -no-canonical-prefixes, a mysterious case where files
                    # are included with an absolute path and fail the build).
                    "-isystem",
                    CLANG_TOOLCHAIN + "/include/clang",
                    "-isystem",
                    CLANG_TOOLCHAIN + "/include/clang-c",
                    "-isystem",
                    CLANG_TOOLCHAIN + "/include/clang-tidy",
                    "-isystem",
                    CLANG_TOOLCHAIN + "/include/lld",
                    "-isystem",
                    CLANG_TOOLCHAIN + "/include/lldb",
                    "-isystem",
                    CLANG_TOOLCHAIN + "/include/llvm",
                    "-isystem",
                    CLANG_TOOLCHAIN + "/include/llvm-c",
                    "-isystem",
                    CLANG_TOOLCHAIN + "/lib/clang/18/include",
                    "-isystem",
                    FULL_WIN_SDK_INCLUDE + "/shared",
                    "-isystem",
                    FULL_WIN_SDK_INCLUDE + "/ucrt",
                    "-isystem",
                    FULL_WIN_SDK_INCLUDE + "/um",
                    "-isystem",
                    FULL_WIN_SDK_INCLUDE + "/winrt",
                    "-isystem",
                    FULL_WIN_SDK_INCLUDE + "/cppwinrt",
                    "-isystem",
                    FULL_MSVC_INCLUDE,
                    # We do not want clang to search in absolute paths for files. This makes
                    # Bazel think we are using an outside resource and fail the compile.
                    "-no-canonical-prefixes",
                ],
            ),
        ],
    )

    cpp_compile_flags = flag_set(
        actions = [
            ACTION_NAMES.cpp_compile,
        ],
        flag_groups = [
            flag_group(
                flags = [
                    "-std=c++17",
                ],
            ),
        ],
    )

    link_exe_flags = flag_set(
        actions = [
            ACTION_NAMES.cpp_link_executable,
            ACTION_NAMES.cpp_link_dynamic_library,
            ACTION_NAMES.cpp_link_nodeps_dynamic_library,
        ],
        flag_groups = [
            flag_group(
                flags = [
                    "-fuse-ld=lld",
                    # We chose to use the llvm runtime, not the gcc one because it is already
                    # included in the clang binary
                    "--rtlib=compiler-rt",
                    "-std=c++17",
                    "-L" + FULL_MSVC_LIB + "/x64",
                    "-L" + FULL_WIN_SDK_LIB + "/ucrt/x64",
                    "-L" + FULL_WIN_SDK_LIB + "/um/x64",
                ],
            ),
        ],
    )
    return [feature(
        "default_flags",
        enabled = True,
        flag_sets = [
            cxx_compile_includes,
            cpp_compile_flags,
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
                    "-Wl,-verbose",
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
        feature(
            "diagnostic_link",
            enabled = False,
            flag_sets = [
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
