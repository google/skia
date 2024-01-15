"""
THIS IS THE EXTERNAL-ONLY VERSION OF THIS FILE. G3 HAS ITS OWN.

This file contains a way to set flags from BUILD.bazel instead of requiring users to set them from
the CLI.

It is based off of https://github.com/bazelbuild/examples/tree/main/rules/starlark_configurations/cc_binary_selectable_copts

"""

load("@skia_user_config//:copts.bzl", "DEFAULT_COPTS")
load("@skia_user_config//:linkopts.bzl", "DEFAULT_LINKOPTS")

_bool_flags = [
    "//bazel/common_config_settings:use_harfbuzz",
    "//bazel/common_config_settings:use_fontations",
    "//bazel/common_config_settings:use_icu",
    "//src/gpu/ganesh/vk:enable_secondary_draw_context",
    "//src/gpu:enable_gpu_test_utils",
    "//src/lazy:enable_discardable_memory",
    "//src/lazy:use_default_global_memory_pool",
    "//src/pdf:enable_pdf_backend",
    "//src/sksl:enable_sksl_tracing",
    "//src/sksl:enable_skslc",
    "//src/svg:enable_svg_canvas",
]

_string_flags = [
    "//src/gpu:with_gl_standard",
    "//tools/testrunners/gm/vias:via",
]

_string_list_flags = [
    "//src/gpu:gpu_backend",
    "//src/codec:include_decoder",
    "//src/encode:include_encoder",
    "//bazel/common_config_settings:include_fontmgr",
]

# These are the flags that we support setting via set_flags
_flags = _bool_flags + _string_flags + _string_list_flags
_short_flags = [long_flag.split(":")[1] for long_flag in _flags]

def _flag_transition_impl(settings, attr):
    rv = {}
    for flag in attr.set_flags:
        if flag not in _short_flags:
            fail("unknown flag " + flag)

    for key in settings:
        # Get the short form of the name. This the short form used as the keys in the
        # set_flags dictionary.
        flag_name = key.split(":")[1]

        # If there is an entry in set_flags for the short-version of a flag, use that
        # value or values. If not, use whatever value is set via flags.
        flag_setting = attr.set_flags.get(flag_name, settings[key])
        if key in _string_list_flags:
            if type(flag_setting) == "list":
                rv[key] = flag_setting
            else:
                rv[key] = [flag_setting]  # This usually happens when the default value is used.
        elif key in _string_flags:
            if type(flag_setting) == "list":
                rv[key] = flag_setting[0]
            else:
                rv[key] = flag_setting  # we know flag_setting is a string (e.g. the default).
        elif key in _bool_flags:
            if type(flag_setting) == "list":
                rv[key] = flag_setting[0].lower() == "true"
            else:
                rv[key] = flag_setting  # flag_setting will be a boolean, the default
    return rv

# This defines a Starlark transition and which flags it reads and writes.
with_flags_transition = transition(
    implementation = _flag_transition_impl,
    inputs = _flags,
    outputs = _flags,
)

# The implementation of transition_rule: all this does is copy the cc_binary's output to
# its own output and propagate its runfiles and executable to use for "$ bazel run".
#
# This makes transition_rule as close to a pure wrapper of cc_binary as possible.
def _transition_rule_impl(ctx):
    actual_binary = ctx.attr.actual_binary[0]
    outfile = ctx.actions.declare_file(ctx.label.name)
    cc_binary_outfile = actual_binary[DefaultInfo].files.to_list()[0]

    ctx.actions.run_shell(
        inputs = [cc_binary_outfile],
        outputs = [outfile],
        command = "cp %s %s" % (cc_binary_outfile.path, outfile.path),
    )
    return [
        DefaultInfo(
            executable = outfile,
            runfiles = actual_binary[DefaultInfo].data_runfiles,
        ),
    ]

# The purpose of this rule is to take a "set_flags" attribute, invoke a transition that sets
# any of _flags to the specified values, then depend on a cc_binary whose deps will be able
# to select() on those flags as if the user had set them via the CLI.
transition_rule = rule(
    implementation = _transition_rule_impl,
    attrs = {
        # set_flags is a dictionary with the keys being the short-form of a flag name
        # (e.g. the part that comes after the colon) and the value being a list of values
        # that the flag should be set to, regardless of the relevant CLI flags.
        # https://bazel.build/rules/lib/attr#string_list_dict
        "set_flags": attr.string_list_dict(),
        # This is the cc_binary whose deps will select() on that feature.
        # Note specifically how it is modified with _flag_transition, which
        # ensures that the flags propagates down the graph.
        # https://bazel.build/rules/lib/attr#label
        "actual_binary": attr.label(cfg = with_flags_transition),
        # This is a stock Bazel requirement for any rule that uses Starlark
        # transitions. It's okay to copy the below verbatim for all such rules.
        #
        # The purpose of this requirement is to give the ability to restrict
        # which packages can invoke these rules, since Starlark transitions
        # make much larger graphs possible that can have memory and performance
        # consequences for your build. The allowlist defaults to "everything".
        # But you can redefine it more strictly if you feel that's prudent.
        "_allowlist_function_transition": attr.label(
            default = "@bazel_tools//tools/allowlists/function_transition_allowlist",
        ),
    },
    # Making this executable means it works with "$ bazel run".
    executable = True,
)

def cc_binary_with_flags(name, set_flags = {}, copts = DEFAULT_COPTS, linkopts = DEFAULT_LINKOPTS, **kwargs):
    """Builds a cc_binary as if set_flags were set on the CLI.

    Args:
        name: string, the name for the rule that is the binary, but with the flags changed via
            a transition. Any dependents should use this name.
        set_flags: dictionary of string to list of strings. The keys should be the name of the
            flag, and the values should be the desired valid settings for that flag.
        copts: a list of strings or select statements that control the compiler flags.
            It has a sensible list of defaults.
        linkopts: a list of strings or select statements that control the linker flags.
            It has a sensible list of defaults.
        **kwargs: Any flags that a cc_binary normally takes.
    """
    cc_binary_name = name + "_native_binary"
    transition_rule(
        name = name,
        actual_binary = ":%s" % cc_binary_name,
        set_flags = set_flags,
        testonly = kwargs.get("testonly", False),
    )
    tags = kwargs.get("tags", [])
    tags.append("manual")  # We want to exclude this helper binary from bazel build foo/...
    kwargs["tags"] = tags
    native.cc_binary(
        name = cc_binary_name,
        copts = copts,
        linkopts = linkopts,
        **kwargs
    )
