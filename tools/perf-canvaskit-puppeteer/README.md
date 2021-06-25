Measuring the performance of CanvasKit using Puppeteer and Chrome.

## Initial setup

Run `npm ci` to install the dependencies need to run the tests. In //modules/canvaskit, run
`make release` to build the canvaskit that will be used. With modifications to the Makefile,
other builds (e.g. `make profile`) can be used as well.

If needed, one can download the lottie-samples and/or skp assets from CIPD using the sk tool:
```
sk asset download lottie-samples ~/Downloads/lottie-samples
sk asset download skps ~/Downloads/skps
```

The actual location that these assets can be downloaded to is not important - the Makefile assumes
them to be in Downloads, but that can be modified by the local user.

## Basic Performance Tests
We have a harness for running benchmarks. Benchmark code snippets can be added to `canvas_perf.js`.
The harness itself is the `canvas_perf.html` and `benchmark.js`. It will run the "test" portion of
the code on multiple frames and gather data.

To run the benchmarks, run `make perf_js`. By default, this will use the most recent release build
of canvaskit done locally. If you want to only run one or a few, modify the
`canvas_perf.js` file by changing the relevent `tests.push` to `onlytests.push` and then run
`make perf_js`.

On the CI, the results from these tests are uploaded to Perf. For example:
<https://perf.skia.org/e/?queries=test%3Dcanvas_drawOval>
We include metrics such as the 90th, 95th, and 99th percentile frame, average frame time, median
frame time, and standard deviation. There are three types of measurements: without_flush_ms is
the measurement of the test() function; with_flush_ms is the measurement of test() and the
subsequent flush() call; total_frame_ms is the frame-to-frame time. Frame-to-frame is important to
measure because it accounts for any work the GPU needs to do, even after CanvasKit flushes.

## Skottie Frames Performance
There is a harness that gathers data about rendering 600 frames of a skottie animation, cycling
through it in a similar fashion to how it would be displayed to a user (e.g. as it is on
skottie.skia.org).

To test it locally with a specific skottie animation, feel free to modify the Makefile to adjust the
`input_lottie` argument and then run `make frames`. The harness itself is `skottie-frames.html` and
`benchmark.js`.

On the CI, the results from these tests are uploaded to Perf. For example:
<https://perf.skia.org/e/?queries=test%3Dlego_loader>
We include metrics such as the first 5 frame times, average frame times, 90th, 95th and 99th
percentile frame time.

## SKP Performance
There is a harness that repeatedly will draw an SKP and measure various metrics. This is handled
by `skottie-frames.html` and `benchmark.js`. As before, feel free to modify the Makefile (the
`input_skp` argument) and run `make skp`.

On the CI, the results from these tests are uploaded to Perf. For example:
<https://perf.skia.org/e/?queries=binary%3DCanvasKit%26test%3Ddesk_chalkboard.skp>