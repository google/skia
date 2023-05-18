"""This module defines the adb_test rule."""

load("//bazel:remove_indentation.bzl", "remove_indentation")

def _adb_test_impl(ctx):
    # TODO(lovisolo): Add device-specific (via ctx.attr.device) setup steps such as turning cores
    #                 on/off and setting the CPU/GPU frequencies.

    # TODO(lovisolo): Replace this with a Go program.
    template = remove_indentation("""
        #!/bin/bash

        # Runner script for device "{device}".

        # TODO(lovisolo): Should we check that the machine is attached to the expected device type?
        #                 E.g. run "adb devices -l" and check that the output contains
        #                 "model:Pixel_5".

        # Note: this script was only tested on Pixel devices.
        #
        # The /sdcard/revenge_of_the_skiabot directory is writable for non-root users, but files in
        # this directory cannot be executed. For this reason, we extract the archive in a directory
        # under /data, which allows executing files but requires root privileges.
        #
        # TODO(lovisolo): Can we do this without "su"-ing as root? Does this work on non-rooted
        #                 devices?
        # TODO(lovisolo): Test on more devices.

        ARCHIVE_ON_DEVICE=/sdcard/revenge_of_the_skiabot/bazel-adb-test.tar.gz
        DIRECTORY_ON_DEVICE=/data/bazel-adb-test

        # Print commands and expand variables for easier debugging.
        set -x

        # Ensure that we clean up the device on exit, even in the case of failures.
        # TODO(lovisolo): Also clean up before running the test, as the device might be in a dirty
        #                 state if the previous task did not finish correctly (e.g. device reboot).
        trap "adb shell su root rm -rf ${{ARCHIVE_ON_DEVICE}} ${{DIRECTORY_ON_DEVICE}}" EXIT

        # Upload archive.
        adb push $(rootpath {archive}) ${{ARCHIVE_ON_DEVICE}}

        # Extract archive.
        adb shell su root mkdir ${{DIRECTORY_ON_DEVICE}}
        adb shell su root tar xzvf ${{ARCHIVE_ON_DEVICE}} -C ${{DIRECTORY_ON_DEVICE}}

        # Run test inside the directory where the archive was extracted. We set the working
        # directory to the root of the archive, which emulates the directory structure expected by
        # the test when invoked with "bazel test". See
        # https://bazel.build/reference/test-encyclopedia#initial-conditions.
        echo "cd ${{DIRECTORY_ON_DEVICE}} && $(rootpath {test_runner})" | adb shell su root
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
    ), targets = [
        ctx.attr.archive,
        ctx.attr.test_runner,
    ])

    output_file = ctx.actions.declare_file(ctx.attr.name)
    ctx.actions.write(output_file, template, is_executable = True)

    return [DefaultInfo(
        executable = output_file,
        runfiles = ctx.runfiles(files = [ctx.file.archive]),
    )]

adb_test = rule(
    doc = """Runs an Android test on device via `adb`.""",
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
            allow_single_file = [".sh"],
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
    },
    test = True,
)
