"""This module defines the skia_android_unit_test macro."""

load("//bazel:cc_binary_with_flags.bzl", "cc_binary_with_flags")
load("//bazel:remove_indentation.bzl", "remove_indentation")
load(":adb_test.bzl", "adb_test")

def skia_test(
        name,
        srcs,
        deps,
        requires_resources_dir = False,
        extra_args = [],
        flags = {},
        limit_to = [],
        tags = [],
        size = None):
    """Defines a generic Skia C++ unit test.

    This macro produces a <name>_binary C++ binary and a <name>.sh wrapper script that runs the
    binary with the desired command-line arguments (see the extra_args and requires_resources_dir
    arguments). The <name>.sh wrapper script is exposed as a Bazel test target via the sh_target
    rule.

    The reason why we place command-line arguments in a wrapper script is that it makes it easier
    to run a Bazel-built skia_test outside of Bazel. This is useful e.g. for CI jobs where we want
    to perform test compilation and execution as different steps on different hardware (e.g.
    compile on a GCE machine, run tests on a Skolo device). In this scenario, the test could be
    executed outside of Bazel by simply running the <name>.sh script without any arguments. See the
    skia_android_unit_test macro for an example.

    Note: The srcs attribute must explicitly include a test runner (e.g.
    //tests/BazelTestRunner.cpp).

    Args:
        name: The name of the test.
        srcs: C++ source files.
        deps: C++ library dependencies.
        requires_resources_dir: Indicates whether this test requires any files under //resources,
            such as images, fonts, etc. If so, the compiled C++ binary will be invoked with flag
            --resourcePath set to the path to the //resources directory under the runfiles tree.
            Note that this implies the test runner must recognize the --resourcePath flag for this
            to work.
        extra_args: Any additional command-line arguments to pass to the compiled C++ binary.
        flags: A map of strings to lists of strings to specify features that must be compiled in
            for these tests to work. For example, tests targeting our codec logic will want the
            various codecs included, but most tests won't need that.
        limit_to: A list of platform labels (e.g. @platform//os:foo; @platform//cpu:bar) which
            restrict where this test will be compiled and ran. If the list is empty, it will run
            anywhere. If it is non-empty, it will only run on platforms which match the entire set
            of constraints. See https://github.com/bazelbuild/platforms for these.
        tags: A list of tags for the generated test target.
        size: The size of the test.
    """
    test_binary = "%s_binary" % name

    # We compile the test as a cc_binary, rather than as as a cc_test, because we will not
    # "bazel test" this binary directly. Instead, we will "bazel test" a wrapper script that
    # invokes this binary with the required command-line parameters.
    cc_binary_with_flags(
        name = test_binary,
        srcs = srcs,
        deps = deps,
        data = ["//resources"] if requires_resources_dir else [],
        set_flags = flags,
        target_compatible_with = limit_to,
        testonly = True,  # Needed to gain access to test-only files.
    )

    test_runner = "%s.sh" % name

    test_args = ([
        "--resourcePath",
        "$$(realpath $$(dirname $(rootpath //resources:README)))",
    ] if requires_resources_dir else []) + extra_args

    # This test runner might run on Android devices, which might not have a /bin/bash binary.
    test_runner_template = remove_indentation("""
        #!/bin/sh
        $(rootpath {test_binary}) {test_args}
    """)

    # TODO(lovisolo): This should be an actual rule. This will allow us to select() the arguments
    #                 based on the device (e.g. for device-specific --skip flags to skip tests).
    native.genrule(
        name = "%s_runner" % name,
        srcs = [test_binary] + (
            # The script template computes the path to //resources under the runfiles tree via
            # $$(dirname $(rootpath //resources:README)), so we need to list //resources:README
            # here explicitly. This file was chosen arbitrarily; there is nothing special about it.
            ["//resources", "//resources:README"] if requires_resources_dir else []
        ),
        outs = [test_runner],
        cmd = "echo '%s' > $@" % test_runner_template.format(
            test_binary = test_binary,
            test_args = "\\\n    ".join(test_args),
        ),
        testonly = True,
    )

    native.sh_test(
        name = name,
        size = size,
        srcs = [test_runner],
        data = [test_binary] + (["//resources"] if requires_resources_dir else []),
        tags = tags,
    )

def skia_android_unit_test(
        name,
        srcs,
        deps = [],
        flags = {},
        extra_args = [],
        requires_condition = "//:always_true",
        requires_resources_dir = False):
    """Defines a Skia Android unit test.

    This macro compiles one or more C++ unit tests into a single Android binary and produces a
    script that runs the test on an attached Android device via `adb`.

    This macro requires a device-specific Android platform such as //bazel/devices:pixel_5. This is
    used to decide what device-specific set-up steps to apply, such as setting CPU/GPU frequencies.

    The test target produced by this macro can be executed on a machine attached to an Android
    device. This can be either via USB, or by port-forwarding a remote ADB server (TCP port 5037)
    running on a machine attached to the target device, such as a Skolo Raspberry Pi.

    High-level overview of how this rule works:

    - It produces a <name>.tar.gz archive containing the Android binary, a minimal launcher script
      that invokes the binary with the necessary command-line arguments, and any static resources
      needed by the test, such as fonts and images under //resources.
    - It produces a <name>.sh test runner script that extracts the tarball into the device via
      `adb`, sets up the device, runs the test, cleans up and pipes through the test's exit code.

    For CI jobs, rather than invoking "bazel test" on a Raspberry Pi attached to the Android device
    under test, we compile and run the test in two separate tasks:

    - A build task running on a GCE machine compiles the test on RBE with Bazel and stores the
      <name>.tar.gz and <name>.sh output files to CAS.
    - A test task running on a Skolo Raspberry Pi downloads <name>.tar.gz and <name>.sh from CAS
      and executes <name>.sh *outside of Bazel*.

    The reason why we don't want to run Bazel on a Raspberry Pi is due to its constrained
    resources.

    Note: Although not currently supported, we could use a similar approach for Apple devices in
    in the future.

    Args:
        name: The name of the test.
        srcs: A list of C++ source files. This list should not include a main function (see the
            requires_condition argument).
        deps: Any dependencies needed by the srcs. This list should not include a main function
            (see the requires_condition argument).
        flags: A map of strings to lists of strings to specify features that must be compiled in
            for these tests to work. For example, tests targeting our codec logic will want the
            various codecs included, but most tests won't need that.
        extra_args: Additional command-line arguments to pass to the test, for example, any
            device-specific --skip flags to skip incompatible or buggy test cases.
            TODO(lovisolo): Do we need to support skipping tests? IIUC today we only skip DMs, but
                            we don't skip any unit tests.
        requires_condition: A necessary condition for the test to work. For example, GPU tests
            should set this argument to "//src/gpu:has_gpu_backend". If the condition is satisfied,
            //tests:BazelTestRunner.cpp will be appended to the srcs attribute. If the condition is
            not satisfied, //tests:BazelNoopRunner.cpp will be included instead, and no deps will
            be included. This prevents spurious build failures when using wildcard expressions
            (e.g. "bazel build //tests/...") with a configuration that is incompatible with this
            test.
        requires_resources_dir: If set, the contents of the //resources directory will be included
            in the test runfiles, and the test binary will be invoked with flag --resourcePath set
            to the path to said directory.
    """

    skia_test(
        name = "%s_cpp_test" % name,
        srcs = select({
            requires_condition: srcs + ["//tests:BazelTestRunner.cpp"],
            "//conditions:default": ["//tests:BazelNoopRunner.cpp"],
        }),
        deps = select({
            requires_condition: deps,
            "//conditions:default": [],
        }),
        flags = flags,
        extra_args = extra_args,
        requires_resources_dir = requires_resources_dir,
        tags = [
            # Exclude it from wildcards, e.g. "bazel test //...". We never want to run this binary
            # directly.
            "manual",
            "no-remote",  # RBE workers cannot run Android tests.
        ],
        size = "large",  # Can take several minutes.
    )

    test_binary = "%s_cpp_test_binary" % name
    test_runner = "%s_cpp_test.sh" % name

    archive = "%s_archive" % name
    archive_srcs = [test_runner, test_binary] + (
        ["//resources"] if requires_resources_dir else []
    )

    # Create an archive containing the test and its resources, with a structure that emulates
    # the environment expected by the test when executed via "bazel test". This archive can be
    # pushed to an Android device via "adb push", and the contained test can be executed on the
    # device via "adb shell" as long as the working directory is set to the directory where the
    # archive is extracted.
    #
    # See https://bazel.build/reference/test-encyclopedia#initial-conditions.
    native.genrule(
        name = archive,
        srcs = archive_srcs,
        outs = ["%s.tar.gz" % name],
        cmd = """
            $(location //tests/make_adb_test_tarball) \
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
        tools = ["//tests/make_adb_test_tarball"],
    )

    adb_test(
        name = name,
        archive = archive,
        test_runner = test_runner,
        device = select(
            {
                "//bazel/devices:pixel_5": "pixel_5",
                "//bazel/devices:pixel_7": "pixel_7",
                "//conditions:default": "unknown",
            },
        ),
        tags = ["no-remote"],  # Incompatible with RBE because it requires an Android device.
        target_compatible_with = select({
            "//bazel/devices:has_android_device": [],
            "//conditions:default": ["@platforms//:incompatible"],
        }),
    )
