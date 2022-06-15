"""
This file specifies a clang toolchain that can run on a Mac host.

Hermetic toolchains still need access to Xcode for sys headers included in Skia's codebase.

See download_mac_m1_toolchain.bzl for more details on the creation of the toolchain.

It uses the usr subfolder of the built toolchain as a sysroot

It follows the example of:
 - lunix_amd64_toolchain_config.bzl
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

# https://github.com/bazelbuild/bazel/blob/master/tools/build_defs/cc/action_names.bzl
load("@bazel_tools//tools/build_defs/cc:action_names.bzl", "ACTION_NAMES")

# The location of the created clang toolchain.
EXTERNAL_TOOLCHAIN = "external/clang_mac_m1"

# Symlink location.
# Must be the same as where the symlink points to in download_mac_m1_toolchain.bzl
XCODE_SYMLINK = EXTERNAL_TOOLCHAIN + "/symlinks/xcode/MacSDK/usr"

def _mac_m1_toolchain_info(ctx):
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
        builtin_sysroot = EXTERNAL_TOOLCHAIN,
        compiler = "clang",
        host_system_name = "local",
        target_cpu = "m1",
        target_system_name = "local",
        # does this matter?
        target_libc = "glibc-2.31",
        toolchain_identifier = "clang-toolchain",
    )

provide_mac_m1_toolchain_config = rule(
    attrs = {},
    provides = [CcToolchainConfigInfo],
    implementation = _mac_m1_toolchain_info,
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
    clang_tool = tool(path = "mac_trampolines/clang_trampoline_mac.sh")
    lld_tool = tool(path = "mac_trampolines/lld_trampoline_mac.sh")
    ar_tool = tool(path = "mac_trampolines/ar_trampoline_mac.sh")

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
    objc_compile_action = action_config(
        action_name = ACTION_NAMES.objc_compile,
        tools = [clang_tool],
    )
    objcpp_compile_action = action_config(
        action_name = ACTION_NAMES.objcpp_compile,
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

    # objc archiver and cpp archiver actions use the same base flags
    common_archive_flags = [
        flag_set(
            flag_groups = [
                flag_group(
                    # https://llvm.org/docs/CommandGuide/llvm-ar.html
                    # [r]eplace existing files or insert them if they already exist,
                    # [c]reate the file if it doesn't already exist
                    # [s]ymbol table should be added
                    # [D]eterministic timestamps should be used
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
    ]

    # This is the same rule as
    # https://github.com/emscripten-core/emsdk/blob/7f39d100d8cd207094decea907121df72065517e/bazel/emscripten_toolchain/crosstool.bzl#L143
    # By default, there are no flags or libraries passed to the llvm-ar tool, so
    # we need to specify them. The variables mentioned by expand_if_available are defined
    # https://docs.bazel.build/versions/main/cc-toolchain-config-reference.html#cctoolchainconfiginfo-build-variables
    cpp_link_static_library_action = action_config(
        action_name = ACTION_NAMES.cpp_link_static_library,
        flag_sets = common_archive_flags,
        tools = [ar_tool],
    )

    objc_archive_action = action_config(
        action_name = ACTION_NAMES.objc_archive,
        flag_sets = common_archive_flags,
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
        objc_archive_action,
        objc_compile_action,
        objcpp_compile_action,
        preprocess_assemble_action,
    ]
    return action_configs

# In addition to pointing the c and cpp compile actions to our toolchain, we also need to set objc
# and objcpp action flags as well. We build .m and .mm files with the objc_library rule, which
# will use the default toolchain if not specified here.
# https://docs.bazel.build/versions/3.3.0/be/objective-c.html#objc_library
def _make_default_flags():
    """Here we define the flags for certain actions that are always applied."""
    cxx_compile_includes = flag_set(
        actions = [
            ACTION_NAMES.c_compile,
            ACTION_NAMES.cpp_compile,
            ACTION_NAMES.objc_compile,
            ACTION_NAMES.objcpp_compile,
        ],
        flag_groups = [
            flag_group(
                flags = [
                    # THIS ORDER MATTERS GREATLY. If these are in the wrong order, the
                    # #include_next directives will fail to find the files, causing a compilation
                    # error (or, without -no-canonical-prefixes, a mysterious case where files
                    # are included with an absolute path and fail the build).
                    "-isystem",
                    EXTERNAL_TOOLCHAIN + "/include/c++/v1",
                    "-isystem",
                    XCODE_SYMLINK + "/include",
                    "-isystem",
                    EXTERNAL_TOOLCHAIN + "/lib/clang/13.0.0/include",
                    # Set the framework path to the Mac SDK framework directory. We can't include it
                    # through XCODE_SYMLINK because of infinite symlink recursion introduced in the
                    # framework folder.
                    # TODO(jmbetancourt): feed this path similarly to how we used xcode-select
                    # idea: set this in the trampoline script
                    "-F",
                    "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks",
                    # We do not want clang to search in absolute paths for files. This makes
                    # Bazel think we are using an outside resource and fail the compile.
                    "-no-canonical-prefixes",
                    "-Wno-deprecated-declarations",
                ],
            ),
        ],
    )

    cpp_compile_includes = flag_set(
        actions = [
            ACTION_NAMES.cpp_compile,
            ACTION_NAMES.objc_compile,
            ACTION_NAMES.objcpp_compile,
        ],
        flag_groups = [
            flag_group(
                flags = [
                    "-std=c++17",
                    "-Wno-psabi",  # noisy
                ],
            ),
        ],
    )

    link_exe_flags = flag_set(
        actions = [ACTION_NAMES.cpp_link_executable],
        flag_groups = [
            flag_group(
                flags = [
                    # lld goes through dynamic library dependencies for dylib and tbh files through
                    # absolute paths (/System/Library/Frameworks). However, the dependencies live in
                    # [Xcode dir]/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks
                    # -syslibroot appends to the beginning of the dylib dependency path.
                    # https://github.com/llvm/llvm-project/blob/d61341768cf0cff7ceeaddecc2f769b5c1b901c4/lld/MachO/InputFiles.cpp#L1418-L1420
                    "-Wl,-syslibroot",
                    "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/",
                    "-fuse-ld=lld",
                    # We chose to use the llvm runtime, not the gcc one because it is already
                    # included in the clang binary
                    "--rtlib=compiler-rt",
                    "-std=c++17",
                    # Tell the linker where to look for libraries.
                    "-L",
                    XCODE_SYMLINK + "/lib",
                    # We statically include these libc++ libraries so they do not need to be
                    # on a developer's machine (they can be tricky to get).
                    EXTERNAL_TOOLCHAIN + "/lib/libc++.a",
                    EXTERNAL_TOOLCHAIN + "/lib/libc++abi.a",
                    EXTERNAL_TOOLCHAIN + "/lib/libunwind.a",
                    # Dynamically Link in the other parts of glibc (not needed in glibc 2.34+)
                    "-lpthread",
                    "-lm",
                    "-ldl",
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
