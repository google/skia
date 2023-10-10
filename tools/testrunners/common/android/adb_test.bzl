"""This module defines the adb_test rule."""

load("@local_config_platform//:constraints.bzl", "HOST_CONSTRAINTS")
load("//bazel:remove_indentation.bzl", "remove_indentation")

def _adb_test_runner_transition_impl(settings, attr):  # buildifier: disable=unused-variable
    platform = settings["//tools/testrunners/common/android/adb_test_runner:adb_platform"]

    # If no platform was specified via --adb_platform, use the host platform. This allows us to
    # "bazel test" an adb_test target on a developer workstation without passing said flag to
    # Bazel.
    if platform == "":
        # The HOST_CONSTRAINTS list should always be of the form [cpu, os], e.g.:
        #
        #     HOST_CONSTRAINTS = ["@platforms//cpu:x86_64", "@platforms//os:linux"]
        #
        # The CPU and OS constraints will be omitted from HOST_CONSTRAINTS in the rare case that
        # they cannot be determined.
        #
        # Reference:
        # https://github.com/bazelbuild/bazel/blob/30ca122db02d953068ebb2b036b015e6b375c9ce/src/main/java/com/google/devtools/build/lib/bazel/repository/LocalConfigPlatformFunction.java#L186
        if len(HOST_CONSTRAINTS) != 2 or \
           not HOST_CONSTRAINTS[0].startswith("@platforms//cpu:") or \
           not HOST_CONSTRAINTS[1].startswith("@platforms//os:"):
            fail(
                "Expected HOST_CONSTRAINTS to be of the form " +
                """["@platforms//cpu:<cpu>", "@platforms//os:<os>"], got""",
                HOST_CONSTRAINTS,
            )

        # Map the Bazel constants to GOARCH constants. More can be added as needed. See
        # https://github.com/bazelbuild/rules_go/blob/5933b6ed063488472fc14ceca232b3115e8bc39f/go/private/platforms.bzl#LL30C9-L30C9.
        cpu = HOST_CONSTRAINTS[0].removeprefix("@platforms//cpu:")
        os = HOST_CONSTRAINTS[1].removeprefix("@platforms//os:")
        cpu = {
            "x86_64": "amd64",
            "aarch64": "arm64",
        }.get(cpu, cpu)  # Defaults to the original CPU if not in the dictionary.
        os = {
            "osx": "darwin",
        }.get(os, os)  # Default to the original OS if not in the dictionary.

        platform = os + "_" + cpu
    else:
        # Map the --adb_platform CPU part to GOARCH style, which we differ from for readability.
        platform = platform.replace("x86", "amd64")

    return {"//command_line_option:platforms": "@io_bazel_rules_go//go/toolchain:" + platform}

# This transition allows us to cross-compile the Go test runner (i.e. the program that issues adb
# commands) for a different platform, for example when "bazel build"-ing on an x86 GCE machine and
# running the compiled artifact on a Raspberry Pi in a subsequent CI task.
adb_test_runner_transition = transition(
    implementation = _adb_test_runner_transition_impl,
    inputs = ["//tools/testrunners/common/android/adb_test_runner:adb_platform"],
    outputs = ["//command_line_option:platforms"],
)

def _adb_test_impl(ctx):
    test_undeclared_outputs_dir_env_var_check = ""
    output_dir_flag = ""
    if ctx.attr.save_output_files:
        test_undeclared_outputs_dir_env_var_check = remove_indentation("""
            if [[ -z "${TEST_UNDECLARED_OUTPUTS_DIR}" ]]; then
                echo "FAILED: Environment variable TEST_UNDECLARED_OUTPUTS_DIR is unset. If you"
                echo "        are running this test outside of Bazel, set said variable to the"
                echo "        directory where you wish to store any files produced by this test."

                exit 1
            fi
        """)
        output_dir_flag = "--output-dir $TEST_UNDECLARED_OUTPUTS_DIR"

    template = remove_indentation("""
        #!/bin/bash

        {test_undeclared_outputs_dir_env_var_check}

        # Print commands and expand variables for easier debugging.
        set -x

        # List the test runner binary for debugging purposes.
        ls -l $(rootpath {adb_test_runner})

        $(rootpath {adb_test_runner}) \
            --device {device} \
            --archive $(rootpath {archive}) \
            --test-runner $(rootpath {test_runner}) \
            {output_dir_flag}
    """)

    if ctx.attr.device == "unknown":
        template = remove_indentation("""
            #!/bin/bash

            echo "FAILED: No Android device was specified. Try re-running with a Bazel flag that"
            echo "        specifies an Android device under test, such as --config=pixel_5."

            exit 1
        """)

    # Expand variables.
    template = ctx.expand_location(template.format(
        device = ctx.attr.device,
        archive = ctx.attr.archive.label,
        test_runner = ctx.attr.test_runner.label,
        adb_test_runner = ctx.attr._adb_test_runner[0].label,
        test_undeclared_outputs_dir_env_var_check = test_undeclared_outputs_dir_env_var_check,
        output_dir_flag = output_dir_flag,
    ), targets = [
        ctx.attr.archive,
        ctx.attr.test_runner,
        ctx.attr._adb_test_runner[0],
    ])

    output_file = ctx.actions.declare_file(ctx.attr.name)
    ctx.actions.write(output_file, template, is_executable = True)

    runfiles = ctx.runfiles(files = [ctx.file.archive])
    runfiles = runfiles.merge(ctx.attr._adb_test_runner[0][DefaultInfo].default_runfiles)

    return [DefaultInfo(
        executable = output_file,
        runfiles = runfiles,
    )]

adb_test = rule(
    doc = """Runs an Android test on device via `adb`.

    Note: This rule is not intended to be used directly in BUILD files. Instead, please use macros
    android_unit_test, android_gm_test, etc.

    This test rule produces a wrapper shell script that invokes a Go proram that issues adb
    commands to interact with the device under test.

    When building a test that should run on a different host (e.g. a Skolo Raspberry Pi), invoke
    Bazel with flag --adb_platform to set the adb_test_runner target platform accordingly, for
    example --adb_platform=linux_arm64.
    """,
    implementation = _adb_test_impl,
    attrs = {
        "device": attr.string(
            doc = "Device under test.",
            mandatory = True,
            values = [
                "pixel_5",
                "pixel_7",
                "unknown",
            ],
        ),
        "test_runner": attr.label(
            doc = (
                "Test runner script that calls the compiled C++ binary with any necessary " +
                "command-line arguments. This script will be executed on the Android device."
            ),
            allow_single_file = True,
            mandatory = True,
        ),
        "archive": attr.label(
            doc = (
                "Tarball containing the test runner script, the compiled C++ binary and any" +
                "necessary static resources such as fonts, images, etc."
            ),
            allow_single_file = [".tar.gz"],
            mandatory = True,
        ),
        "save_output_files": attr.bool(
            doc = (
                "If true, save any files produced by this test (e.g. PNG and JSON files in the " +
                "case of GM tests) as undeclared outputs (see documentation for the " +
                "TEST_UNDECLARED_OUTPUTS_DIR environment variable at " +
                "https://bazel.build/reference/test-encyclopedia#initial-conditions)."
            ),
        ),
        "_adb_test_runner": attr.label(
            default = Label("//tools/testrunners/common/android/adb_test_runner"),
            allow_single_file = True,
            executable = True,
            cfg = adb_test_runner_transition,
        ),
        "_allowlist_function_transition": attr.label(
            default = "@bazel_tools//tools/allowlists/function_transition_allowlist",
        ),
    },
    test = True,
)
