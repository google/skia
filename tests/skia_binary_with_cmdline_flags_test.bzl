"""This module defines the skia_binary_with_cmdline_flags_test rule."""

load("//bazel:remove_indentation.bzl", "remove_indentation")

# https://bazel.build/rules/lib/builtins/ctx
def _skia_binary_with_cmdline_flags_test_impl(ctx):
    test_args = ([
        "--resourcePath",
        "$(dirname $(realpath $(rootpath %s)))" % ctx.attr._arbitrary_file_in_resources_dir.label,
    ] if ctx.attr.requires_resources_dir else []) + ctx.attr.extra_args

    template = remove_indentation("""
        #!/bin/sh
        $(rootpath {test_binary}) {test_args}
    """)

    template = ctx.expand_location(template.format(
        test_binary = ctx.attr.test_binary.label,
        test_args = " ".join(test_args),
    ), targets = [
        ctx.attr.test_binary,
        ctx.attr._arbitrary_file_in_resources_dir,
    ])

    # https://bazel.build/rules/lib/builtins/actions.html#declare_file
    output_file = ctx.actions.declare_file(ctx.attr.name)
    ctx.actions.write(output_file, template, is_executable = True)

    runfiles = ctx.runfiles(
        files = ctx.files._resources_dir if ctx.attr.requires_resources_dir else [],
    )
    runfiles = runfiles.merge(ctx.attr.test_binary[DefaultInfo].default_runfiles)

    return [DefaultInfo(
        executable = output_file,
        runfiles = runfiles,
    )]

skia_binary_with_cmdline_flags_test = rule(
    doc = """Runs a C++ test binary with any necessary command-line flags.

    This test rule produces a wrapper script that invokes a C++ test with a predetermined set of
    command-line flags.

    The reason why we use a custom rule rather than a genrule is that we wish to select() the
    extra_args attribute based e.g. on the device under test and various build settings.
    """,
    implementation = _skia_binary_with_cmdline_flags_test_impl,
    attrs = {
        "test_binary": attr.label(
            mandatory = True,
            executable = True,
            allow_single_file = True,
            cfg = "target",
        ),
        "requires_resources_dir": attr.bool(
            doc = (
                "If true, the test will be passed flag --resourcePath <path to //resources dir>."
            ),
        ),
        "extra_args": attr.string_list(
            doc = "Any additional command-line arguments to pass to the C++ test binary.",
        ),
        "_resources_dir": attr.label(
            doc = "This directory will optionally be added to the test's runfiles.",
            default = Label("//resources"),
        ),
        "_arbitrary_file_in_resources_dir": attr.label(
            doc = "Used to compute the --resourcePath flag.",
            default = Label("//resources:README"),
            allow_single_file = True,
        ),
    },
    test = True,
)
