"""
This file contains helpers for defining build flags and options that are used to
configure the Skia build.
"""

# https://github.com/bazelbuild/bazel-skylib/blob/main/rules/common_settings.bzl
load("@bazel_skylib//rules:common_settings.bzl", "string_flag", skylib_bool_flag = "bool_flag")

# Forked from https://github.com/bazelbuild/bazel-skylib/blob/main/rules/common_settings.bzl
BuildSettingInfo = provider(
    doc = "A singleton provider that contains the raw value of a multi-string build setting",
    fields = ["values"],
)

def _multi_string_impl(ctx):
    allowed_values = ctx.attr.values
    values = ctx.build_setting_value
    for v in values:
        if v not in ctx.attr.values:
            fail("Error setting " + str(ctx.label) + ": invalid value '" + v + "'. Allowed values are " + str(allowed_values))
    return BuildSettingInfo(values = values)

multi_string_flag = rule(
    implementation = _multi_string_impl,
    build_setting = config.string(flag = True, allow_multiple = True),
    attrs = {
        "values": attr.string_list(
            doc = "The list of allowed values for this setting. An error is raised if any other values are given.",
        ),
    },
    doc = "A string-typed build setting that can be set multiple times on the command line",
)

def string_flag_with_values(flag_name, values, default = "", multiple = False, name = ""):
    """Create a string flag and corresponding config_settings.

    string_flag_with_values is a Bazel Macro that defines a flag with the given name and a set
    of valid values for that flag. For each value, a config_setting is defined with the name
    of the value, associated with the created flag.
    This is defined to make the BUILD.bazel file easier to read w/o the boilerplate of defining
    a string_flag rule and n config_settings
    https://docs.bazel.build/versions/main/skylark/macros.html

    Args:
        flag_name: string, the name of the flag to create and use for the config_settings
        values: list of strings, the valid values for this flag to be set to.
        default: string, whatever the default value should be if the flag is not set. Can be
            empty string for both a string_flag and a multi_string flag.
        multiple: boolean, True if the flag should be able to be set multiple times on the CLI.
        name: string unused, https://github.com/bazelbuild/buildtools/blob/master/WARNINGS.md#unnamed-macro
    """
    if multiple:
        multi_string_flag(
            name = flag_name,
            # We have to specify a default value, even if that value is empty string.
            # https://docs.bazel.build/versions/main/skylark/config.html#instantiating-build-settings
            build_setting_default = default,
            # If empty string is the default, we need to make sure it is in the list
            # of acceptable values. If the default is not empty string, we don't want
            # to make empty string a valid value. Having duplicate values in the list
            # does not cause any issues, so we can just add the default to achieve
            # this affect.
            values = values + [default],
        )
    else:
        string_flag(
            name = flag_name,
            # We have to specify a default value, even if that value is empty string.
            # https://docs.bazel.build/versions/main/skylark/config.html#instantiating-build-settings
            build_setting_default = default,
            # If empty string is the default, we need to make sure it is in the list
            # of acceptable values. If the default is not empty string, we don't want
            # to make empty string a valid value. Having duplicate values in the list
            # does not cause any issues, so we can just add the default to achieve
            # this affect.
            values = values + [default],
        )

    # For each of the values given, we define a config_setting. This allows us to use
    # select statements, on the given setting, e.g. referencing
    # //bazel/common_config_settings:some_valid_value_for_a_flag
    for v in values:
        native.config_setting(
            name = v,
            flag_values = {
                ":" + flag_name: v,
            },
        )

def bool_flag(flag_name, default = True, name = ""):
    """Create a boolean flag and corresponding config_settings.

    bool_flag is a Bazel Macro that defines a boolean flag with the given name two config_settings,
    one for True, one for False. Reminder that Bazel has special syntax for unsetting boolean flags,
    but this does not work well with aliases.
    https://docs.bazel.build/versions/main/skylark/config.html#using-build-settings-on-the-command-line
    Thus it is best to define both an "enabled" alias and a "disabled" alias.

    Args:
        flag_name: string, the name of the flag to create and use for the config_settings
        default: boolean, if the flag should default to on or off.
        name: string unused, https://github.com/bazelbuild/buildtools/blob/master/WARNINGS.md#unnamed-macro
    """
    skylib_bool_flag(name = flag_name, build_setting_default = default)

    native.config_setting(
        name = flag_name + "_true",
        flag_values = {
            # The value must be a string, but it will be parsed to a boolean
            # https://docs.bazel.build/versions/main/skylark/config.html#build-settings-and-select
            ":" + flag_name: "True",
        },
    )

    native.config_setting(
        name = flag_name + "_false",
        flag_values = {
            ":" + flag_name: "False",
        },
    )
