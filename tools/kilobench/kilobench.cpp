/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCaps.h"
#include "GrContextFactory.h"
#include "Benchmark.h"
#include "ResultsWriter.h"
#include "SkCommandLineFlags.h"
#include "SkOSFile.h"
#include "SkStream.h"
#include "SkSurface.h"
#include "SkTime.h"
#include "Stats.h"
#include "Timer.h"
#include "VisualSKPBench.h"
#include "gl/GrGLDefines.h"

// posix only for now
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/*
 * This is an experimental GPU only benchmarking program.  The initial implementation will only
 * support SKPs.
 */

// To get image decoders linked in we have to do the below magic
#include "SkForceLinking.h"
#include "SkImageDecoder.h"
__SK_FORCE_IMAGE_DECODER_LINKING;


static const int kAutoTuneLoops = 0;

static const int kDefaultLoops =
#ifdef SK_DEBUG
    1;
#else
    kAutoTuneLoops;
#endif

static SkString loops_help_txt() {
    SkString help;
    help.printf("Number of times to run each bench. Set this to %d to auto-"
                "tune for each bench. Timings are only reported when auto-tuning.",
                kAutoTuneLoops);
    return help;
}

DEFINE_string(skps, "skps", "Directory to read skps from.");
DEFINE_string2(match, m, nullptr,
               "[~][^]substring[$] [...] of GM name to run.\n"
               "Multiple matches may be separated by spaces.\n"
               "~ causes a matching bench to always be skipped\n"
               "^ requires the start of the bench to match\n"
               "$ requires the end of the bench to match\n"
               "^ and $ requires an exact match\n"
               "If a bench does not match any list entry,\n"
               "it is skipped unless some list entry starts with ~");
DEFINE_int32(gpuFrameLag, 5, "If unknown, estimated maximum number of frames GPU allows to lag.");
DEFINE_int32(samples, 10, "Number of samples to measure for each bench.");
DEFINE_int32(maxLoops, 1000000, "Never run a bench more times than this.");
DEFINE_int32(loops, kDefaultLoops, loops_help_txt().c_str());
DEFINE_double(gpuMs, 5, "Target bench time in millseconds for GPU.");
DEFINE_string2(writePath, w, "", "If set, write bitmaps here as .pngs.");

static SkString humanize(double ms) {
    return HumanizeMs(ms);
}
#define HUMANIZE(ms) humanize(ms).c_str()

namespace kilobench {
class BenchmarkStream {
public:
    BenchmarkStream() : fCurrentSKP(0) {
        for (int i = 0; i < FLAGS_skps.count(); i++) {
            if (SkStrEndsWith(FLAGS_skps[i], ".skp")) {
                fSKPs.push_back() = FLAGS_skps[i];
            } else {
                SkOSFile::Iter it(FLAGS_skps[i], ".skp");
                SkString path;
                while (it.next(&path)) {
                    fSKPs.push_back() = SkOSPath::Join(FLAGS_skps[0], path.c_str());
                }
            }
        }
    }

    Benchmark* next() {
        Benchmark* bench = nullptr;
        // skips non matching benches
        while ((bench = this->innerNext()) &&
               (SkCommandLineFlags::ShouldSkip(FLAGS_match, bench->getUniqueName()) ||
                !bench->isSuitableFor(Benchmark::kGPU_Backend))) {
            delete bench;
        }
        return bench;
    }

private:
    static bool ReadPicture(const char* path, SkAutoTUnref<SkPicture>* pic) {
        // Not strictly necessary, as it will be checked again later,
        // but helps to avoid a lot of pointless work if we're going to skip it.
        if (SkCommandLineFlags::ShouldSkip(FLAGS_match, path)) {
            return false;
        }

        SkAutoTDelete<SkStream> stream(SkStream::NewFromFile(path));
        if (stream.get() == nullptr) {
            SkDebugf("Could not read %s.\n", path);
            return false;
        }

        pic->reset(SkPicture::CreateFromStream(stream.get()));
        if (pic->get() == nullptr) {
            SkDebugf("Could not read %s as an SkPicture.\n", path);
            return false;
        }
        return true;
    }

    Benchmark* innerNext() {
        // Render skps
        while (fCurrentSKP < fSKPs.count()) {
            const SkString& path = fSKPs[fCurrentSKP++];
            SkAutoTUnref<SkPicture> pic;
            if (!ReadPicture(path.c_str(), &pic)) {
                continue;
            }

            SkString name = SkOSPath::Basename(path.c_str());
            return new VisualSKPBench(name.c_str(), pic.get());
        }

        return nullptr;
    }

    SkTArray<SkString> fSKPs;
    int fCurrentSKP;
};

struct GPUTarget {
    void setup() {
        this->gl->makeCurrent();
        // Make sure we're done with whatever came before.
        SK_GL(*this->gl, Finish());
    }

    SkCanvas* beginTiming(SkCanvas* canvas) { return canvas; }

    void endTiming() {
        if (this->gl) {
            SK_GL(*this->gl, Flush());
            this->gl->swapBuffers();
        }
    }
    void fence() {
        SK_GL(*this->gl, Finish());
    }

    bool needsFrameTiming(int* maxFrameLag) const {
        if (!this->gl->getMaxGpuFrameLag(maxFrameLag)) {
            // Frame lag is unknown.
            *maxFrameLag = FLAGS_gpuFrameLag;
        }
        return true;
    }

    bool init(Benchmark* bench, GrContextFactory* factory, bool useDfText,
              GrContextFactory::GLContextType ctxType,
              GrContextFactory::GLContextOptions ctxOptions, int numSamples) {
        GrContext* context = factory->get(ctxType, ctxOptions);
        int maxRTSize = context->caps()->maxRenderTargetSize();
        SkImageInfo info = SkImageInfo::Make(SkTMin(bench->getSize().fX, maxRTSize),
                                             SkTMin(bench->getSize().fY, maxRTSize),
                                              kN32_SkColorType, kPremul_SkAlphaType);
        uint32_t flags = useDfText ? SkSurfaceProps::kUseDeviceIndependentFonts_Flag :
                                                  0;
        SkSurfaceProps props(flags, SkSurfaceProps::kLegacyFontHost_InitType);
        this->surface.reset(SkSurface::NewRenderTarget(context,
                                                       SkSurface::kNo_Budgeted, info,
                                                       numSamples, &props));
        this->gl = factory->getContextInfo(ctxType, ctxOptions).fGLContext;
        if (!this->surface.get()) {
            return false;
        }

        // Kilobench should only be used on platforms with fence sync support
        SkASSERT(this->gl->fenceSyncSupport());
        return true;
    }

    SkCanvas* getCanvas() const {
        if (!surface.get()) {
            return nullptr;
        }
        return surface->getCanvas();
    }

    bool capturePixels(SkBitmap* bmp) {
        SkCanvas* canvas = this->getCanvas();
        if (!canvas) {
            return false;
        }
        bmp->setInfo(canvas->imageInfo());
        if (!canvas->readPixels(bmp, 0, 0)) {
            SkDebugf("Can't read canvas pixels.\n");
            return false;
        }
        return true;
    }

private:
    //const Config config;
    SkGLContext* gl;
    SkAutoTDelete<SkSurface> surface;
};

static bool write_canvas_png(GPUTarget* target, const SkString& filename) {

    if (filename.isEmpty()) {
        return false;
    }
    if (target->getCanvas() &&
        kUnknown_SkColorType == target->getCanvas()->imageInfo().colorType()) {
        return false;
    }

    SkBitmap bmp;

    if (!target->capturePixels(&bmp)) {
        return false;
    }

    SkString dir = SkOSPath::Dirname(filename.c_str());
    if (!sk_mkdir(dir.c_str())) {
        SkDebugf("Can't make dir %s.\n", dir.c_str());
        return false;
    }
    SkFILEWStream stream(filename.c_str());
    if (!stream.isValid()) {
        SkDebugf("Can't write %s.\n", filename.c_str());
        return false;
    }
    if (!SkImageEncoder::EncodeStream(&stream, bmp, SkImageEncoder::kPNG_Type, 100)) {
        SkDebugf("Can't encode a PNG.\n");
        return false;
    }
    return true;
}

static int detect_forever_loops(int loops) {
    // look for a magic run-forever value
    if (loops < 0) {
        loops = SK_MaxS32;
    }
    return loops;
}

static int clamp_loops(int loops) {
    if (loops < 1) {
        SkDebugf("ERROR: clamping loops from %d to 1. "
                 "There's probably something wrong with the bench.\n", loops);
        return 1;
    }
    if (loops > FLAGS_maxLoops) {
        SkDebugf("WARNING: clamping loops from %d to FLAGS_maxLoops, %d.\n", loops, FLAGS_maxLoops);
        return FLAGS_maxLoops;
    }
    return loops;
}

static double now_ms() { return SkTime::GetNSecs() * 1e-6; }
static double time(int loops, Benchmark* bench, GPUTarget* target) {
    SkCanvas* canvas = target->getCanvas();
    if (canvas) {
        canvas->clear(SK_ColorWHITE);
    }
    bench->preDraw(canvas);
    double start = now_ms();
    canvas = target->beginTiming(canvas);
    bench->draw(loops, canvas);
    if (canvas) {
        canvas->flush();
    }
    target->endTiming();
    double elapsed = now_ms() - start;
    bench->postDraw(canvas);
    return elapsed;
}

static int setup_gpu_bench(GPUTarget* target, Benchmark* bench, int maxGpuFrameLag) {
    // First, figure out how many loops it'll take to get a frame up to FLAGS_gpuMs.
    int loops = bench->calculateLoops(FLAGS_loops);
    if (kAutoTuneLoops == loops) {
        loops = 1;
        double elapsed = 0;
        do {
            if (1<<30 == loops) {
                // We're about to wrap.  Something's wrong with the bench.
                loops = 0;
                break;
            }
            loops *= 2;
            // If the GPU lets frames lag at all, we need to make sure we're timing
            // _this_ round, not still timing last round.
            for (int i = 0; i < maxGpuFrameLag; i++) {
                elapsed = time(loops, bench, target);
            }
        } while (elapsed < FLAGS_gpuMs);

        // We've overshot at least a little.  Scale back linearly.
        loops = (int)ceil(loops * FLAGS_gpuMs / elapsed);
        loops = clamp_loops(loops);

        // Make sure we're not still timing our calibration.
        target->fence();
    } else {
        loops = detect_forever_loops(loops);
    }

    // Pretty much the same deal as the calibration: do some warmup to make
    // sure we're timing steady-state pipelined frames.
    for (int i = 0; i < maxGpuFrameLag - 1; i++) {
        time(loops, bench, target);
    }

    return loops;
}

struct AutoSetupContextBenchAndTarget {
    AutoSetupContextBenchAndTarget(Benchmark* bench) : fBenchmark(bench) {
        GrContextOptions grContextOpts;
        fCtxFactory.reset(new GrContextFactory(grContextOpts));

        SkAssertResult(fTarget.init(bench, fCtxFactory, false,
                                    GrContextFactory::kNative_GLContextType,
                                    GrContextFactory::kNone_GLContextOptions, 0));

        fCanvas = fTarget.getCanvas();
        fTarget.setup();

        bench->perCanvasPreDraw(fCanvas);
        fTarget.needsFrameTiming(&fMaxFrameLag);
    }

    int getLoops() { return setup_gpu_bench(&fTarget, fBenchmark, fMaxFrameLag); }

    double timeSample(int loops) {
        for (int i = 0; i < fMaxFrameLag; i++) {
            time(loops, fBenchmark, &fTarget);
        }

        return time(loops, fBenchmark, &fTarget) / loops;
    }
    void teardownBench() { fBenchmark->perCanvasPostDraw(fCanvas); }

    SkAutoTDelete<GrContextFactory> fCtxFactory;
    GPUTarget fTarget;
    SkCanvas* fCanvas;
    Benchmark* fBenchmark;
    int fMaxFrameLag;
};

int setup_loops(Benchmark* bench) {
    AutoSetupContextBenchAndTarget ascbt(bench);
    int loops = ascbt.getLoops();
    ascbt.teardownBench();

    if (!FLAGS_writePath.isEmpty() && FLAGS_writePath[0]) {
        SkString pngFilename = SkOSPath::Join(FLAGS_writePath[0], "gpu");
        pngFilename = SkOSPath::Join(pngFilename.c_str(), bench->getUniqueName());
        pngFilename.append(".png");
        write_canvas_png(&ascbt.fTarget, pngFilename);
    }
    return loops;
}

double time_sample(Benchmark* bench, int loops) {
    AutoSetupContextBenchAndTarget ascbt(bench);
    double sample = ascbt.timeSample(loops);
    ascbt.teardownBench();

    return sample;
}

} // namespace kilobench

static const int kOutResultSize = 1024;

int kilobench_main() {
    kilobench::BenchmarkStream benchStream;

    SkDebugf("loops\tmin\tmedian\tmean\tmax\tstddev\t%-*s\tconfig\tbench\n",
             FLAGS_samples, "samples");

    int descriptors[2];
    if (pipe(descriptors) != 0) {
        SkFAIL("Failed to open a pipe\n");
    }

    while (Benchmark* b = benchStream.next()) {
        SkAutoTDelete<Benchmark> bench(b);

        int loops;
        SkTArray<double> samples;
        for (int i = 0; i < FLAGS_samples + 1; i++) {
            // We fork off a new process to setup the grcontext and run the test while we wait
            int childPid = fork();
            if (childPid > 0) {
                char result[kOutResultSize];
                if (read(descriptors[0], result, kOutResultSize) < 0) {
                     SkFAIL("Failed to read from pipe\n");
                }

                // if samples == 0 then parse # of loops
                // else parse float
                if (i == 0) {
                    sscanf(result, "%d", &loops);
                } else {
                    sscanf(result, "%lf", &samples.push_back());
                }

                // wait until exit
                int status;
                waitpid(childPid, &status, 0);
            } else if (0 == childPid) {
                char result[kOutResultSize];
                if (i == 0) {
                    sprintf(result, "%d", kilobench::setup_loops(bench));
                } else {
                    sprintf(result, "%lf", kilobench::time_sample(bench, loops));
                }

                // Make sure to write the null terminator
                if (write(descriptors[1], result, strlen(result) + 1) < 0) {
                    SkFAIL("Failed to write to pipe\n");
                }
                return 0;
            } else {
                SkFAIL("Fork failed\n");
            }
        }

        Stats stats(samples);
        const double stddev_percent = 100 * sqrt(stats.var) / stats.mean;
        SkDebugf("%d\t%s\t%s\t%s\t%s\t%.0f%%\t%s\t%s\t%s\n"
                , loops
                , HUMANIZE(stats.min)
                , HUMANIZE(stats.median)
                , HUMANIZE(stats.mean)
                , HUMANIZE(stats.max)
                , stddev_percent
                , stats.plot.c_str()
                , "gpu"
                , bench->getUniqueName()
                );

    }
    return 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char** argv) {
    SkCommandLineFlags::Parse(argc, argv);
    return kilobench_main();
}
#endif
