// Copyright 2021 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "include/core/SkCanvas.h"
#include "include/core/SkGraphics.h"
#include "include/gpu/GrDirectContext.h"

#include "tools/DDLPromiseImageHelper.h"
#include "tools/DDLTileHelper.h"

#include "tools/flags/CommandLineFlags.h"
#include "tools/gpu/GrContextFactory.h"
#include "tools/gpu/TestContext.h"

#include "src/sksl/SkSLDefines.h"

#include <chrono>
#include <deque>
#include <thread>

using hires_clock = std::chrono::high_resolution_clock;
using duration = std::chrono::nanoseconds;

using sk_gpu_test::ContextInfo;
using sk_gpu_test::GrContextFactory;
using sk_gpu_test::TestContext;

static DEFINE_int(ddlNumRecordingThreads, 1, "number of DDL recording threads");
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

int main(int argc, char** argv) {
    CommandLineFlags::Parse(argc, argv);

    if (FLAGS_src.count() != 1) {
        exitf("Missing input file");
    }

    // TODO: get these from the command line flags
    const GrContextFactory::ContextType kContextType = GrContextFactory::kGL_ContextType;
    const GrContextOptions kContextOptions;
    const GrContextFactory::ContextOverrides kOverrides = GrContextFactory::ContextOverrides::kNone;

    SkGraphics::Init();

    GrContextFactory factory(kContextOptions);

    std::unique_ptr<ThreadInfo> mainContext(new ThreadInfo);
    std::unique_ptr<ThreadInfo[]> utilityContexts(new ThreadInfo[FLAGS_ddlNumRecordingThreads]);

    if (!create_contexts(&factory,
                         kContextType,
                         kOverrides,
                         mainContext.get(),
                         SkSpan<ThreadInfo>(utilityContexts.get(), FLAGS_ddlNumRecordingThreads))) {
        return 1;
    }

    mainContext->fTestContext->makeCurrent();

    SkYUVAPixmapInfo::SupportedDataTypes supportedYUVADTypes(*mainContext->fDirectContext);
    DDLPromiseImageHelper promiseImageHelper(supportedYUVADTypes);

    sk_sp<SkPicture> skp = create_shared_skp(FLAGS_src[0],
                                             mainContext->fDirectContext,
                                             &promiseImageHelper);

    promiseImageHelper.createCallbackContexts(mainContext->fDirectContext);

    // TODO: do this later on a utility thread!
    promiseImageHelper.uploadAllToGPU(nullptr, mainContext->fDirectContext);

    mainContext->fTestContext->makeNotCurrent();

    {
        GrTaskGroup gpuTaskGroup(SkSpan<ThreadInfo>(mainContext.get(), 1));
        GrTaskGroup recordingTaskGroup(SkSpan<ThreadInfo>(utilityContexts.get(),
                                                          FLAGS_ddlNumRecordingThreads));

        for (int i = 0; i < FLAGS_ddlNumRecordingThreads; ++i) {
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
    }

    mainContext->fTestContext->makeCurrent();

    mainContext->dump();
    for (int i = 0; i < FLAGS_ddlNumRecordingThreads; ++i) {
        utilityContexts[i].dump();
    }

    return 0;
}
