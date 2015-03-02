Testing
=======

Skia relies heavily on our suite of unit and Golden Master \(GM\) tests, which
are served by our Diamond Master \(DM\) test tool, for correctness testing.
Tests are executed by our trybots, for every commit, across most of our
supported platforms and configurations. 
Skia [Gold](https://gold.skia.org) is a web interface for triaging these results.

We also have a robust set of performance tests, served by the nanobench tool and
accessible via the Skia [Perf](https://perf.skia.org) web interface.

Cluster Telemetry is a powerful framework that helps us capture and benchmark
SKP files, a binary format for draw commands, across up to one million websites.

See the individual subpages for more details on our various test tools.
