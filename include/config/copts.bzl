"""
THIS IS THE EXTERNAL-ONLY VERSION OF THIS FILE. G3 HAS ITS OWN.

This file contains flags for the C++ compiler, referred to by Bazel as copts.

The copts in a cc_library to not flow to the children (dependencies) nor the parents
(dependents), so we use skia_cc_library to inject them in every library along the chain.

Now that we have a modular build, this file could maybe go away and folded into our toolchains.

They are divided into several lists/selects and were initially created to be identical to the
 GN ones [2][3].

[2] https://github.com/google/skia/blob/2b07cdb07e88f2870260eabac708f31bc7977d08/gn/BUILDCONFIG.gn#L177-L181
[3] https://github.com/google/skia/blob/2b07cdb07e88f2870260eabac708f31bc7977d08/gn/skia/BUILD.gn#L593-L630
"""

CORE_COPTS = [
    "-fstrict-aliasing",
] + select({
    "@platforms//os:android": [],
    "//conditions:default": [
        # On Android, this option causes the linker to fail
        # (e.g. "undefined reference to `SkString::data()'").
        "-fvisibility=hidden",
    ],
}) + select({
    "@platforms//os:windows": [],
    "//conditions:default": [
        # In Clang 14, this default was changed. We turn this off to (hopefully) make our
        # GMs more consistent and avoid some floating-point related test failures on M1 macs.
        "-ffp-contract=off",
        # Windows doesn't support position-independent code.
        "-fPIC",
    ],
}) + select({
    # Turning off RTTI reduces code size, but is necessary for connecting C++
    # and Objective-C code.
    "@platforms//os:macos": [],
    "@platforms//os:ios": [],
    "//conditions:default": [
        "-fno-rtti",
    ],
})

OPT_LEVEL = select({
    "//bazel/common_config_settings:debug_build": [
        "--optimize=0",
        "--debug",
    ],
    "//bazel/common_config_settings:release_build": [
        "--optimize=3",
        # Strip dead code (in conjunction with linkopts)
        "-fdata-sections",
        "-ffunction-sections",
    ],
    "//bazel/common_config_settings:fast_build": [
        "--optimize=0",
        "-gline-tables-only",
    ],
})

WARNINGS = [
    "-fcolor-diagnostics",
    "-Wall",
    "-Werror",
    "-Weverything",
    "-Wextra",
    "-Wpointer-arith",
    "-Wsign-compare",
    "-Wvla",
    #### Warnings we are unlikely to fix ####
    "-Wno-c++98-compat",
    "-Wno-c++98-compat-pedantic",
    "-Wno-covered-switch-default",
    "-Wno-declaration-after-statement",
    "-Wno-deprecated",
    "-Wno-missing-noreturn",
    "-Wno-newline-eof",
    "-Wno-old-style-cast",
    "-Wno-padded",
    "-Wno-psabi",  # noisy
    "-Wno-return-std-move-in-c++11",
    "-Wno-shadow-field-in-constructor",
    "-Wno-shadow-uncaptured-local",
    "-Wno-undefined-func-template",
    "-Wno-unused-parameter",  # It is common to have unused parameters in src/
    "-Wno-zero-as-null-pointer-constant",  # VK_NULL_HANDLE is defined as 0
    "-Wno-unsafe-buffer-usage",
    #### Warnings we would like to fix ####
    "-Wno-abstract-vbase-init",
    "-Wno-cast-align",
    "-Wno-cast-function-type-strict",
    "-Wno-cast-qual",
    "-Wno-class-varargs",
    "-Wno-conversion",  # -Wsign-conversion re-enabled for header sources
    "-Wno-disabled-macro-expansion",
    "-Wno-documentation",
    "-Wno-documentation-unknown-command",
    "-Wno-double-promotion",
    "-Wno-exit-time-destructors",  # TODO: OK outside libskia
    "-Wno-float-equal",
    "-Wno-global-constructors",  # TODO: OK outside libskia
    "-Wno-missing-prototypes",
    "-Wno-missing-variable-declarations",
    "-Wno-pedantic",
    "-Wno-reserved-id-macro",
    "-Wno-reserved-identifier",
    "-Wno-shift-sign-overflow",
    "-Wno-signed-enum-bitfield",
    "-Wno-switch-enum",
    "-Wno-thread-safety-negative",
    "-Wno-undef",
    "-Wno-unreachable-code-break",
    "-Wno-unreachable-code-return",
    "-Wno-unused-macros",
    "-Wno-unused-member-function",
    "-Wno-weak-template-vtables",  # This was deprecated in Clang 14 and removed in Clang 15.
    "-Wno-weak-vtables",
    # https://quuxplusone.github.io/blog/2020/08/26/wrange-loop-analysis/
    # https://bugzilla.mozilla.org/show_bug.cgi?id=1683213
    # https://reviews.llvm.org/D73007
    # May be re-enabled once clang > 12 or XCode > 12 are required.
    # When this line is removed the -Wrange-loop-construct line below can also be removed.
    "-Wno-range-loop-analysis",
    # Wno-range-loop-analysis turns off the whole group, but this warning was later split into
    # range-loop-construct and range-loop-bind-reference. We want the former but not the latter.
    # Created from
    # https://github.com/llvm/llvm-project/blob/bd08f413c089da5a56438cc8902f60df91a08a66/clang/include/clang/Basic/DiagnosticGroups.td
    "-Wrange-loop-construct",
    # Wno-deprecated turns off the whole group, but also has its own warnings like
    # out-of-line definition of constexpr static data member is redundant in C++17 and is deprecated [-Werror,-Wdeprecated]
    # but we would like others. Created from
    # https://github.com/llvm/llvm-project/blob/bd08f413c089da5a56438cc8902f60df91a08a66/clang/include/clang/Basic/DiagnosticGroups.td
    "-Wdeprecated-anon-enum-enum-conversion",
    "-Wdeprecated-array-compare",
    "-Wdeprecated-attributes",
    "-Wdeprecated-comma-subscript",
    "-Wdeprecated-copy",
    "-Wdeprecated-copy-dtor",
    "-Wdeprecated-dynamic-exception-spec",
    "-Wdeprecated-enum-compare",
    "-Wdeprecated-enum-compare-conditional",
    "-Wdeprecated-enum-enum-conversion",
    "-Wdeprecated-enum-float-conversion",
    "-Wdeprecated-increment-bool",
    "-Wdeprecated-register",
    "-Wdeprecated-this-capture",
    "-Wdeprecated-volatile",
    "-Wdeprecated-writable-strings",
    # A catch-all for when the version of clang we are using does not have the prior options
    "-Wno-unknown-warning-option",
] + select({
    "@platforms//os:windows": [
        # skbug.com/14203
        "-Wno-nonportable-system-include-path",
        "-Wno-unknown-argument",
        # Clang warns even when all enum values are covered.
        "-Wno-switch-default",
        # The Windows build fails without these options:
        "-Wno-extra-semi-stmt",
        "-Wno-invalid-constexpr",
        "-D_CRT_USE_BUILTIN_OFFSETOF",
    ],
    "//conditions:default": [],
})

DEFAULT_COPTS = CORE_COPTS + OPT_LEVEL + WARNINGS

OBJC_COPTS = [
    "-Wno-direct-ivar-access",
    "-Wno-objc-interface-ivars",
]

DEFAULT_OBJC_COPTS = DEFAULT_COPTS + OBJC_COPTS
