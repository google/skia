"""
This file specifies a clang toolchain that can run on a Mac host (with either M1 or Intel CPU).

Hermetic toolchains still need access to Xcode for sys headers included in Skia's codebase.

See download_mac_toolchain.bzl for more details on the creation of the toolchain.

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
load(":clang_layering_check.bzl", "make_layering_check_features")

# The location of the created clang toolchain.
EXTERNAL_TOOLCHAIN = "external/clang_mac"

# Root of our symlinks. These symlinks are created in download_mac_toolchain.bzl
XCODE_MACSDK_SYMLINK = EXTERNAL_TOOLCHAIN + "/symlinks/xcode/MacSDK"

_platform_constraints_to_import = {
    "@platforms//cpu:arm64": "_arm64_cpu",
    "@platforms//cpu:x86_64": "_x86_64_cpu",
}

def _mac_toolchain_info(ctx):
    action_configs = _make_action_configs()
    features = []
    features += _make_default_flags()
    features += make_layering_check_features()
    features += _make_diagnostic_flags()
    features += _make_target_specific_flags(ctx)

    # https://bazel.build/rules/lib/cc_common#create_cc_toolchain_config_info
    # Note, this rule is defined in Java code, not Starlark
    # https://cs.opensource.google/bazel/bazel/+/master:src/main/java/com/google/devtools/build/lib/starlarkbuildapi/cpp/CcModuleApi.java
    return cc_common.create_cc_toolchain_config_info(
        ctx = ctx,
        features = features,
        action_configs = action_configs,
        builtin_sysroot = EXTERNAL_TOOLCHAIN,
        cxx_builtin_include_directories = [
            # https://stackoverflow.com/a/61419490
            # "If the compiler has --sysroot support, then these paths should use %sysroot%
            #  rather than the include path"
            # https://bazel.build/rules/lib/cc_common#create_cc_toolchain_config_info.cxx_builtin_include_directories
            "%sysroot%/symlinks/xcode/MacSDK/Frameworks/",
        ],
        # If `ctx.attr.cpu` is blank (which is declared as optional below), this config will target
        # the host CPU. Specifying a target_cpu allows this config to be used for cross compilation.
        target_cpu = ctx.attr.cpu,
        # These are required, but do nothing
        compiler = "",
        target_libc = "",
        target_system_name = "",
        toolchain_identifier = "",
    )

def _import_platform_constraints():
    # In order to "import" constraint values so they can be passed in as parameters to
    # ctx.target_platform_has_constraint(), we need to list them as a default value on a
    # private attributes. It doesn't really matter what we call these private attributes,
    # but to make it easier to read elsewhere, we create a mapping between the "official"
    # name of the constraints and the private name. Then, we can refer to the official name
    # without having to remember the secondary name.
    # https://bazel.build/rules/rules#private_attributes_and_implicit_dependencies
    # https://github.com/bazelbuild/proposals/blob/91579f36031f768bcf68b18a86b8df8b43cc590b/designs/2019-11-11-target-platform-constraints.md
    rule_attributes = {}
    for constraint in _platform_constraints_to_import:
        private_attr = _platform_constraints_to_import[constraint]
        rule_attributes[private_attr] = attr.label(default = constraint)

    # Define an optional attribute to allow the target architecture to be explicitly specified (e.g.
    # when selecting a cross-compilation toolchain).
    rule_attributes["cpu"] = attr.string(
        mandatory = False,
        values = ["arm64", "x64"],
    )
    return rule_attributes

def _has_platform_constraint(ctx, official_constraint_name):
    # ctx is of type https://bazel.build/rules/lib/ctx
    # This pattern is from
    # https://github.com/bazelbuild/proposals/blob/91579f36031f768bcf68b18a86b8df8b43cc590b/designs/2019-11-11-target-platform-constraints.md
    private_attr = _platform_constraints_to_import[official_constraint_name]
    constraint = getattr(ctx.attr, private_attr)[platform_common.ConstraintValueInfo]
    return ctx.target_platform_has_constraint(constraint)

provide_mac_toolchain_config = rule(
    attrs = _import_platform_constraints(),
    provides = [CcToolchainConfigInfo],
    implementation = _mac_toolchain_info,
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
    # https://bazel.build/docs/cc-toolchain-config-reference#cctoolchainconfiginfo-build-variables
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
#
# Note: These values must be kept in sync with those defined in cmake_exporter.go.
def _make_default_flags():
    """Here we define the flags for certain actions that are always applied.

    For any flag that might be conditionally applied, it should be defined in //bazel/copts.bzl.

    Flags that are set here will be unconditionally applied to everything we compile with
    this toolchain, even third_party deps.

    """
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
                    XCODE_MACSDK_SYMLINK + "/usr/include",
                    "-isystem",
                    EXTERNAL_TOOLCHAIN + "/lib/clang/15.0.1/include",
                    # Set the framework path to the Mac SDK framework directory. This has
                    # subfolders like OpenGL.framework
                    # We want -iframework so Clang hides diagnostic warnings from those header
                    # files we include. -F does not hide those.
                    "-iframework",
                    XCODE_MACSDK_SYMLINK + "/Frameworks",
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
            ACTION_NAMES.objc_compile,
            ACTION_NAMES.objcpp_compile,
        ],
        flag_groups = [
            flag_group(
                flags = [
                    "-std=c++17",
                ],
            ),
        ],
    )

    # copts and defines appear to not automatically be set
    # https://bazel.build/docs/cc-toolchain-config-reference#cctoolchainconfiginfo-build-variables
    # https://github.com/bazelbuild/bazel/blob/5ad4a6126be2bdc53ee7e2457e076c90efe86d56/tools/cpp/cc_toolchain_config_lib.bzl#L200-L209
    objc_compile_flags = flag_set(
        actions = [
            ACTION_NAMES.objc_compile,
            ACTION_NAMES.objcpp_compile,
        ],
        flag_groups = [
            flag_group(
                iterate_over = "user_compile_flags",
                flags = ["%{user_compile_flags}"],
            ),
            flag_group(
                iterate_over = "preprocessor_defines",
                flags = ["-D%{preprocessor_defines}"],
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
                    # lld goes through dynamic library dependencies for dylib and tbh files through
                    # absolute paths (/System/Library/Frameworks). However, the dependencies live in
                    # [Xcode dir]/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks
                    # -Wl tells clang to forward the next flag to the linker.
                    # -syslibroot appends to the beginning of the dylib dependency path.
                    # https://github.com/llvm/llvm-project/blob/d61341768cf0cff7ceeaddecc2f769b5c1b901c4/lld/MachO/InputFiles.cpp#L1418-L1420
                    "-Wl,-syslibroot",
                    XCODE_MACSDK_SYMLINK,
                    "-fuse-ld=lld",
                    "-std=c++17",
                    "-stdlib=libc++",
                    EXTERNAL_TOOLCHAIN + "/lib/libc++.a",
                    EXTERNAL_TOOLCHAIN + "/lib/libc++abi.a",
                    EXTERNAL_TOOLCHAIN + "/lib/libunwind.a",
                ],
            ),
        ],
    )

    return [feature(
        "default_flags",
        enabled = True,
        flag_sets = [
            cpp_compile_flags,
            cxx_compile_includes,
            link_exe_flags,
            objc_compile_flags,
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

# The parameter is of type https://bazel.build/rules/lib/ctx
def _make_target_specific_flags(ctx):
    m1_mac_target = flag_set(
        actions = [
            ACTION_NAMES.assemble,
            ACTION_NAMES.preprocess_assemble,
            ACTION_NAMES.c_compile,
            ACTION_NAMES.cpp_compile,
            ACTION_NAMES.objc_compile,
            ACTION_NAMES.objcpp_compile,
            ACTION_NAMES.cpp_link_executable,
            ACTION_NAMES.cpp_link_dynamic_library,
        ],
        flag_groups = [
            flag_group(
                flags = [
                    "--target=arm64-apple-macos12",
                ],
            ),
        ],
    )
    intel_mac_target = flag_set(
        actions = [
            ACTION_NAMES.assemble,
            ACTION_NAMES.preprocess_assemble,
            ACTION_NAMES.c_compile,
            ACTION_NAMES.cpp_compile,
            ACTION_NAMES.objc_compile,
            ACTION_NAMES.objcpp_compile,
            ACTION_NAMES.cpp_link_executable,
            ACTION_NAMES.cpp_link_dynamic_library,
        ],
        flag_groups = [
            flag_group(
                flags = [
                    "--target=x86_64-apple-macos12",
                ],
            ),
        ],
    )

    target_specific_features = []
    if _has_platform_constraint(ctx, "@platforms//cpu:arm64"):
        target_specific_features.append(
            feature(
                name = "_m1_mac_target",
                enabled = True,
                flag_sets = [m1_mac_target],
            ),
        )
    elif _has_platform_constraint(ctx, "@platforms//cpu:x86_64"):
        target_specific_features.append(
            feature(
                name = "_intel_mac_target",
                enabled = True,
                flag_sets = [intel_mac_target],
            ),
        )

    return target_specific_features
