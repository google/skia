# ANGLE + dEQP
drawElements (dEQP) is a very robust and comprehensive set of open-source tests for GLES2, GLES3+ and EGL. They provide a huge net of coverage for almost every GL API feature. ANGLE by default builds dEQP testing targets for testing with GLES 2 with D3D9/11 and OpenGL, and GLES 3 with D3D11.

## How to build dEQP

You should have dEQP as a target if you followed the [DevSetup](DevSetup.md) instructions. Current targets:

  * GLES2 testing with `angle_deqp_gles2_tests` (supported on Linux/GL and Windows/D3D9/D3D11/GL)
  * GLES3 testing with `angle_deqp_gles3_tests` (supported on Windows/D3D11)
  * EGL testing with `angle_deqp_egl_tests` (supported on Windows)

## How to use dEQP

The `--deqp-case` flag allows you to run individual tests, with wildcard support. For example: `--deqp-case=dEQP-GLES2.functional.shaders.linkage.*`.

Full tests lists are archived in `src/tests/deqp_support`. You can also dump a list of test case names: append the command line argument `--deqp-runmode=txt-caselist`, run the test target, then look for the file named `third_party/deqp/src/data/dEQP-<target>-cases.txt`.

If you're running the full test suite, Debug can take a very long time. Running in Debug is more useful to isolate and fix particular failures.

### Choosing a Renderer on Windows

By default Windows ANGLE tests with D3D11. To specify the exact platform for ANGLE + dEQP, use the arguments:

  * `--deqp-egl-display-type=angle-d3d11` for D3D11 (high feature level)
  * `--deqp-egl-display-type=angle-d3d9` for D3D9
  * `--deqp-egl-display-type=angle-d3d11-fl93` for D3D11 Feature level 9_3
  * `--deqp-egl-display-type=angle-gl` for OpenGL

### Check your results

dEQP generates a test log to `src/tests/TestResults.qpa`. To view the test log information, you'll need to use the open-source GUI [Cherry](https://android.googlesource.com/platform/external/cherry).

See the [official Cherry README](https://android.googlesource.com/platform/external/cherry/+/master/README) for instructions on how to build and install Cherry on Linux or Windows.

### GoogleTest, ANGLE and dEQP

ANGLE also supports the same set of targets built with GoogleTest, for running on the bots. We don't currently recommend using these for local debugging, but we do maintain lists of test expectations in `src/tests/deqp_support`. When you fix tests, please remove the suppression(s) from the relevant files!
