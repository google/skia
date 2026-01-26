"""This module defines the binary_wrapper_script_with_cmdline_flags rule."""

load("//bazel:remove_indentation.bzl", "remove_indentation")

# https://bazel.build/rules/lib/builtins/ctx
def _binary_wrapper_script_with_cmdline_flags_impl(ctx):
    args = ([
        "--resourcePath",
        "$(dirname $(realpath $(rootpath %s)))" % ctx.attr._arbitrary_file_in_resources_dir.label,
    ] if ctx.attr.requires_resources_dir else [])
    args += ctx.attr.extra_args

    # This will gather any additional command-line arguments passed to the C++ binary. When running
    # the binary via "bazel test", additional arguments can be passed via Bazel's --test_arg flag.
    # When building the binary with "bazel build" and running the resulting binary outside of
    # Bazel, any arguments passed to the Bazel-built binary will be captured here.
    args.append("$@")

    template = remove_indentation("""
        #!/bin/sh
        $(rootpath {binary}) {args}
    """)

    template = ctx.expand_location(template.format(
        binary = ctx.attr.binary.label,
        args = " ".join(args),
    ), targets = [
        ctx.attr.binary,
        ctx.attr._arbitrary_file_in_resources_dir,
    ])

    # https://bazel.build/rules/lib/builtins/actions.html#declare_file
    output_file = ctx.actions.declare_file(ctx.attr.name)
    ctx.actions.write(output_file, template, is_executable = True)

    runfiles = ctx.runfiles(
        files = ctx.files._resources_dir if ctx.attr.requires_resources_dir else [],
    )
    runfiles = runfiles.merge(ctx.attr.binary[DefaultInfo].default_runfiles)

    return [DefaultInfo(
        executable = output_file,
        runfiles = runfiles,
    )]

binary_wrapper_script_with_cmdline_flags = rule(
    doc = """Produces a script that invokes a C++ test runner with the given command-line flags.

    This rule is intended to wrap C++ test runners and therefore has convenience attributes
    specific to said binaries, such as requires_resources_dir.

    The reason why we use a custom rule rather than a genrule is that we wish to select() the
    extra_args attribute based e.g. on the device under test and various build settings.
    """,
    implementation = _binary_wrapper_script_with_cmdline_flags_impl,
    attrs = {
        "binary": attr.label(
            doc = "Binary to wrap.",
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
)
