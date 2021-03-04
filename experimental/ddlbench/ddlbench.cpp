// Copyright 2021 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "include/core/SkCanvas.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/private/SkSLDefines.h"

#include "src/core/SkOSFile.h"

#include "src/gpu/GrDirectContextPriv.h"

#include "src/utils/SkOSPath.h"

#include "tools/DDLPromiseImageHelper.h"
#include "tools/DDLTileHelper.h"
#include "tools/ToolUtils.h"

#include "tools/flags/CommandLineFlags.h"
#include "tools/gpu/GrContextFactory.h"
#include "tools/gpu/TestContext.h"

#include <chrono>
#include <deque>
#include <thread>

using hires_clock = std::chrono::high_resolution_clock;
using duration = std::chrono::nanoseconds;

using sk_gpu_test::ContextInfo;
using sk_gpu_test::GrContextFactory;
using sk_gpu_test::TestContext;

static DEFINE_int(numRecordingThreads, 1, "number of DDL recording threads");
static DEFINE_int(numTilesX, 3, "number of tiles horizontally");
static DEFINE_int(numTilesY, 3, "number of tiles vertically");
static DEFINE_int(numSamples, 1, "number of MSAA samples");
static DEFINE_string(png, "", "if set, save a .png proof to disk at this file location");
static DEFINE_string(src, "", "input .skp file");

static void exitf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    exit(1);
}

// Each thread holds this chunk of data thread_locally
struct ThreadInfo {
    ThreadInfo() = default;

    ThreadInfo(const SkString& name, GrDirectContext* directContext, TestContext* testContext)
            : fName(name)
            , fDirectContext(directContext)
            , fTestContext(testContext) {
    }

    double elapsedWorkSeconds() const {
        return std::chrono::duration<double>(fWorkElapsed).count();
    }

    void dump() const {
        duration totalThreadTime = fThreadStop - fThreadStart;
        double totalThreadTimeSeconds = std::chrono::duration<double>(totalThreadTime).count();

        printf("%s: num work units %d work: %.2gs total: %.2gs utilization %.2g%%\n",
               fName.c_str(),
               fWorkUnit,
               this->elapsedWorkSeconds(),
               totalThreadTimeSeconds,
               100.0f * this->elapsedWorkSeconds() / totalThreadTimeSeconds);
    }

    SkString                fName;

    // These two can be null on recording/utility threads
    GrDirectContext*        fDirectContext = nullptr;
    TestContext*            fTestContext = nullptr;

    int                     fWorkUnit = 0;
    duration                fWorkElapsed {0};
    hires_clock::time_point fThreadStart;
    hires_clock::time_point fThreadStop;
};

#if SKSL_USE_THREAD_LOCAL

static thread_local ThreadInfo* gThreadInfo;

static ThreadInfo* get_thread_local_info() {
    return gThreadInfo;
}

static void set_thread_local_info(ThreadInfo* threadInfo) {
    gThreadInfo = threadInfo;
}

#else

#include <pthread.h>

static pthread_key_t get_pthread_key() {
    static pthread_key_t sKey = []{
        pthread_key_t key;
        int result = pthread_key_create(&key, /*destructor=*/nullptr);
        if (result != 0) {
            SK_ABORT("pthread_key_create failure: %d", result);
        }
        return key;
    }();
    return sKey;
}

static ThreadInfo* get_thread_local_info() {
    return static_cast<ThreadInfo*>(pthread_getspecific(get_pthread_key()));
}

static void set_thread_local_info(ThreadInfo* threadInfo) {
    pthread_setspecific(get_pthread_key(), threadInfo);
}

#endif

static void set_up_context_on_thread(ThreadInfo* threadInfo) {
    if (threadInfo->fDirectContext) {
        threadInfo->fTestContext->makeCurrent();
    }

    threadInfo->fThreadStart = hires_clock::now();

    set_thread_local_info(threadInfo);
}

class GrThreadPool {
public:
    explicit GrThreadPool(SkSpan<ThreadInfo> threadInfo) {
        for (size_t i = 0; i < threadInfo.size(); i++) {
            fThreads.emplace_back(&Loop, this, &threadInfo[i]);
        }
    }

    ~GrThreadPool() {
        // Signal each thread that it's time to shut down.
        for (int i = 0; i < fThreads.count(); i++) {
            this->add(nullptr);
        }
        // Wait for each thread to shut down.
        for (int i = 0; i < fThreads.count(); i++) {
            fThreads[i].join();
        }
    }

    void add(std::function<void(void)> work) {
        // Add some work to our pile of work to do.
        {
            SkAutoMutexExclusive lock(fWorkLock);
            fWork.emplace_back(std::move(work));
        }
        // Tell the Loop() threads to pick it up.
        fWorkAvailable.signal(1);
    }

private:
    // This method should be called only when fWorkAvailable indicates there's work to do.
    bool do_work(ThreadInfo* threadInfo) {
        std::function<void(void)> work;
        {
            SkAutoMutexExclusive lock(fWorkLock);
            SkASSERT(!fWork.empty());        // TODO: if (fWork.empty()) { return true; } ?
            work = std::move(fWork.front());
            fWork.pop_front();
        }

        if (!work) {
            threadInfo->fThreadStop = hires_clock::now();
            return false;  // This is Loop()'s signal to shut down.
        }

        hires_clock::time_point start = hires_clock::now();
        work();
        threadInfo->fWorkElapsed = hires_clock::now() - start;
        threadInfo->fWorkUnit++;

        return true;
    }

    static void Loop(void* ctx, ThreadInfo* threadInfo) {
        set_up_context_on_thread(threadInfo);

        auto pool = (GrThreadPool*)ctx;
        do {
            pool->fWorkAvailable.wait();
        } while (pool->do_work(threadInfo));
    }

    using WorkList = std::deque<std::function<void(void)>>;

    SkTArray<std::thread> fThreads;
    WorkList              fWork;
    SkMutex               fWorkLock;
    SkSemaphore           fWorkAvailable;
};

class GrTaskGroup : SkNoncopyable {
public:
    explicit GrTaskGroup(SkSpan<ThreadInfo> threadInfo)
            : fPending(0)
            , fThreadPool(std::make_unique<GrThreadPool>(threadInfo)) {
    }

    ~GrTaskGroup() { this->wait(); }

    // Add a task to this SkTaskGroup.
    void add(std::function<void(void)> fn) {
        fPending.fetch_add(+1, std::memory_order_relaxed);
        fThreadPool->add([this, fn{std::move(fn)}] {
            fn();
            fPending.fetch_add(-1, std::memory_order_release);
        });
    }

    // Returns true if all Tasks previously add()ed to this SkTaskGroup have run.
    // It is safe to reuse this SkTaskGroup once done().
    bool done() const {
        return fPending.load(std::memory_order_acquire) == 0;
    }

    // Block until done().
    void wait() {
        while (!this->done()) {
            std::this_thread::yield();
        }
    }

private:
    std::atomic<int32_t>          fPending;
    std::unique_ptr<GrThreadPool> fThreadPool;
};

static bool create_contexts(GrContextFactory* factory,
                            GrContextFactory::ContextType contextType,
                            const GrContextFactory::ContextOverrides& overrides,
                            ThreadInfo* gpuThread,
                            SkSpan<ThreadInfo> utilityThreads) {

    ContextInfo mainInfo = factory->getContextInfo(contextType, overrides);
    if (!mainInfo.directContext()) {
        exitf("Could not create primary direct context.");
    }

    *gpuThread = { SkString("g0"), mainInfo.directContext(), mainInfo.testContext() };

    bool allSucceeded = true, allFailed = true;
    // Create the utility contexts in a share group with the primary one. This is allowed to fail
    // but either they should all work or the should all fail.
    for (size_t i = 0; i < utilityThreads.size(); ++i) {
        SkString name = SkStringPrintf("r%zu", i);

        ContextInfo tmp = factory->getSharedContextInfo(mainInfo.directContext(), i);

        utilityThreads[i] = { name, tmp.directContext(), tmp.testContext() };
        allSucceeded &= SkToBool(tmp.directContext());
        allFailed &= !tmp.directContext();
    }

    return allSucceeded || allFailed;
}

static sk_sp<SkPicture> create_shared_skp(const char* src,
                                          GrDirectContext* dContext,
                                          DDLPromiseImageHelper* promiseImageHelper) {
    SkString srcfile(src);

    std::unique_ptr<SkStream> srcstream(SkStream::MakeFromFile(srcfile.c_str()));
    if (!srcstream) {
        exitf("failed to open file %s", srcfile.c_str());
    }

    sk_sp<SkPicture> skp = SkPicture::MakeFromStream(srcstream.get());
    if (!skp) {
        exitf("failed to parse file %s", srcfile.c_str());
    }

    sk_sp<SkData> compressedPictureData = promiseImageHelper->deflateSKP(skp.get());
    if (!compressedPictureData) {
        exitf("skp deflation failed %s", srcfile.c_str());
    }

    // TODO: use the new shared promise images to just create one skp here

    return skp;
}

static void check_params(GrDirectContext* dContext,
                         int width, int height, SkColorType ct, SkAlphaType at, int numSamples) {

    if (dContext->maxRenderTargetSize() < std::max(width, height)) {
        exitf("render target size %ix%i not supported by platform (max: %i)",
              width, height, dContext->maxRenderTargetSize());
    }

    GrBackendFormat format = dContext->defaultBackendFormat(ct, GrRenderable::kYes);
    if (!format.isValid()) {
        exitf("failed to get GrBackendFormat from SkColorType: %d", ct);
    }

    int supportedSampleCount = dContext->priv().caps()->getRenderTargetSampleCount(numSamples,
                                                                                   format);
    if (supportedSampleCount != numSamples) {
        exitf("sample count %i not supported by platform", numSamples);
    }
}

static bool mkdir_p(const SkString& dirname) {
    if (dirname.isEmpty() || dirname == SkString("/")) {
        return true;
    }
    return mkdir_p(SkOSPath::Dirname(dirname.c_str())) && sk_mkdir(dirname.c_str());
}

static void maybe_save_file(SkSurface* surface) {
    if (FLAGS_png.isEmpty()) {
        return;
    }

    SkBitmap bmp;
    bmp.allocPixels(surface->imageInfo());
    if (!surface->getCanvas()->readPixels(bmp, 0, 0)) {
        exitf("failed to read canvas pixels for png");
    }
    if (!mkdir_p(SkOSPath::Dirname(FLAGS_png[0]))) {
        exitf("failed to create directory for png \"%s\"", FLAGS_png[0]);
    }
    if (!ToolUtils::EncodeImageToFile(FLAGS_png[0], bmp, SkEncodedImageFormat::kPNG, 100)) {
        exitf("failed to save png to \"%s\"", FLAGS_png[0]);
    }
}

int main(int argc, char** argv) {
    CommandLineFlags::Parse(argc, argv);

    if (FLAGS_src.count() != 1) {
        exitf("Missing input file");
    }

    // TODO: get these from the command line flags
    const GrContextFactory::ContextType kContextType = GrContextFactory::kGL_ContextType;
    const GrContextOptions kContextOptions;
    const GrContextFactory::ContextOverrides kOverrides = GrContextFactory::ContextOverrides::kNone;
    SkColorType ct = kRGBA_8888_SkColorType;
    SkAlphaType at = kPremul_SkAlphaType;

    SkGraphics::Init();

    GrContextFactory factory(kContextOptions);

    std::unique_ptr<ThreadInfo> mainContext(new ThreadInfo);
    std::unique_ptr<ThreadInfo[]> utilityContexts(new ThreadInfo[FLAGS_numRecordingThreads]);

    if (!create_contexts(&factory,
                         kContextType,
                         kOverrides,
                         mainContext.get(),
                         SkSpan<ThreadInfo>(utilityContexts.get(), FLAGS_numRecordingThreads))) {
        return 1;
    }

    mainContext->fTestContext->makeCurrent();

    SkYUVAPixmapInfo::SupportedDataTypes supportedYUVADTypes(*mainContext->fDirectContext);
    DDLPromiseImageHelper promiseImageHelper(supportedYUVADTypes);

    sk_sp<SkPicture> skp = create_shared_skp(FLAGS_src[0],
                                             mainContext->fDirectContext,
                                             &promiseImageHelper);

    int width = std::min(SkScalarCeilToInt(skp->cullRect().width()), 2048);
    int height = std::min(SkScalarCeilToInt(skp->cullRect().height()), 2048);

    check_params(mainContext->fDirectContext, width, height, ct, at, FLAGS_numSamples);

    promiseImageHelper.createCallbackContexts(mainContext->fDirectContext);

    // TODO: do this later on a utility thread!
    promiseImageHelper.uploadAllToGPU(nullptr, mainContext->fDirectContext);

    SkImageInfo info = SkImageInfo::Make(width, height, ct, at, nullptr);

    sk_sp<SkSurface> dstSurface = SkSurface::MakeRenderTarget(mainContext->fDirectContext,
                                                              SkBudgeted::kNo, info,
                                                              FLAGS_numSamples, nullptr);
    if (!dstSurface) {
        exitf("Could not create a surface.");
    }

    if (FLAGS_numRecordingThreads == 0) {
        mainContext->fThreadStart = hires_clock::now();

        dstSurface->getCanvas()->drawPicture(skp);

        mainContext->fThreadStop = hires_clock::now();

        mainContext->fWorkElapsed = mainContext->fThreadStop - mainContext->fThreadStart;
        mainContext->fWorkUnit++;
    } else {
        mainContext->fTestContext->makeNotCurrent();

        GrTaskGroup gpuTaskGroup(SkSpan<ThreadInfo>(mainContext.get(), 1));
        GrTaskGroup recordingTaskGroup(SkSpan<ThreadInfo>(utilityContexts.get(),
                                                          FLAGS_numRecordingThreads));

        for (int i = 0; i < FLAGS_numRecordingThreads; ++i) {
            recordingTaskGroup.add([] {
                                       ThreadInfo* threadLocal = get_thread_local_info();
                                       printf("%s: dContext %p\n", threadLocal->fName.c_str(),
                                                                   threadLocal->fDirectContext);
                                       std::this_thread::sleep_for(std::chrono::seconds(1));
                                   });
        }

        gpuTaskGroup.add([] {
                             ThreadInfo* threadLocal = get_thread_local_info();
                             printf("%s: dContext %p\n", threadLocal->fName.c_str(),
                                                         threadLocal->fDirectContext);
                         });

        gpuTaskGroup.add([] {
                             ThreadInfo* threadLocal = get_thread_local_info();
                             threadLocal->fTestContext->makeNotCurrent();
                         });

        recordingTaskGroup.wait();
        gpuTaskGroup.wait();

        mainContext->fTestContext->makeCurrent();
    }

    maybe_save_file(dstSurface.get());

    promiseImageHelper.deleteAllFromGPU(nullptr, mainContext->fDirectContext);

    // Dump out the timing stats
    mainContext->dump();
    for (int i = 0; i < FLAGS_numRecordingThreads; ++i) {
        utilityContexts[i].dump();
    }

    return 0;
}
