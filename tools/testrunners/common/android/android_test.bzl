"""This module defines the android_test macro."""

load("//bazel:skia_rules.bzl", "skia_cc_binary")
load("//tools/testrunners/common:binary_wrapper_script_with_cmdline_flags.bzl", "binary_wrapper_script_with_cmdline_flags")
load("//tools/testrunners/common/android:adb_test.bzl", "adb_test")

def android_test(
        name,
        srcs,
        deps = [],
        extra_args = [],
        benchmark = False,
        requires_resources_dir = False,
        save_output_files = False):
    """Defines an Android test.

    Note: This macro is not intended to be used directly in BUILD files. Instead, please use macros
    android_unit_test, android_gm_test, android_benchmark_test, etc.

    This macro compiles one or more C++ tests into a single Android binary and produces a script
    that runs the test on an attached Android device via `adb`. This macro is compatible with unit,
    GM and benchmark tests.

    The test target produced by this macro can be executed on a machine attached to an Android
    device. This can be either via USB, or by port-forwarding a remote ADB server (TCP port 5037)
    running on a machine attached to the target device, such as a Skolo Raspberry Pi.

    High-level overview of how this rule works:

    - It produces a <name>.tar.gz archive containing the Android binary, a minimal launcher script
      that invokes the binary on the device under test with any necessary command-line arguments,
      and any static resources needed by the C++ tests, such as fonts and images under //resources.
    - It produces a <name> test runner script that extracts the tarball into the device via `adb`,
      sets up the device, runs the test, cleans up and pipes through the test's exit code.
    - Optionally, the <name> test runner script can be configured to download from the device any
      files produced by the C++ tests (such as PNG and JSON files produced by GM tests). These
      files will be available as undeclared test outputs (see documentation for the
      TEST_UNDECLARED_OUTPUTS_DIR environment variable at
      https://bazel.build/reference/test-encyclopedia#initial-conditions).

    For CI jobs, rather than invoking "bazel test" on a Raspberry Pi attached to the Android device
    under test, we compile and run the test in two separate tasks:

    - A build task running on a GCE machine compiles the test on RBE with Bazel and stores the
      <name>.tar.gz and <name> output files to CAS.
    - A test task running on a Skolo Raspberry Pi downloads <name>.tar.gz and <name> from CAS and
      executes <name> *outside of Bazel*.

    The reason why we don't want to run Bazel on a Raspberry Pi is due to its constrained
    resources.

    Note: Although not currently supported, we could use a similar approach for Apple devices in
    in the future.

    Args:
        name: The name of the test.
        srcs: A list of C++ source files.
        deps: Any dependencies needed by the srcs.
        extra_args: Additional command-line arguments to pass to the test, for example, any
            device-specific --skip flags to skip incompatible or buggy test cases.
        benchmark: Set up the device for benchmark tests. This might affect e.g. CPU and GPU
            settings specific to the Android device under test.
        requires_resources_dir: If set, the contents of the //resources directory will be included
            in the tarball that is pushed to the device via `adb push`, and the test binary will be
            invoked with flag --resourcePath set to the path to said directory.
        save_output_files: If true, save any files produced by this test (e.g. PNG and JSON files
            in the case of GM tests) as undeclared outputs (see documentation for the
            TEST_UNDECLARED_OUTPUTS_DIR environment variable at
            https://bazel.build/reference/test-encyclopedia#initial-conditions).
    """

    test_binary = "%s_binary" % name

    skia_cc_binary(
        name = test_binary,
        srcs = srcs,
        deps = deps,
        testonly = True,  # Needed to gain access to test-only files.
    )

    test_runner = "%s_runner" % name

    binary_wrapper_script_with_cmdline_flags(
        name = test_runner,
        binary = test_binary,
        extra_args = extra_args,
        requires_resources_dir = requires_resources_dir,
        testonly = True,  # Needed to gain access to test-only files.
    )

    archive = "%s_archive" % name
    archive_srcs = [test_runner, test_binary] + (
        ["//resources"] if requires_resources_dir else []
    )

    # Create an archive containing the test and its resources, with a structure that emulates
    # the environment expected by the test when executed via "bazel test". This archive can be
    # pushed to an Android device via "adb push", and once extracted, the test binary can be
    # executed on the device via "adb shell" as long as the working directory is set to the
    # directory where the archive is extracted.
    #
    # See https://bazel.build/reference/test-encyclopedia#initial-conditions.
    native.genrule(
        name = archive,
        srcs = archive_srcs,
        outs = ["%s.tar.gz" % name],
        cmd = """
            $(location //tools/testrunners/common/make_tarball) \
                --execpaths "{execpaths}" \
                --rootpaths "{rootpaths}" \
                --output-file $@
        """.format(
            execpaths = " ".join(["$(execpaths %s)" % src for src in archive_srcs]),
            rootpaths = " ".join(["$(rootpaths %s)" % src for src in archive_srcs]),
        ),
        testonly = True,  # Needed to gain access to test-only files.
        # Tools are always built for the exec platform
        # (https://bazel.build/reference/be/general#genrule.tools), e.g. Linux on x86_64 when
        # running on a gLinux workstation or on a Linux GCE machine.
        tools = ["//tools/testrunners/common/make_tarball"],
    )

    adb_test(
        name = name,
        archive = archive,
        test_runner = test_runner,
        benchmark = benchmark,
        save_output_files = save_output_files,
        tags = ["no-remote"],  # Incompatible with RBE because it requires an Android device.
        target_compatible_with = ["@platforms//os:android"],
    )
