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

#include <deque>
#include <thread>

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

    ThreadInfo(const char* name, GrDirectContext* directContext, TestContext* testContext)
        : fDirectContext(directContext)
        , fTestContext(testContext) {
        memcpy(fName, name, 8);
        fName[7] = '\0';
    }

    char             fName[8] = { '\0' };

    // These two can be null on recording/utility threads
    GrDirectContext* fDirectContext = nullptr;
    TestContext*     fTestContext = nullptr;
};

#if SKSL_USE_THREAD_LOCAL

static thread_local ThreadInfo gThreadInfo;

static ThreadInfo* get_thread_local_info() {
    return &gThreadInfo;
}

static void set_thread_local_info(const ThreadInfo& threadInfo) {
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

static void set_thread_local_info(const ThreadInfo& threadInfo) {
    pthread_setspecific(get_pthread_key(), nullptr);
}

#endif

static void set_up_context_on_thread(const ThreadInfo& threadInfo) {
    if (threadInfo.fDirectContext) {
        threadInfo.fTestContext->makeCurrent();
    }
    set_thread_local_info(threadInfo);
}

// TODO: upstream this back into SkThreadPool - the only difference is adding some initialization
// at the start of each thread and some thread_local data to hold the utility context/
class GrThreadPool {
public:
    static std::unique_ptr<GrThreadPool> MakeFIFOThreadPool(int threads,
                                                            std::vector<ThreadInfo>& contexts,
                                                            bool allowBorrowing = true) {
        SkASSERT(threads > 0);
        return std::make_unique<GrThreadPool>(threads, contexts, allowBorrowing);
    }

    explicit GrThreadPool(int threads, std::vector<ThreadInfo>& contexts, bool allowBorrowing)
            : fAllowBorrowing(allowBorrowing) {
        SkASSERT(((int)contexts.size()) >= threads);

        for (int i = 0; i < threads; i++) {
            fThreads.emplace_back(&Loop, this, contexts[i]);
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

    void borrow() {
        // If there is work waiting and we're allowed to borrow work, do it.
        if (fAllowBorrowing && fWorkAvailable.try_wait()) {
            SkAssertResult(this->do_work());
        }
    }

private:
    // This method should be called only when fWorkAvailable indicates there's work to do.
    bool do_work() {
        std::function<void(void)> work;
        {
            SkAutoMutexExclusive lock(fWorkLock);
            SkASSERT(!fWork.empty());        // TODO: if (fWork.empty()) { return true; } ?
            work = std::move(fWork.front());
            fWork.pop_front();
        }

        if (!work) {
            return false;  // This is Loop()'s signal to shut down.
        }

        work();
        return true;
    }

    static void Loop(void* ctx, const ThreadInfo& threadInfo) {
        set_up_context_on_thread(threadInfo);

        auto pool = (GrThreadPool*)ctx;
        do {
            pool->fWorkAvailable.wait();
        } while (pool->do_work());
    }

    using WorkList = std::deque<std::function<void(void)>>;

    SkTArray<std::thread> fThreads;
    WorkList              fWork;
    SkMutex               fWorkLock;
    SkSemaphore           fWorkAvailable;
    bool                  fAllowBorrowing;
};

class GrTaskGroup : SkNoncopyable {
public:
    explicit GrTaskGroup(GrThreadPool& executor) : fPending(0), fExecutor(executor) {}
    ~GrTaskGroup() { this->wait(); }

    // Add a task to this SkTaskGroup.
    void add(std::function<void(void)> fn) {
        fPending.fetch_add(+1, std::memory_order_relaxed);
        fExecutor.add([this, fn{std::move(fn)}] {
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
        // Actively help the executor do work until our task group is done.
        // This lets SkTaskGroups nest arbitrarily deep on a single SkExecutor:
        // no thread ever blocks waiting for others to do its work.
        // (We may end up doing work that's not part of our task group.  That's fine.)
        while (!this->done()) {
            fExecutor.borrow();
        }
    }

private:
    std::atomic<int32_t> fPending;
    GrThreadPool&        fExecutor;
};

static bool create_contexts(GrContextFactory* factory,
                            GrContextFactory::ContextType contextType,
                            const GrContextFactory::ContextOverrides& overrides,
                            std::vector<ThreadInfo>* mainContext,
                            int numUtilityContexts,
                            std::vector<ThreadInfo>* utilityContexts) {

    ContextInfo mainInfo = factory->getContextInfo(contextType, overrides);
    if (!mainInfo.directContext()) {
        exitf("Could not create primary direct context.");
    }

    mainContext->push_back({ "g0", mainInfo.directContext(), mainInfo.testContext() });

    bool allSucceeded = true, allFailed = true;
    // Create the utility contexts in a share group with the primary one. This is allowed to fail
    // but either they should all work or the should all fail.
    for (int i = 0; i < numUtilityContexts; ++i) {
        SkString name = SkStringPrintf("r%d", i);

        ContextInfo tmp = factory->getSharedContextInfo(mainInfo.directContext(), i);

        utilityContexts->push_back({ name.c_str(), tmp.directContext(), tmp.testContext() });
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

    std::vector<ThreadInfo> mainContext;
    mainContext.reserve(1);
    std::vector<ThreadInfo> utilityContexts;
    utilityContexts.reserve(FLAGS_ddlNumRecordingThreads);

    if (!create_contexts(&factory,
                         kContextType,
                         kOverrides,
                         &mainContext,
                         FLAGS_ddlNumRecordingThreads,
                         &utilityContexts)) {
        return 1;
    }

    mainContext.front().fTestContext->makeCurrent();

    SkYUVAPixmapInfo::SupportedDataTypes supportedYUVADTypes(*mainContext.front().fDirectContext);
    DDLPromiseImageHelper promiseImageHelper(supportedYUVADTypes);

    sk_sp<SkPicture> skp = create_shared_skp(FLAGS_src[0],
                                             mainContext.front().fDirectContext,
                                             &promiseImageHelper);

    promiseImageHelper.createCallbackContexts(mainContext.front().fDirectContext);

    // TODO: do this later on a utility thread!
    promiseImageHelper.uploadAllToGPU(nullptr, mainContext.front().fDirectContext);

    mainContext.front().fTestContext->makeNotCurrent();

    std::unique_ptr<GrThreadPool> fGPUExecutor(GrThreadPool::MakeFIFOThreadPool(1,
                                                                                mainContext,
                                                                                false));
    std::unique_ptr<GrThreadPool> fRecordingExecutor(GrThreadPool::MakeFIFOThreadPool(
                                                                    FLAGS_ddlNumRecordingThreads,
                                                                    utilityContexts,
                                                                    false));
    GrTaskGroup gpuTaskGroup(*fGPUExecutor);
    GrTaskGroup recordingTaskGroup(*fRecordingExecutor);

    for (int i = 0; i < FLAGS_ddlNumRecordingThreads; ++i) {
        recordingTaskGroup.add([] {
                                   ThreadInfo* threadLocal = get_thread_local_info();
                                   printf("%s: dContext %p\n", threadLocal->fName,
                                                               threadLocal->fDirectContext);
                                   std::this_thread::sleep_for(std::chrono::seconds(1));
                               });
    }

    gpuTaskGroup.add([] {
                         ThreadInfo* threadLocal = get_thread_local_info();
                         printf("%s: dContext %p\n", threadLocal->fName,
                                                     threadLocal->fDirectContext);
                     });

    gpuTaskGroup.add([testCtx = mainContext.front().fTestContext] {
                         ThreadInfo* threadLocal = get_thread_local_info();
                         threadLocal->fTestContext->makeNotCurrent();
                     });

    recordingTaskGroup.wait();
    gpuTaskGroup.wait();

    mainContext.front().fTestContext->makeCurrent();

    return 0;
}
