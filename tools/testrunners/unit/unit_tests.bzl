"""This module contains macros to generate C++ unit test targets."""

load("@skia_user_config//:copts.bzl", "DEFAULT_COPTS")
load("@skia_user_config//:linkopts.bzl", "DEFAULT_LINKOPTS")

def unit_tests(
        name,
        tests,
        deps,
        resources = [],
        extra_srcs = [],
        tags = None):
    """This macro will create one cc_test rule for each file in tests.

    These tests are configured to use the BazelUnitTestRunner and run all tests
    (e.g. those defined with DEF_TEST) in the file.

    Args:
        name: The name of the test_suite that groups these tests together.
        tests: A list of strings, corresponding to C++ files with one or more DEF_TEST (see Test.h).
        deps: A list of labels corresponding to cc_library targets which this test needs to work.
            This typically includes some Skia modules and maybe some test utils.
        resources: A label corresponding to a file_group target that has any skia resource files
            (e.g. images, fonts) needed to run these tests. Resources change infrequently, so
            it's not super important that this be a precise list.
        extra_srcs: Any extra files (e.g. headers) that are needed to build these tests. This is
            a more convenient way to include a few extra files without needing to create a
            distinct test cc_library.
        tags: Added to all the generated test targets
    """
    test_targets = []
    if not tags:
        tags = []
    for filename in tests:
        new_target = name + "_" + filename[:-4]  # trim .cpp
        test_targets.append(new_target)
        native.cc_test(
            name = new_target,
            copts = DEFAULT_COPTS,
            linkopts = DEFAULT_LINKOPTS,
            size = "small",
            srcs = [filename] + extra_srcs,
            deps = deps + ["//tools/testrunners/unit:testrunner"],
            data = resources,
            tags = tags,
        )

    # https://bazel.build/reference/be/general#test_suite
    native.test_suite(
        name = name,
        tests = test_targets,
    )
