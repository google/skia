"""
THIS IS THE EXTERNAL-ONLY VERSION OF THIS FILE. G3 HAS ITS OWN.

This file contains a way to set flags from BUILD.bazel instead of requiring users to set them from
the CLI. This allows us to have tests that build with a specific set of features (e.g. the pdf
backend or the codecs) without having a large number of flags to set (easier to forget).

It uses the same core implementation as cc_binary_with_flags.

It is based off of:
https://github.com/bazelbuild/examples/blob/7fc3f8b587ee415ff02ce358caa960f9533a912c/configurations/cc_test/defs.bzl

"""

load("@skia_user_config//:copts.bzl", "DEFAULT_COPTS")
load("@skia_user_config//:linkopts.bzl", "DEFAULT_LINKOPTS")
load("//bazel:cc_binary_with_flags.bzl", "with_flags_transition")

def _transition_rule_impl(ctx):
    executable_src = ctx.executable.actual_test
    executable_dst = ctx.actions.declare_file(ctx.label.name)
    ctx.actions.run_shell(
        tools = [executable_src],
        outputs = [executable_dst],
        command = "cp %s %s" % (executable_src.path, executable_dst.path),
    )
    runfiles = ctx.attr.actual_test[0][DefaultInfo].default_runfiles
    return [DefaultInfo(runfiles = runfiles, executable = executable_dst)]

# This rule must end with a _test suffix or Bazel doesn't allow the test attribute to be true.
transition_test = rule(
    implementation = _transition_rule_impl,
    attrs = {
        # set_flags is a dictionary with the keys being the short-form of a flag name
        # (e.g. the part that comes after the colon) and the value being a list of values
        # that the flag should be set to, regardless of the relevant CLI flags.
        # https://bazel.build/rules/lib/attr#string_list_dict
        "set_flags": attr.string_list_dict(),
        # This is the cc_test that should be made with the flags being set.
        # Note specifically how it is modified using with_flags_transition, which
        # ensures that the flags propagates down the graph. Must be executable
        # so the _transition_rule_impl can use it as an executable.
        # https://bazel.build/rules/lib/attr#label
        "actual_test": attr.label(cfg = with_flags_transition, executable = True),
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
    # Means it works with "$ bazel test". https://bazel.build/rules/lib/globals#rule.test
    test = True,
)

def cc_test_with_flags(name, set_flags = {}, copts = DEFAULT_COPTS, linkopts = DEFAULT_LINKOPTS, args = [], **kwargs):
    """Builds a cc_test as if set_flags were set on the CLI.

    Args:
        name: string, the name for the rule that is the binary, but with the flags changed via
            a transition. Any dependents should use this name.
        set_flags: dictionary of string to list of strings. The keys should be the name of the
            flag, and the values should be the desired valid settings for that flag.
        copts: a list of strings or select statements that control the compiler flags.
            It has a sensible list of defaults.
        linkopts: a list of strings or select statements that control the linker flags.
            It has a sensible list of defaults.
        args: A list of strings with any command-line arguments to pass to the binary.
        **kwargs: Any flags that a cc_binary normally takes.
    """
    cc_test_name = name + "_native_test"
    transition_test(
        name = name,
        actual_test = ":%s" % cc_test_name,
        set_flags = set_flags,
        args = args,
        testonly = True,
    )
    tags = kwargs.get("tags", [])
    tags.append("manual")  # We want to exclude this helper test from bazel test foo/...
    kwargs["tags"] = tags
    native.cc_test(
        name = cc_test_name,
        copts = copts,
        linkopts = linkopts,
        **kwargs
    )
