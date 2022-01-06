"""
This file contains a way to set flags from BUILD.bazel instead of requiring users to set them from
the CLI.

It is based off of https://github.com/bazelbuild/examples/tree/main/rules/starlark_configurations/cc_binary_selectable_copts

"""

_bool_flags = [
    "//bazel/common_config_settings:use_icu",
]

_string_flags = [
    "//bazel/common_config_settings:fontmgr_factory",
    "//bazel/common_config_settings:with_gl_standard",
]

_string_list_flags = [
    "//bazel/common_config_settings:gpu_backend",
    "//bazel/common_config_settings:include_decoder",
    "//bazel/common_config_settings:include_encoder",
    "//bazel/common_config_settings:include_fontmgr",
    "//bazel/common_config_settings:shaper_backend",
]

# These are the flags that we support setting via set_flags
_flags = _bool_flags + _string_flags + _string_list_flags

def _flag_transition_impl(settings, attr):
    rv = {}
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
                rv[key] = flag_setting[0] == "True"
            else:
                rv[key] = flag_setting  # flag_setting will be a boolean, the default
    return rv

# This defines a Starlark transition and which flags it reads and writes.
_flag_transition = transition(
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
            data_runfiles = actual_binary[DefaultInfo].data_runfiles,
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
        # https://docs.bazel.build/versions/main/skylark/lib/attr.html#string_list_dict
        "set_flags": attr.string_list_dict(),
        # This is the cc_binary whose deps will select() on that feature.
        # Note specifically how it is modified with _flag_transition, which
        # ensures that the flags propagates down the graph.
        # https://docs.bazel.build/versions/main/skylark/lib/attr.html#label
        "actual_binary": attr.label(cfg = _flag_transition),
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

def cc_binary_with_flags(name, set_flags = {}, **kwargs):
    """Builds a cc_binary as if set_flags were set on the CLI.

    Args:
        name: string, the name for the rule that is the binary, but with the flags changed via
            a transition. Any dependents should use this name.
        set_flags: dictionary of string to list of strings. The keys should be the name of the
            flag, and the values should be the desired valid settings for that flag.
        **kwargs: Any flags that a cc_binary normally takes.
    """
    cc_binary_name = name + "_native_binary"
    transition_rule(
        name = name,
        actual_binary = ":%s" % cc_binary_name,
        set_flags = set_flags,
        testonly = kwargs.get("testonly", False),
    )
    native.cc_binary(
        name = cc_binary_name,
        **kwargs
    )
