"""This module defines the android_gm_test macro."""

load("//bazel:android_test.bzl", "android_test")

# This list should be kept in sync with the union of all configs supported by all surface factories
# in //gm/surface_factory.
_KNOWN_CONFIGS = [
    "8888",
    "565",
]

def android_gm_test(config, extra_args = [], **kwargs):
    """Defines an Android GM test.

    This macro is just a wrapper around the android_test macro with the necessary defaults for
    Android GM tests. See the android_test macro documentation for details.

    Args:
        config: The config under which the GM should run.
        extra_args: See the android_test macro documentation.
        **kwargs: Any arguments to pass to the underlying android_test macro instance.
    """
    if config not in _KNOWN_CONFIGS:
        fail("Unknown config: " + config)

    android_test(
        test_runner_if_required_condition_is_satisfied = "//gm:BazelGMRunner.cpp",
        test_runner_if_required_condition_is_not_satisfied = "//gm:BazelNoopRunner.cpp",
        extra_args = extra_args + [
            "--outputDir",
            # This environment variable is set by the adb_test_runner.go program.
            "$ADB_TEST_OUTPUT_DIR",
            "--surfaceConfig",
            config,
        ],
        save_output_files = True,  # Save any produced PNG and JSON files as undeclared outputs.
        **kwargs
    )
