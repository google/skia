"""This module defines the android_gm_test macro."""

load("//tools/testrunners/common/android:android_test.bzl", "android_test")

# This list should be kept in sync with the union of all configs supported by all surface factories
# in //tools/testrunners/common/surface_factory.
_KNOWN_CONFIGS = [
    "8888",
    "565",
    "gles",
]

def android_gm_test(config, via = None, extra_args = [], flags = {}, **kwargs):
    """Defines an Android GM test.

    This macro is just a wrapper around the android_test macro with the necessary defaults for
    Android GM tests. See the android_test macro documentation for details.

    Args:
        config: The config under which the GMs should run.
        via: The via under which the GMs should run. If set, the "flags" argument will be updated
            to set the //gm/vias:via flag accordingly. If unset, no via will be used.
        extra_args: See the android_test macro documentation.
        flags: See the android_test macro documentation.
        **kwargs: Any arguments to pass to the underlying android_test macro instance.
    """
    if config not in _KNOWN_CONFIGS:
        fail("Unknown config: " + config)

    # Set the //gm/vias:via flag to match the "via" argument. This ensures that the build includes
    # the sources for the requested via. If the "via" argument has an unknown value, Bazel will
    # produce an error when the underlying cc_binary_with_flags target attempts to set the
    # //gm/vias:via flag.
    if via:
        flags.update([("via", [via])])

    android_test(
        test_runner_if_required_condition_is_satisfied = "//tools/testrunners/gm:testrunner",
        test_runner_if_required_condition_is_not_satisfied = "//tools/testrunners/common:noop_testrunner",
        extra_args = extra_args + [
            "--outputDir",
            # This environment variable is set by the adb_test_runner.go program.
            "$ADB_TEST_OUTPUT_DIR",
            "--surfaceConfig",
            config,
        ] + (["--via", via] if via else []),
        flags = flags,
        save_output_files = True,  # Save any produced PNG and JSON files as undeclared outputs.
        **kwargs
    )
