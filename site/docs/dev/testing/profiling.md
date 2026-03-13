---
title: "Profiling Skia with pprof"
linkTitle: "Profiling"

---

Skia binaries (like `nanobench` and `dm`) can be instrumented to produce [CPU](https://github.com/gperftools/gperftools/blob/07c5e9226bda1720bdf783a11f5df0f515e3c9d3/docs/cpuprofile.adoc) and [Heap profiles](https://github.com/gperftools/gperftools/blob/07c5e9226bda1720bdf783a11f5df0f515e3c9d3/docs/tcmalloc.adoc)
compatible with the [pprof](https://github.com/google/pprof) visualizer.

<img src=../pprof_webview.png width=846 height=176 alt="A pprof weblist showing lines of code and time spent on each line." />

Prerequisites
-------------

    # On Debian/Ubuntu:
    $ sudo apt-get install libgoogle-perftools-dev

This provides `libprofiler.so` (for CPU) and `libtcmalloc.so` (for Heap).

Googlers already have the `pprof` analysis tool, but external users can do the following to install `google-pprof` (and may want to make an alias to call it `pprof`).

    # On Debian/Ubuntu:
    $ sudo apt-get install google-perftools

Terminology
-----------

When analyzing profiles, you will see two primary metrics:

*   **Flat**: Time (or memory) spent **strictly within** that specific function. High flat time indicates a bottleneck in the function's own logic (e.g., a heavy loop).
*   **Cumulative (cum)**: Total time spent (or memory allocated) in that function *plus all functions it calls*. High cumulative time with low flat time indicates a bottleneck in one of the function's children.

Building with Profiling Support
-------------------------------

To enable the profiling instrumentation, set `skia_use_pprof=true` in your `args.gn`. It may help to use `-Og` to get accurate line-level attribution without sacrificing the performance benefits of optimization.

    # Example args.gn in out/Profile
    is_debug = false
    skia_use_pprof = true
    extra_cflags = ["-Og"]

Then build your target:

    $ ninja -C out/Profile nanobench

This links in the CPU instrumenter (which will stop the program repeatedly and note where the program was running, aggregating the samples into the profile) and heap instrumenter (which keeps track of all allocations and frees).

Creating Profiles in Nanobench
------------------------------

When built with `skia_use_pprof`, `nanobench` provides flags to enable the profiler(s) to produce output.

## CPU Profiling

Use the `--cpuprofile` flag to specify the output filename. It is often useful to increase the duration of the run to get more samples.

    $ ./out/Profile/nanobench --match <bench_name> --cpuprofile <output.prof> --ms 1000

## Heap Profiling

Use the `--memprofile` flag to specify an output prefix. The heap profiler will produce snapshots as the program runs and at the end.

    $ ./out/Profile/nanobench --match <bench_name> --memprofile <output.heap>
    ...
    Dumping heap profile to output.heap.0001.heap
    ...
    Dumping heap profile to output.heap.0002.heap

Analysis
--------

Use the `pprof` tool to visualize the results.

## Web Interface

### Graph (using GraphViz)

The CPU graph shows how much time was spent with each function on the callstack. This can help identify potential bottlenecks.

<img src=../pprof_cpu_web.png width=500 height=600 alt="A graphviz graph showing time spent in different functions." />

    $ pprof -web ./out/Profile/nanobench <output.prof>

The alloc_space heap graph shows how much memory was allocated on the heap by each function throughout the entire run (even if it was freed up). This can identify where excess memory was allocated.

<img src=../pprof_mem_web.png width=500 height=400 alt="A graphviz graph showing allocations from different functions." />

    $ pprof -alloc_space -web ./out/Profile/nanobench output.heap.0005.heap

Without `-alloc_space`, only live bytes will be shown (unfreed memory). You can use any of the heap files, but it's probably most useful to see the latest one.


### Annotated Source

`pprof` can show how much time was spent on individual lines of code, even breaking down the assembly instructions. Due to instruction re-ordering, this isn't perfect (see Tips below). Large heap allocations can also muddy the performance blame.

<img src=../pprof_cpu_weblist.png width=565 height=293 alt="A pprof weblist showing lines of code and time spent on each line. One line is expanded to show the assembly instructions" />

    $ pprof -weblist <function> ./out/Profile/nanobench <output.prof>
    # Pick a function you want to zoom in. You can run the command w/o providing
    # a function (or function regex), but it's quite noisy.

The `-weblist` works similarly for heap profiles. By using `-alloc_space`, you'll see how much total memory was allocated for a given line.

<img src=../pprof_mem_weblist.png width=794 height=1083 alt="A pprof weblist showing total allocations on a few of the lines in a function." />

    $ pprof -alloc_space -weblist <function> ./out/Profile/nanobench output.heap.0005.heap

### Flame Graphs

As an alternative view to the web graph, a flame graph can be shown. Googlers, this will be created and uploaded into an [internal tool](http://pprof/?id=aaaf3cb7d1c0c1f3dc033d5068d06e29) (which is easier to share with coworkers/bugs).

<img src=../pprof_cpu_flame.png width=951 height=308 alt="A flame graph showing which functions the CPU spent the most time with." />

    $ pprof -flame ./out/Profile/nanobench <output.prof>
    $ pprof -alloc_space -flame ./out/Profile/nanobench output.heap.0005.heap

## Command Line

If you don't want to use the web UI, you can perform quick analysis directly in your terminal.

### Top Functions
See where the most "flat" time is spent.

    `$ pprof -top ./out/Profile/nanobench <output.prof>`

See which functions are responsible for the most allocations (total).

    $ pprof -alloc_space -top ./out/Profile/nanobench output.heap.0005.heap

See which functions allocate the most *objects* (rather than bytes).

    $ pprof -alloc_objects -top ./out/Profile/nanobench output.heap.0005.heap

### Annotated Source
Print annotated source code for a specific function.

    $ pprof -list <function_name> ./out/Profile/nanobench <output.prof>
    $ pprof -alloc_space -list <function_name> ./out/Profile/nanobench output.heap.0005.heap
    $ pprof -alloc_objects -list <function_name> ./out/Profile/nanobench output.heap.0005.heap

## Comparing Profiles (Diffing)

Comparing two profiles is the best way to verify an optimization or find a memory leak. See the [official docs](https://github.com/google/pprof/blob/a15ffb7f9dccb95074ad153aef0f1fcbb01e61e3/doc/README.md#comparing-profiles) for more on that.

Tips
----

*   **Instruction Drifting**: If samples appear on the wrong line (e.g. an `if` statement that
    should take zero time), it may be due to the compiler reordering instructions. Use `-Og` to minimize this.

