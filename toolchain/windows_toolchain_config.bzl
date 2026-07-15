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
    "artifact_name_pattern",
    "feature",
    "flag_group",
    "flag_set",
    "tool",
    "variable_with_value",
    "with_feature_set",
)
load("@rules_cc//cc/common:cc_common.bzl", "cc_common")

# These variables were copied from the generated @clang_windows_amd64//:vars.bzl file.
# They are available to be directly used here, but in order to do so, Bazel needs access
# to both the Clang and MSVC archives, even when we aren't going to use this toolchain.
# Due to the time required to download and extract these archives, we've opted to hard-code
# the versions and paths here.
MSVC_VERSION = "14.51.36231"
MSVC_INCLUDE = "VC/Tools/MSVC/" + MSVC_VERSION + "/include"
MSVC_LIB = "VC/Tools/MSVC/" + MSVC_VERSION + "/lib"

WIN_SDK_VERSION = "10.0.26100.0"
WIN_SDK_INCLUDE = "win_sdk/Include/" + WIN_SDK_VERSION
WIN_SDK_LIB = "win_sdk/Lib/" + WIN_SDK_VERSION

def _windows_amd64_toolchain_info(ctx):
    # https://bazel.build/rules/lib/builtins/Label#repo_name
    clang_repo_name = ctx.attr.clang_windows_amd64.label.repo_name
    clang_toolchain_path = "external/" + clang_repo_name
    action_configs = _make_action_configs(clang_toolchain_path)
    features = [
        # These are defined https://github.com/bazelbuild/rules_cc/blob/77eb752fc74b89e96e729ce9777854836fcbd9a2/cc/private/toolchain/windows_cc_toolchain_config.bzl#L468-L476
        feature(
            name = "archive_param_file",
            enabled = True,
        ),
        feature(
            name = "compiler_param_file",
            enabled = True,
        ),
        feature(
            name = "linker_param_file",
            enabled = True,
        ),
        # This feature is defined but disabled by rules_rust
        # https://github.com/bazelbuild/rules_rust/blob/ed321505851d2cc8a5ace048188ef3ba8f7e8d71/rust/private/utils.bzl#L38-L41
        # We enable it when compiling C++ code so we can make the toolchain behave differently when
        # linking C++ code vs linking rust code. The rust toolchain assumes it is talking to a raw
        # linker but the C++ rules assume they are talking to a compiler driver. Thus, we set the
        # tool for cpp_link_executable_action (and others) differently depending on if we are in
        # the rust toolchain vs the C++ toolchain.
        feature(
            name = "rules_rust_unsupported_feature",
            enabled = True,
        ),
    ]
    features += _make_default_flags(clang_toolchain_path)
    features += _make_diagnostic_flags()

    # Tells Bazel that our compiler will add .exe etc to the outputs
    # https://stackoverflow.com/a/63889676
    artifact_name_patterns = [
        artifact_name_pattern(
            category_name = "executable",
            prefix = "",
            extension = ".exe",
        ),
        artifact_name_pattern(
            category_name = "dynamic_library",
            prefix = "",
            extension = ".dll",
        ),
        artifact_name_pattern(
            category_name = "static_library",
            prefix = "",
            extension = ".lib",
        ),
    ]

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
        artifact_name_patterns = artifact_name_patterns,
    )

provide_windows_amd64_toolchain_config = rule(
    attrs = {
        "clang_windows_amd64": attr.label(default = "@clang_windows_amd64//:compile_files"),
    },
    provides = [CcToolchainConfigInfo],
    implementation = _windows_amd64_toolchain_info,
)

def _make_action_configs(clang_toolchain_path):
    """
    This function sets up the tools needed to perform the various compile/link actions.

    Tools:
    https://cs.opensource.google/bazel/bazel/+/master:tools/cpp/cc_toolchain_config_lib.bzl;l=435;drc=3b9e6f201a9a3465720aad8712ab7bcdeaf2e5da
    Action Configs:
    https://cs.opensource.google/bazel/bazel/+/master:tools/cpp/cc_toolchain_config_lib.bzl;l=488;drc=3b9e6f201a9a3465720aad8712ab7bcdeaf2e5da
    """

    # Clang can run in GCC compatible mode or MSVC compatible mode depending on the file name.
    # The Bazel rules assume GCC mode for compiling but we have to use MSVC mode for llvm-lib
    # (which is `lld-link.exe /lib` under the hood). GCC mode for compiling makes it easier to
    # align copts.bzl (and other custome C++ flags) at the expense of a more complicated toolchain.
    compile_gcc_tool = tool(path = "../" + clang_toolchain_path + "/bin/clang.exe")
    archive_msvc_tool = tool(path = "../" + clang_toolchain_path + "/bin/llvm-lib.bat")

    link_cpp_tool = tool(
        path = "../" + clang_toolchain_path + "/bin/clang-cl.exe",
        # "The tool applied to the action will be the first Tool with a feature set that matches the
        # feature configuration." Bazel assumes it's talking to a C++ "compiler driver" when linking
        # C++ code (e.g. cl.exe or clang-cl.exe), so we only enable it when the
        # rules_rust_unsupported_feature is set (see above).
        with_features = [with_feature_set(features = ["rules_rust_unsupported_feature"])],
    )
    link_raw_tool = tool(path = "../" + clang_toolchain_path + "/bin/lld-link.exe")

    assemble_action = action_config(
        action_name = ACTION_NAMES.assemble,
        tools = [compile_gcc_tool],
    )
    c_compile_action = action_config(
        action_name = ACTION_NAMES.c_compile,
        tools = [compile_gcc_tool],
    )
    cpp_compile_action = action_config(
        action_name = ACTION_NAMES.cpp_compile,
        tools = [compile_gcc_tool],
    )
    linkstamp_compile_action = action_config(
        action_name = ACTION_NAMES.linkstamp_compile,
        tools = [compile_gcc_tool],
    )
    preprocess_assemble_action = action_config(
        action_name = ACTION_NAMES.preprocess_assemble,
        tools = [compile_gcc_tool],
    )

    cpp_link_dynamic_library_action = action_config(
        action_name = ACTION_NAMES.cpp_link_dynamic_library,
        tools = [link_cpp_tool, link_raw_tool],  # C++ mode with fallback for rust
    )
    cpp_link_executable_action = action_config(
        action_name = ACTION_NAMES.cpp_link_executable,
        tools = [link_cpp_tool, link_raw_tool],
    )
    cpp_link_nodeps_dynamic_library_action = action_config(
        action_name = ACTION_NAMES.cpp_link_nodeps_dynamic_library,
        tools = [link_cpp_tool, link_raw_tool],
    )

    # By default, there are no flags or libraries passed to the linker tool, so
    # we need to specify them. The variables mentioned by expand_if_available are defined
    # https://bazel.build/docs/cc-toolchain-config-reference#cctoolchainconfiginfo-build-variables
    cpp_link_static_library_action = action_config(
        action_name = ACTION_NAMES.cpp_link_static_library,
        # We need to use the param file to avoid Windows command line limits. Enable that here
        # (Bazel still decides when to use it or not).
        implies = ["archive_param_file"],
        # The order of these sets matters. We effectively want
        #   archive_msvc_tool /OUT:thing.lib first.obj second.obj third.obj ...
        # Thus, we put the set that has /OUT *first* and the logic to handle .obj files second.
        # The third set is for when Bazel decides things have gotten and it puts all those previous
        # args in a param file and runs
        #   archive_msvc_tool @thing_lib.params
        flag_sets = [
            flag_set(
                flag_groups = [
                    # How to read basic flag_groups
                    # (see also https://bazel.build/docs/cc-toolchain-config-reference#flag-groups)
                    # flag_group(
                    #   # If Bazel has this variable...
                    #   expand_if_available = "SOME_VARIABLE",
                    #   # ... here's how to format it (with substitution)
                    #    flags = ["/MV:%{SOME_VARIABLE}"],
                    # ),
                    flag_group(
                        # Despite the name, output_execpath just refers to linker output,
                        # e.g. libFoo.lib
                        expand_if_available = "output_execpath",
                        # lld-link.exe in /lib mode expects the MSVC-style /OUT:<file> option
                        flags = ["/OUT:%{output_execpath}"],
                    ),
                ],
            ),
            flag_set(
                flag_groups = [
                    flag_group(
                        iterate_over = "libraries_to_link",
                        flag_groups = [
                            # Pull out only the object files as a list (e.g. SkColor.obj)
                            flag_group(
                                flags = ["%{libraries_to_link.name}"],
                                expand_if_equal = variable_with_value(
                                    name = "libraries_to_link.type",
                                    value = "object_file",
                                ),
                            ),
                            # Handle nested lists of object files.
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
                        # This should be available because of the archive_param_file feature above.
                        expand_if_available = "linker_param_file",
                        flags = ["@%{linker_param_file}"],
                    ),
                ],
            ),
        ],
        tools = [archive_msvc_tool],
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

def _make_default_flags(clang_toolchain):
    """Here we define the flags for certain actions that are always applied.

    For any flag that might be conditionally applied, it should be defined in //bazel/copts.bzl.

    Flags that are set here will be unconditionally applied to everything we compile with
    this toolchain, even third_party deps.
    """
    full_msvc_include = clang_toolchain + "/" + MSVC_INCLUDE
    full_msvc_lib = clang_toolchain + "/" + MSVC_LIB
    full_win_sdk_include = clang_toolchain + "/" + WIN_SDK_INCLUDE
    full_win_sdk_lib = clang_toolchain + "/" + WIN_SDK_LIB

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
                    clang_toolchain + "/include/clang",
                    "-isystem",
                    clang_toolchain + "/include/clang-c",
                    "-isystem",
                    clang_toolchain + "/include/clang-tidy",
                    "-isystem",
                    clang_toolchain + "/include/lld",
                    "-isystem",
                    clang_toolchain + "/include/lldb",
                    "-isystem",
                    clang_toolchain + "/include/llvm",
                    "-isystem",
                    clang_toolchain + "/include/llvm-c",
                    "-isystem",
                    clang_toolchain + "/lib/clang/23/include",
                    "-isystem",
                    full_win_sdk_include + "/shared",
                    "-isystem",
                    full_win_sdk_include + "/ucrt",
                    "-isystem",
                    full_win_sdk_include + "/um",
                    "-isystem",
                    full_win_sdk_include + "/winrt",
                    "-isystem",
                    full_win_sdk_include + "/cppwinrt",
                    "-isystem",
                    full_msvc_include,
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
                    "-std=c++20",
                    "-D_CRT_SECURE_NO_WARNINGS",  # Disables warnings about sscanf()
                    "-D_HAS_EXCEPTIONS=0",  # Disables exceptions in MSVC STL
                    "-DWIN32_LEAN_AND_MEAN",  # Avoids pollution of global namespace
                    "-DNOMINMAX",  # https://stackoverflow.com/a/22744273
                ],
            ),
        ],
    )

    link_exe_flags_cpp = flag_set(
        actions = [
            ACTION_NAMES.cpp_link_executable,
            ACTION_NAMES.cpp_link_dynamic_library,
            ACTION_NAMES.cpp_link_nodeps_dynamic_library,
        ],
        with_features = [with_feature_set(features = ["rules_rust_unsupported_feature"])],
        flag_groups = [
            flag_group(
                flags = [
                    "/MT",  # clang in GCC mode implies the static C runtime (/MT) by default
                    "/link",
                    "/subsystem:console",  # Make a console window and look for main()
                    "libcpmt.lib",  # Standard C++ library
                    "libcmt.lib",  # static core C runtime
                    "libvcruntime.lib",  # Visual C++
                    "libucrt.lib",  # Universal C Runtime
                    "kernel32.lib",  # Win32 base API
                    "ntdll.lib",  # native NT system services.
                    "/LIBPATH:" + full_msvc_lib + "/x64",
                    "/LIBPATH:" + full_win_sdk_lib + "/ucrt/x64",
                    "/LIBPATH:" + full_win_sdk_lib + "/um/x64",
                ],
            ),
        ],
    )

    link_exe_flags_rust = flag_set(
        actions = [
            ACTION_NAMES.cpp_link_executable,
            ACTION_NAMES.cpp_link_dynamic_library,
            ACTION_NAMES.cpp_link_nodeps_dynamic_library,
        ],
        with_features = [with_feature_set(not_features = ["rules_rust_unsupported_feature"])],
        flag_groups = [
            flag_group(
                flags = [
                    "/LIBPATH:" + full_msvc_lib + "/x64",
                    "/LIBPATH:" + full_win_sdk_lib + "/ucrt/x64",
                    "/LIBPATH:" + full_win_sdk_lib + "/um/x64",
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
            link_exe_flags_cpp,
            link_exe_flags_rust,
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
