"""This module defines the android_benchmark_test macro."""

load("//tools/testrunners/common/android:android_test.bzl", "android_test")
load("//tools/testrunners/common/surface_manager:surface_configs.bzl", "SURFACE_CONFIGS")

def android_benchmark_test(surface_config, extra_args = [], flags = {}, **kwargs):
    """Defines an Android benchmark test.

    This macro is just a wrapper around the android_test macro with the necessary defaults for
    Android benchmark tests. See the android_test macro documentation for details.

    Args:
        surface_config: The surface config under which the benchmarks should run.
        extra_args: See the android_test macro documentation.
        flags: See the android_test macro documentation.
        **kwargs: Any arguments to pass to the underlying android_test macro instance.
    """
    if surface_config not in SURFACE_CONFIGS:
        fail("Unknown surface_config: " + surface_config)

    # The test runner needs the PNG encoder to save PNG images to the output directory.
    if "include_encoder" not in flags:
        # We use the union ("|") operator instead of foo[bar] = baz because the latter yields
        # "Error: trying to mutate a frozen dict value".
        flags = flags | {"include_encoder": []}
    if "png_encode_codec" not in flags["include_encoder"]:
        flags["include_encoder"].append("png_encode_codec")

    android_test(
        test_runner_if_required_condition_is_satisfied = "//tools/testrunners/benchmark:testrunner",
        test_runner_if_required_condition_is_not_satisfied = "//tools/testrunners/common:noop_testrunner",
        extra_args = extra_args + [
            "--outputDir",
            # This environment variable is set by the adb_test_runner.go program.
            "$ADB_TEST_OUTPUT_DIR",
            "--surfaceConfig",
            surface_config,
        ],
        benchmark = True,
        flags = flags,
        save_output_files = True,  # Save any produced PNG and JSON files as undeclared outputs.
        **kwargs
    )
