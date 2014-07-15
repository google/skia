/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <ctype.h>

#include "Benchmark.h"
#include "CrashHandler.h"
#include "ResultsWriter.h"
#include "Stats.h"
#include "Timer.h"

#include "SkCanvas.h"
#include "SkCommandLineFlags.h"
#include "SkForceLinking.h"
#include "SkGraphics.h"
#include "SkString.h"
#include "SkSurface.h"

#if SK_SUPPORT_GPU
    #include "GrContextFactory.h"
    GrContextFactory gGrFactory;
#endif

__SK_FORCE_IMAGE_DECODER_LINKING;

#if SK_DEBUG
    DEFINE_bool(runOnce, true, "Run each benchmark just once?");
#else
    DEFINE_bool(runOnce, false, "Run each benchmark just once?");
#endif

DEFINE_int32(samples, 10, "Number of samples to measure for each bench.");
DEFINE_int32(overheadLoops, 100000, "Loops to estimate timer overhead.");
DEFINE_double(overheadGoal, 0.0001,
              "Loop until timer overhead is at most this fraction of our measurments.");
DEFINE_string(match, "", "The usual filters on file names of benchmarks to measure.");
DEFINE_bool2(quiet, q, false, "Print only bench name and minimum sample.");
DEFINE_bool2(verbose, v, false, "Print all samples.");
DEFINE_string(config, "nonrendering 8888 gpu", "Configs to measure. Options: "
              "565 8888 gpu nonrendering debug nullgpu msaa4 msaa16 nvprmsaa4 nvprmsaa16 angle");
DEFINE_double(gpuMs, 5, "Target bench time in millseconds for GPU.");
DEFINE_int32(gpuFrameLag, 5, "Overestimate of maximum number of frames GPU allows to lag.");

DEFINE_bool(cpu, true, "Master switch for CPU-bound work.");
DEFINE_bool(gpu, true, "Master switch for GPU-bound work.");

DEFINE_string(outResultsFile, "", "If given, write results here as JSON.");
DEFINE_bool(resetGpuContext, true, "Reset the GrContext before running each bench.");


static SkString humanize(double ms) {
    if (ms > 1e+3) return SkStringPrintf("%.3gs",  ms/1e3);
    if (ms < 1e-3) return SkStringPrintf("%.3gns", ms*1e6);
    if (ms < 1)    return SkStringPrintf("%.3gµs", ms*1e3);
    return SkStringPrintf("%.3gms", ms);
}

static double time(int loops, Benchmark* bench, SkCanvas* canvas, SkGLContextHelper* gl) {
    WallTimer timer;
    timer.start();
    if (bench) {
        bench->draw(loops, canvas);
    }
    if (canvas) {
        canvas->flush();
    }
#if SK_SUPPORT_GPU
    if (gl) {
        SK_GL(*gl, Flush());
        gl->swapBuffers();
    }
#endif
    timer.end();
    return timer.fWall;
}

static double estimate_timer_overhead() {
    double overhead = 0;
    for (int i = 0; i < FLAGS_overheadLoops; i++) {
        overhead += time(1, NULL, NULL, NULL);
    }
    return overhead / FLAGS_overheadLoops;
}

static int cpu_bench(const double overhead, Benchmark* bench, SkCanvas* canvas, double* samples) {
    // First figure out approximately how many loops of bench it takes to make overhead negligible.
    double bench_plus_overhead;
    do {
        bench_plus_overhead = time(1, bench, canvas, NULL);
    } while (bench_plus_overhead < overhead);  // Shouldn't normally happen.

    // Later we'll just start and stop the timer once but loop N times.
    // We'll pick N to make timer overhead negligible:
    //
    //          overhead
    //  -------------------------  < FLAGS_overheadGoal
    //  overhead + N * Bench Time
    //
    // where bench_plus_overhead ≈ overhead + Bench Time.
    //
    // Doing some math, we get:
    //
    //  (overhead / FLAGS_overheadGoal) - overhead
    //  ------------------------------------------  < N
    //       bench_plus_overhead - overhead)
    //
    // Luckily, this also works well in practice. :)
    const double numer = overhead / FLAGS_overheadGoal - overhead;
    const double denom = bench_plus_overhead - overhead;
    const int loops = FLAGS_runOnce ? 1 : (int)ceil(numer / denom);

    for (int i = 0; i < FLAGS_samples; i++) {
        samples[i] = time(loops, bench, canvas, NULL) / loops;
    }
    return loops;
}

#if SK_SUPPORT_GPU
static int gpu_bench(SkGLContextHelper* gl,
                     Benchmark* bench,
                     SkCanvas* canvas,
                     double* samples) {
    // Make sure we're done with whatever came before.
    SK_GL(*gl, Finish());

    // First, figure out how many loops it'll take to get a frame up to FLAGS_gpuMs.
    int loops = 1;
    if (!FLAGS_runOnce) {
        double elapsed = 0;
        do {
            loops *= 2;
            // If the GPU lets frames lag at all, we need to make sure we're timing
            // _this_ round, not still timing last round.  We force this by looping
            // more times than any reasonable GPU will allow frames to lag.
            for (int i = 0; i < FLAGS_gpuFrameLag; i++) {
                elapsed = time(loops, bench, canvas, gl);
            }
        } while (elapsed < FLAGS_gpuMs);

        // We've overshot at least a little.  Scale back linearly.
        loops = (int)ceil(loops * FLAGS_gpuMs / elapsed);

        // Might as well make sure we're not still timing our calibration.
        SK_GL(*gl, Finish());
    }

    // Pretty much the same deal as the calibration: do some warmup to make
    // sure we're timing steady-state pipelined frames.
    for (int i = 0; i < FLAGS_gpuFrameLag; i++) {
        time(loops, bench, canvas, gl);
    }

    // Now, actually do the timing!
    for (int i = 0; i < FLAGS_samples; i++) {
        samples[i] = time(loops, bench, canvas, gl) / loops;
    }
    return loops;
}
#endif

static SkString to_lower(const char* str) {
    SkString lower(str);
    for (size_t i = 0; i < lower.size(); i++) {
        lower[i] = tolower(lower[i]);
    }
    return lower;
}

struct Target {
    const char* config;
    Benchmark::Backend backend;
    SkAutoTDelete<SkSurface> surface;
#if SK_SUPPORT_GPU
    SkGLContextHelper* gl;
#endif
};

// If bench is enabled for backend/config, returns a Target* for them, otherwise NULL.
static Target* is_enabled(Benchmark* bench, Benchmark::Backend backend, const char* config) {
    if (!bench->isSuitableFor(backend)) {
        return NULL;
    }

    for (int i = 0; i < FLAGS_config.count(); i++) {
        if (to_lower(FLAGS_config[i]).equals(config)) {
            Target* target = new Target;
            target->config  = config;
            target->backend = backend;
            return target;
        }
    }
    return NULL;
}

// Append all targets that are suitable for bench.
static void create_targets(Benchmark* bench, SkTDArray<Target*>* targets) {
    const int w = bench->getSize().fX,
              h = bench->getSize().fY;
    const SkImageInfo _8888 = { w, h, kN32_SkColorType,     kPremul_SkAlphaType },
                       _565 = { w, h, kRGB_565_SkColorType, kOpaque_SkAlphaType };

    #define CPU_TARGET(config, backend, code)                              \
        if (Target* t = is_enabled(bench, Benchmark::backend, #config)) {  \
            t->surface.reset(code);                                        \
            targets->push(t);                                              \
        }
    if (FLAGS_cpu) {
        CPU_TARGET(nonrendering, kNonRendering_Backend, NULL)
        CPU_TARGET(8888, kRaster_Backend, SkSurface::NewRaster(_8888))
        CPU_TARGET(565,  kRaster_Backend, SkSurface::NewRaster(_565))
    }

#if SK_SUPPORT_GPU
    if (FLAGS_resetGpuContext) {
        gGrFactory.destroyContexts();
    }

    #define GPU_TARGET(config, ctxType, info, samples)                                            \
        if (Target* t = is_enabled(bench, Benchmark::kGPU_Backend, #config)) {                    \
            t->surface.reset(SkSurface::NewRenderTarget(gGrFactory.get(ctxType), info, samples)); \
            t->gl = gGrFactory.getGLContext(ctxType);                                             \
            targets->push(t);                                                                     \
        }
    if (FLAGS_gpu) {
        GPU_TARGET(gpu,      GrContextFactory::kNative_GLContextType, _8888, 0)
        GPU_TARGET(msaa4,    GrContextFactory::kNative_GLContextType, _8888, 4)
        GPU_TARGET(msaa16,   GrContextFactory::kNative_GLContextType, _8888, 16)
        GPU_TARGET(nvprmsaa4,  GrContextFactory::kNVPR_GLContextType, _8888, 4)
        GPU_TARGET(nvprmsaa16, GrContextFactory::kNVPR_GLContextType, _8888, 16)
        GPU_TARGET(debug,     GrContextFactory::kDebug_GLContextType, _8888, 0)
        GPU_TARGET(nullgpu,    GrContextFactory::kNull_GLContextType, _8888, 0)
        #if SK_ANGLE
            GPU_TARGET(angle, GrContextFactory::kANGLE_GLContextType, _8888, 0)
        #endif
    }
#endif
}

static void fill_static_options(ResultsWriter* log) {
#if defined(SK_BUILD_FOR_WIN32)
    log->option("system", "WIN32");
#elif defined(SK_BUILD_FOR_MAC)
    log->option("system", "MAC");
#elif defined(SK_BUILD_FOR_ANDROID)
    log->option("system", "ANDROID");
#elif defined(SK_BUILD_FOR_UNIX)
    log->option("system", "UNIX");
#else
    log->option("system", "other");
#endif
#if defined(SK_DEBUG)
    log->option("build", "DEBUG");
#else
    log->option("build", "RELEASE");
#endif
}

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
    SetupCrashHandler();
    SkAutoGraphics ag;
    SkCommandLineFlags::Parse(argc, argv);

    if (FLAGS_runOnce) {
        FLAGS_samples     = 1;
        FLAGS_gpuFrameLag = 0;
    }

    MultiResultsWriter log;
    SkAutoTDelete<JSONResultsWriter> json;
    if (!FLAGS_outResultsFile.isEmpty()) {
        json.reset(SkNEW(JSONResultsWriter(FLAGS_outResultsFile[0])));
        log.add(json.get());
    }
    CallEnd<MultiResultsWriter> ender(log);
    fill_static_options(&log);

    const double overhead = estimate_timer_overhead();
    SkAutoTMalloc<double> samples(FLAGS_samples);

    if (FLAGS_runOnce) {
        SkDebugf("--runOnce is true; times would only be misleading so we won't print them.\n");
    } else if (FLAGS_verbose) {
        // No header.
    } else if (FLAGS_quiet) {
        SkDebugf("median\tbench\tconfig\n");
    } else {
        SkDebugf("loops\tmin\tmedian\tmean\tmax\tstddev\tsamples\tconfig\tbench\n");
    }

    for (const BenchRegistry* r = BenchRegistry::Head(); r != NULL; r = r->next()) {
        SkAutoTDelete<Benchmark> bench(r->factory()(NULL));
        if (SkCommandLineFlags::ShouldSkip(FLAGS_match, bench->getName())) {
            continue;
        }
        log.bench(bench->getName(), bench->getSize().fX, bench->getSize().fY);

        SkTDArray<Target*> targets;
        create_targets(bench.get(), &targets);

        bench->preDraw();
        for (int j = 0; j < targets.count(); j++) {
            SkCanvas* canvas = targets[j]->surface.get() ? targets[j]->surface->getCanvas() : NULL;

            const int loops =
#if SK_SUPPORT_GPU
                Benchmark::kGPU_Backend == targets[j]->backend
                ? gpu_bench(targets[j]->gl, bench.get(), canvas, samples.get())
                :
#endif
                 cpu_bench(       overhead, bench.get(), canvas, samples.get());

            Stats stats(samples.get(), FLAGS_samples);

            const char* config = targets[j]->config;

            log.config(config);
            log.timer("min_ms",    stats.min);
            log.timer("median_ms", stats.median);
            log.timer("mean_ms",   stats.mean);
            log.timer("max_ms",    stats.max);
            log.timer("stddev_ms", sqrt(stats.var));

            if (FLAGS_runOnce) {
                if (targets.count() == 1) {
                    config = ""; // Only print the config if we run the same bench on more than one.
                }
                SkDebugf("%s\t%s\n", bench->getName(), config);
            } else if (FLAGS_verbose) {
                for (int i = 0; i < FLAGS_samples; i++) {
                    SkDebugf("%s  ", humanize(samples[i]).c_str());
                }
                SkDebugf("%s\n", bench->getName());
            } else if (FLAGS_quiet) {
                if (targets.count() == 1) {
                    config = ""; // Only print the config if we run the same bench on more than one.
                }
                SkDebugf("%s\t%s\t%s\n", humanize(stats.median).c_str(), bench->getName(), config);
            } else {
                const double stddev_percent = 100 * sqrt(stats.var) / stats.mean;
                SkDebugf("%d\t%s\t%s\t%s\t%s\t%.0f%%\t%s\t%s\t%s\n"
                        , loops
                        , humanize(stats.min).c_str()
                        , humanize(stats.median).c_str()
                        , humanize(stats.mean).c_str()
                        , humanize(stats.max).c_str()
                        , stddev_percent
                        , stats.plot.c_str()
                        , config
                        , bench->getName()
                        );
            }
        }
        targets.deleteAll();
    }

    return 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
