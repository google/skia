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
#include "SkTLList.h"
#include "SkThreadUtils.h"
#include "Stats.h"
#include "Timer.h"
#include "VisualSKPBench.h"
#include "gl/GrGLDefines.h"
#include "gl/GrGLUtil.h"
#include "../private/SkMutex.h"
#include "../private/SkSemaphore.h"
#include "../private/SkGpuFenceSync.h"

// posix only for now
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

using namespace sk_gpu_test;

/*
 * This is an experimental GPU only benchmarking program.  The initial implementation will only
 * support SKPs.
 */

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
DEFINE_bool(useBackgroundThread, true, "If false, kilobench will time cpu / gpu work together");
DEFINE_bool(useMultiProcess, true, "If false, kilobench will run all tests in one process");

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
    static sk_sp<SkPicture> ReadPicture(const char path[]) {
        // Not strictly necessary, as it will be checked again later,
        // but helps to avoid a lot of pointless work if we're going to skip it.
        if (SkCommandLineFlags::ShouldSkip(FLAGS_match, path)) {
            return nullptr;
        }

        SkAutoTDelete<SkStream> stream(SkStream::NewFromFile(path));
        if (stream.get() == nullptr) {
            SkDebugf("Could not read %s.\n", path);
            return nullptr;
        }

        return SkPicture::MakeFromStream(stream.get());
    }

    Benchmark* innerNext() {
        // Render skps
        while (fCurrentSKP < fSKPs.count()) {
            const SkString& path = fSKPs[fCurrentSKP++];
            auto pic = ReadPicture(path.c_str());
            if (!pic) {
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
        fGL->makeCurrent();
        // Make sure we're done with whatever came before.
        GR_GL_CALL(fGL->gl(), Finish());
    }

    SkCanvas* beginTiming(SkCanvas* canvas) { return canvas; }

    void endTiming(bool usePlatformSwapBuffers) {
        if (fGL) {
            GR_GL_CALL(fGL->gl(), Flush());
            if (usePlatformSwapBuffers) {
                fGL->swapBuffers();
            } else {
                fGL->waitOnSyncOrSwap();
            }
        }
    }
    void finish() {
        GR_GL_CALL(fGL->gl(), Finish());
    }

    bool needsFrameTiming(int* maxFrameLag) const {
        if (!fGL->getMaxGpuFrameLag(maxFrameLag)) {
            // Frame lag is unknown.
            *maxFrameLag = FLAGS_gpuFrameLag;
        }
        return true;
    }

    bool init(Benchmark* bench, GrContextFactory* factory, bool useDfText,
              GrContextFactory::ContextType ctxType,
              GrContextFactory::ContextOptions ctxOptions, int numSamples) {
        GrContext* context = factory->get(ctxType, ctxOptions);
        int maxRTSize = context->caps()->maxRenderTargetSize();
        SkImageInfo info = SkImageInfo::Make(SkTMin(bench->getSize().fX, maxRTSize),
                                             SkTMin(bench->getSize().fY, maxRTSize),
                                              kN32_SkColorType, kPremul_SkAlphaType);
        uint32_t flags = useDfText ? SkSurfaceProps::kUseDeviceIndependentFonts_Flag :
                                                  0;
        SkSurfaceProps props(flags, SkSurfaceProps::kLegacyFontHost_InitType);
        fSurface.reset(SkSurface::MakeRenderTarget(context,
                                                   SkBudgeted::kNo, info,
                                                   numSamples, &props).release());
        fGL = factory->getContextInfo(ctxType, ctxOptions).glContext();
        if (!fSurface.get()) {
            return false;
        }

        // Kilobench should only be used on platforms with fence sync support
        SkASSERT(fGL->fenceSyncSupport());
        return true;
    }

    SkCanvas* getCanvas() const {
        if (!fSurface.get()) {
            return nullptr;
        }
        return fSurface->getCanvas();
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

    GLTestContext* gl() { return fGL; }

private:
    GLTestContext* fGL;
    SkAutoTDelete<SkSurface> fSurface;
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

struct TimingThread {
    TimingThread(GLTestContext* mainContext)
        : fFenceSync(mainContext->fenceSync())
        ,  fMainContext(mainContext)
        ,  fDone(false) {}

    static void Loop(void* data) {
        TimingThread* timingThread = reinterpret_cast<TimingThread*>(data);
        timingThread->timingLoop();
    }

    // To ensure waiting for the sync actually does something, we check to make sure the we exceed
    // some small value
    const double kMinElapsed = 1e-6;
    bool sanity(double start) const {
        double elapsed = now_ms() - start;
        return elapsed > kMinElapsed;
    }

    void waitFence(SkPlatformGpuFence sync) {
        SkDEBUGCODE(double start = now_ms());
        fFenceSync->waitFence(sync);
        SkASSERT(sanity(start));
    }

    void timingLoop() {
        // Create a context which shares display lists with the main thread
        SkAutoTDelete<GLTestContext> glContext(CreatePlatformGLTestContext(kNone_GrGLStandard,
                                                                           fMainContext));
        glContext->makeCurrent();

        // Basic timing methodology is:
        // 1) Wait on semaphore until main thread indicates its time to start timing the frame
        // 2) Wait on frame start sync, record time.  This is start of the frame.
        // 3) Wait on semaphore until main thread indicates its time to finish timing the frame
        // 4) Wait on frame end sync, record time.  FrameEndTime - FrameStartTime = frame time
        // 5) Wait on semaphore until main thread indicates we should time the next frame or quit
        while (true) {
            fSemaphore.wait();

            // get start sync
            SkPlatformGpuFence startSync = this->popStartSync();

            // wait on sync
            this->waitFence(startSync);
            double start = kilobench::now_ms();

            // do we want to sleep here?
            // wait for end sync
            fSemaphore.wait();

            // get end sync
            SkPlatformGpuFence endSync = this->popEndSync();

            // wait on sync
            this->waitFence(endSync);
            double elapsed = kilobench::now_ms() - start;

            // No mutex needed, client won't touch timings until we're done
            fTimings.push_back(elapsed);

            // clean up fences
            fFenceSync->deleteFence(startSync);
            fFenceSync->deleteFence(endSync);

            fSemaphore.wait();
            if (this->isDone()) {
                break;
            }
        }
    }

    void pushStartSync() { this->pushSync(&fFrameStartSyncs, &fFrameStartSyncsMutex); }

    SkPlatformGpuFence popStartSync() {
        return this->popSync(&fFrameStartSyncs, &fFrameStartSyncsMutex);
    }

    void pushEndSync() { this->pushSync(&fFrameEndSyncs, &fFrameEndSyncsMutex); }

    SkPlatformGpuFence popEndSync() { return this->popSync(&fFrameEndSyncs, &fFrameEndSyncsMutex); }

    void setDone() {
        SkAutoMutexAcquire done(fDoneMutex);
        fDone = true;
        fSemaphore.signal();
    }

    typedef SkTLList<SkPlatformGpuFence, 1> SyncQueue;

    void pushSync(SyncQueue* queue, SkMutex* mutex) {
        SkAutoMutexAcquire am(mutex);
        *queue->addToHead() = fFenceSync->insertFence();
        fSemaphore.signal();
    }

    SkPlatformGpuFence popSync(SyncQueue* queue, SkMutex* mutex) {
        SkAutoMutexAcquire am(mutex);
        SkPlatformGpuFence sync = *queue->head();
        queue->popHead();
        return sync;
    }

    bool isDone() {
        SkAutoMutexAcquire am1(fFrameStartSyncsMutex);
        SkAutoMutexAcquire done(fDoneMutex);
        if (fDone && fFrameStartSyncs.isEmpty()) {
            return true;
        } else {
            return false;
        }
    }

    const SkTArray<double>& timings() const { SkASSERT(fDone); return fTimings; }

private:
    SkGpuFenceSync* fFenceSync;
    SkSemaphore fSemaphore;
    SkMutex fFrameStartSyncsMutex;
    SyncQueue fFrameStartSyncs;
    SkMutex fFrameEndSyncsMutex;
    SyncQueue fFrameEndSyncs;
    SkTArray<double> fTimings;
    SkMutex fDoneMutex;
    GLTestContext* fMainContext;
    bool fDone;
};

static double time(int loops, Benchmark* bench, GPUTarget* target, TimingThread* timingThread) {
    SkCanvas* canvas = target->getCanvas();
    canvas->clear(SK_ColorWHITE);
    bench->preDraw(canvas);

    if (timingThread) {
        timingThread->pushStartSync();
    }
    double start = now_ms();
    canvas = target->beginTiming(canvas);
    bench->draw(loops, canvas);
    canvas->flush();
    target->endTiming(timingThread ? true : false);

    double elapsed = now_ms() - start;
    if (timingThread) {
        timingThread->pushEndSync();
        timingThread->setDone();
    }
    bench->postDraw(canvas);
    return elapsed;
}

// TODO For now we don't use the background timing thread to tune loops
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
                elapsed = time(loops, bench, target, nullptr);
            }
        } while (elapsed < FLAGS_gpuMs);

        // We've overshot at least a little.  Scale back linearly.
        loops = (int)ceil(loops * FLAGS_gpuMs / elapsed);
        loops = clamp_loops(loops);

        // Make sure we're not still timing our calibration.
        target->finish();
    } else {
        loops = detect_forever_loops(loops);
    }

    // Pretty much the same deal as the calibration: do some warmup to make
    // sure we're timing steady-state pipelined frames.
    for (int i = 0; i < maxGpuFrameLag - 1; i++) {
        time(loops, bench, target, nullptr);
    }

    return loops;
}

struct AutoSetupContextBenchAndTarget {
    AutoSetupContextBenchAndTarget(Benchmark* bench) : fBenchmark(bench) {
        GrContextOptions grContextOpts;
        fCtxFactory.reset(new GrContextFactory(grContextOpts));

        SkAssertResult(fTarget.init(bench, fCtxFactory, false,
                                    GrContextFactory::kNativeGL_ContextType,
                                    GrContextFactory::kNone_ContextOptions, 0));

        fCanvas = fTarget.getCanvas();
        fTarget.setup();

        bench->perCanvasPreDraw(fCanvas);
        fTarget.needsFrameTiming(&fMaxFrameLag);
    }

    int getLoops() { return setup_gpu_bench(&fTarget, fBenchmark, fMaxFrameLag); }

    double timeSample(int loops, TimingThread* timingThread) {
        for (int i = 0; i < fMaxFrameLag; i++) {
            time(loops, fBenchmark, &fTarget, timingThread);
        }

        return time(loops, fBenchmark, &fTarget, timingThread) / loops;
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

struct Sample {
    double fCpu;
    double fGpu;
};

Sample time_sample(Benchmark* bench, int loops) {
    AutoSetupContextBenchAndTarget ascbt(bench);

    Sample sample;
    if (FLAGS_useBackgroundThread) {
        TimingThread timingThread(ascbt.fTarget.gl());
        SkAutoTDelete<SkThread> nativeThread(new SkThread(TimingThread::Loop, &timingThread));
        nativeThread->start();
        sample.fCpu = ascbt.timeSample(loops, &timingThread);
        nativeThread->join();

        // return the min
        double min = SK_ScalarMax;
        for (int i = 0; i < timingThread.timings().count(); i++) {
            min = SkTMin(min, timingThread.timings()[i]);
        }
        sample.fGpu = min;
    } else {
        sample.fCpu = ascbt.timeSample(loops, nullptr);
    }

    ascbt.teardownBench();

    return sample;
}

} // namespace kilobench

static const int kOutResultSize = 1024;

void printResult(const SkTArray<double>& samples, int loops, const char* name, const char* mod) {
    SkString newName(name);
    newName.appendf("_%s", mod);
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
        , newName.c_str()
    );
}

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

        int loops = 1;
        SkTArray<double> cpuSamples;
        SkTArray<double> gpuSamples;
        for (int i = 0; i < FLAGS_samples + 1; i++) {
            // We fork off a new process to setup the grcontext and run the test while we wait
            if (FLAGS_useMultiProcess) {
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
                        sscanf(result, "%lf %lf", &cpuSamples.push_back(),
                                                  &gpuSamples.push_back());
                    }

                    // wait until exit
                    int status;
                    waitpid(childPid, &status, 0);
                } else if (0 == childPid) {
                    char result[kOutResultSize];
                    if (i == 0) {
                        sprintf(result, "%d", kilobench::setup_loops(bench));
                    } else {
                        kilobench::Sample sample = kilobench::time_sample(bench, loops);
                        sprintf(result, "%lf %lf", sample.fCpu, sample.fGpu);
                    }

                    // Make sure to write the null terminator
                    if (write(descriptors[1], result, strlen(result) + 1) < 0) {
                        SkFAIL("Failed to write to pipe\n");
                    }
                    return 0;
                } else {
                    SkFAIL("Fork failed\n");
                }
            } else {
                if (i == 0) {
                    loops = kilobench::setup_loops(bench);
                } else {
                    kilobench::Sample sample = kilobench::time_sample(bench, loops);
                    cpuSamples.push_back(sample.fCpu);
                    gpuSamples.push_back(sample.fGpu);
                }
            }
        }

        printResult(cpuSamples, loops, bench->getUniqueName(), "cpu");
        if (FLAGS_useBackgroundThread) {
            printResult(gpuSamples, loops, bench->getUniqueName(), "gpu");
        }
    }
    return 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char** argv) {
    SkCommandLineFlags::Parse(argc, argv);
    return kilobench_main();
}
#endif
