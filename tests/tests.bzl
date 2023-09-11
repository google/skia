"""This module contains macros to generate C++ test targets."""

load("//bazel:cc_test_with_flags.bzl", "cc_test_with_flags")

def skia_cpu_tests(
        name,
        tests,
        harness,
        resources = [],
        flags = {},
        extra_deps = [],
        limit_to = [],
        tags = None):
    """Defines tests that should run only with --config=cpu

    This macro will create one cc_test_with_flags rule for each file in tests.
    These tests are configured to use the BazelTestRunner and actually run all tests
    (e.g. those defined with DEF_TEST) in the file if no GPU backend has been specified,
    that is, the bazel test command was invoked with --config=cpu (see buildrc for this
    definition).

    This macro cannot un-define those targets completely if a GPU backend has been specified
    because the macro runs before the config flags are available for use. An analogy is a
    C++ macro cannot do a #if on the value of a C++ variable.

    Instead, if a GPU backend has been specified (e.g. --config=gl), we only compile the
    BazelNoopRunner.cpp and nothing else (e.g. no deps, not even Skia). This makes any CPU tests
    a trivially passing executable, which can easily be cached and effectively skipped.

    Args:
        name: The name of the test_suite that groups these tests together.
        tests: A list of strings, corresponding to C++ files with one or more DEF_TEST (see Test.h).
        harness: A label (string) corresponding to a skia_cc_library which all the supplementary
                 test helpers, utils, etc. (e.g. Test.h and Test.cpp). Ideally, this is as small
                 as feasible, to avoid unnecessary bloat or recompilation if something unrelated
                 changes.
        resources: A label corresponding to a file_group target that has any skia resource files
                   (e.g. images, fonts) needed to run these tests. Resources change infrequently,
                   so it's not super important that this be a precise list.
        flags: A map of strings to lists of strings to specify features that must be compiled in
               for these tests to work. For example, tests targeting our codec logic will want the
               various codecs included, but most tests won't need that.
        extra_deps: A list of labels corresponding to skia_cc_library targets which this test needs
                    to work, typically some utility located under //tools
        limit_to: A list of platform labels (e.g. @platform//os:foo; @platform//cpu:bar) which
                  restrict where this test will be compiled and ran. If the list is empty, it will
                  run anywhere. If it is non-empty, it will only run on platforms which match the
                  entire set of constraints. See https://github.com/bazelbuild/platforms for these.
        tags: Added to all the generated test targets
    """
    test_targets = []
    if not tags:
        tags = []
    for filename in tests:
        new_target = filename[:-4]  # trim .cpp
        test_targets.append(new_target)
        cc_test_with_flags(
            name = new_target,
            size = "small",
            srcs = select({
                # Make this a no-op test if compiling with a GPU backend.
                ":skip_cpu_tests": ["BazelNoopRunner.cpp"],
                "//conditions:default": [
                    "BazelTestRunner.cpp",
                    filename,
                ],
            }),
            deps = select({
                ":skip_cpu_tests": [],
                # Only build and apply deps if we have a no-op test.
                "//conditions:default": [
                    harness,
                    "//:skia_internal",
                ] + extra_deps,
            }),
            data = resources,
            set_flags = flags,
            target_compatible_with = limit_to,
            tags = tags,
        )

    # https://bazel.build/reference/be/general#test_suite
    native.test_suite(
        name = name,
        tests = test_targets,
    )

def skia_ganesh_tests(
        name,
        tests,
        harness,
        resources = [],
        flags = {},
        extra_deps = [],
        limit_to = [],
        tags = None):
    """Defines tests that should run only when a Ganesh GPU backend is compiled in, e.g --config=gl

    This macro will create one cc_test_with_flags rule for each file in tests.
    These tests are configured to use the BazelTestRunner and actually run all tests (e.g. those
    defined with DEF_TEST or DEF_GANESH_TEST) in the file if a Ganesh GPU backend has been
    specified, that is, the bazel test command was invoked with --config=gl or --config=vk
    (see buildrc for this definition).

    This macro cannot un-define those targets completely if only the CPU backend is being used
    because the macro runs before the config flags are available for use. An analogy is a
    C++ macro cannot do a #if on the value of a C++ variable.

    Instead, if a Ganesh GPU backend has not been specified (e.g. --config=cpu), we only compile the
    BazelNoopRunner.cpp and nothing else (e.g. no deps, not even Skia). This makes any Ganesh GPU
    tests a trivially passing executable, which can easily be cached and effectively skipped.

    Test files may have both CPU tests (i.e. DEF_TEST) and GPU tests (e.g. DEF_GANESH_TEST). The
    BazelTestRunner will always run all tests that were compiled in. The CPU tests defined in this
    way will *not* be run with --config=cpu because they live in a file that can only be compiled
    with at least one GPU backend; they will be run with a config like --config=gl, along side the
    GPU tests defined with DEF_GANESH_TEST.

    Args:
        name: The name of the test_suite that groups these tests together.
        tests: A list of strings, corresponding to C++ files with one or more DEF_GANESH_TEST
               (see Test.h).
        harness: A label (string) corresponding to a skia_cc_library which all the supplementary
                 test helpers, utils, etc. (e.g. Test.h and Test.cpp). Ideally, this is as small
                 as feasible, to avoid unnecessary bloat or recompilation if something unrelated
                 changes.
        resources: A label corresponding to a file_group target that has any skia resource files
                   (e.g. images, fonts) needed to run these tests. Resources change infrequently,
                   so it's not super important that this be a precise list.
        flags: A map of strings to lists of strings to specify features that must be compiled in
               for these tests to work. For example, tests targeting our codec logic will want the
               various codecs included, but most tests won't need that.
        extra_deps: A list of labels corresponding to skia_cc_library targets which this test needs
                    to work, typically some utility located under //tools
        limit_to: A list of platform labels (e.g. @platform//os:foo; @platform//cpu:bar) which
                  restrict where this test will be compiled and ran. If the list is empty, it will
                  run anywhere. If it is non-empty, it will only run on platforms which match the
                  entire set of constraints. See https://github.com/bazelbuild/platforms for these.
        tags: Added to all the generated test targets
    """
    test_targets = []
    if not tags:
        tags = []
    for filename in tests:
        new_target = filename[:-4]  # trim .cpp
        test_targets.append(new_target)
        cc_test_with_flags(
            name = new_target,
            size = "small",
            srcs = select({
                "//src/gpu:has_ganesh_backend": [
                    "BazelTestRunner.cpp",
                    filename,
                ],
                # Make this a no-op test if not compiling with a Ganesh GPU backend.
                "//conditions:default": ["BazelNoopRunner.cpp"],
            }),
            deps = select({
                # Only build and apply deps if we have a no-op test.
                "//src/gpu:has_ganesh_backend": [
                    harness,
                    "//:skia_internal",
                ] + extra_deps,
                "//conditions:default": [],
            }),
            data = resources,
            set_flags = flags,
            target_compatible_with = limit_to,
            tags = tags + [
                # We currently have no RBE machines with GPUs, so we cannot run these remotely.
                "no-remote",
            ],
        )

    # https://bazel.build/reference/be/general#test_suite
    native.test_suite(
        name = name,
        tests = test_targets,
    )
