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

#if 1 /* SKSL_USE_THREAD_LOCAL */

static thread_local GrDirectContext* sDirectContext = nullptr;

static GrDirectContext* get_thread_local_context() {
    return sDirectContext;
}

static void set_thread_local_context(GrDirectContext* dContext) {
    sDirectContext = dContext;
}

#else
// TODO: handle
#endif

static void set_up_context(GrDirectContext* dContext, TestContext* testContext) {
    if (dContext) {
        testContext->makeCurrent();
        set_thread_local_context(dContext);
    }
}

class SK_API SkExecutor2 {
public:
    virtual ~SkExecutor2() {}

    // Create a thread pool SkExecutor with a fixed thread count, by default the number of cores.
    static std::unique_ptr<SkExecutor2> MakeFIFOThreadPool(int threads,
                                                           std::vector<ContextInfo>& contexts,
                                                           bool allowBorrowing = true);
    static std::unique_ptr<SkExecutor2> MakeLIFOThreadPool(int threads,
                                                           std::vector<ContextInfo>& contexts,
                                                           bool allowBorrowing = true);

    // Add work to execute.
    virtual void add(std::function<void(void)>) = 0;

    // If it makes sense for this executor, use this thread to execute work for a little while.
    virtual void borrow() {}
};

// We'll always push_back() new work, but pop from the front of deques or the back of SkTArray.
static inline std::function<void(void)> pop2(std::deque<std::function<void(void)>>* list) {
    std::function<void(void)> fn = std::move(list->front());
    list->pop_front();
    return fn;
}
static inline std::function<void(void)> pop2(SkTArray<std::function<void(void)>>* list) {
    std::function<void(void)> fn = std::move(list->back());
    list->pop_back();
    return fn;
}

// An SkThreadPool is an executor that runs work on a fixed pool of OS threads.
template <typename WorkList>
class SkThreadPool2 final : public SkExecutor2 {
public:
    explicit SkThreadPool2(int threads, std::vector<ContextInfo>& contexts, bool allowBorrowing)
            : fAllowBorrowing(allowBorrowing) {
        SkASSERT(((int)contexts.size()) >= threads);

        for (int i = 0; i < threads; i++) {
            printf("handing out %p\n", contexts[i].directContext());
            fThreads.emplace_back(&Loop, this, contexts[i]);
        }
    }

    ~SkThreadPool2() override {
        // Signal each thread that it's time to shut down.
        for (int i = 0; i < fThreads.count(); i++) {
            this->add(nullptr);
        }
        // Wait for each thread to shut down.
        for (int i = 0; i < fThreads.count(); i++) {
            fThreads[i].join();
        }
    }

    void add(std::function<void(void)> work) override {
        // Add some work to our pile of work to do.
        {
            SkAutoMutexExclusive lock(fWorkLock);
            fWork.emplace_back(std::move(work));
        }
        // Tell the Loop() threads to pick it up.
        fWorkAvailable.signal(1);
    }

    void borrow() override {
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
            work = pop2(&fWork);
        }

        if (!work) {
            return false;  // This is Loop()'s signal to shut down.
        }

        work();
        return true;
    }

    static void Loop(void* ctx, const ContextInfo& contextInfo) {
        set_up_context(contextInfo.directContext(), contextInfo.testContext());

        auto pool = (SkThreadPool2*)ctx;
        do {
            pool->fWorkAvailable.wait();
        } while (pool->do_work());
    }

    // Both SkMutex and SkSpinlock can work here.
    using Lock = SkMutex;

    SkTArray<std::thread> fThreads;
    WorkList              fWork;
    Lock                  fWorkLock;
    SkSemaphore           fWorkAvailable;
    bool                  fAllowBorrowing;
};

std::unique_ptr<SkExecutor2> SkExecutor2::MakeFIFOThreadPool(int threads,
                                                             std::vector<ContextInfo>& contexts,
                                                             bool allowBorrowing) {
    using WorkList = std::deque<std::function<void(void)>>;
    SkASSERT(threads > 0);
    return std::make_unique<SkThreadPool2<WorkList>>(threads, contexts, allowBorrowing);
}
std::unique_ptr<SkExecutor2> SkExecutor2::MakeLIFOThreadPool(int threads,
                                                             std::vector<ContextInfo>& contexts,
                                                             bool allowBorrowing) {
    using WorkList = SkTArray<std::function<void(void)>>;
    SkASSERT(threads > 0);
    return std::make_unique<SkThreadPool2<WorkList>>(threads, contexts, allowBorrowing);
}

class SkTaskGroup2 : SkNoncopyable {
public:
    // Tasks added to this SkTaskGroup will run on its executor.
    explicit SkTaskGroup2(SkExecutor2& executor) : fPending(0), fExecutor(executor) {}
    ~SkTaskGroup2() { this->wait(); }

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
    SkExecutor2&          fExecutor;
};

static bool create_contexts(GrContextFactory* factory,
                            GrContextFactory::ContextType contextType,
                            const GrContextFactory::ContextOverrides& overrides,
                            std::vector<ContextInfo>* mainContext,
                            int numUtilityContexts,
                            std::vector<ContextInfo>* utilityContexts) {

    mainContext->push_back(factory->getContextInfo(contextType, overrides));
    if (!mainContext->back().directContext()) {
        exitf("Could not create primary direct context.");
    }

    bool allSucceeded = true, allFailed = true;
    // Create the utility contexts in a share group with the primary one. This is allowed to fail
    // but either they should all work or the should all fail.
    for (int i = 0; i < numUtilityContexts; ++i) {
        utilityContexts->push_back(factory->getSharedContextInfo(mainContext->back().directContext(), i));
        allSucceeded &= SkToBool(utilityContexts->back().directContext());
        allFailed &= !utilityContexts->back().directContext();
    }

    return allSucceeded || allFailed;
}

static sk_sp<SkPicture> prep_data(const char* src,
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

    std::vector<ContextInfo> mainContext;
    mainContext.reserve(1);
    std::vector<ContextInfo> utilityContexts;
    utilityContexts.reserve(FLAGS_ddlNumRecordingThreads+1);

    if (!create_contexts(&factory,
                         kContextType,
                         kOverrides,
                         &mainContext,
                         FLAGS_ddlNumRecordingThreads+1,
                         &utilityContexts)) {
        return 1;
    }

    mainContext.front().testContext()->makeCurrent();

    SkYUVAPixmapInfo::SupportedDataTypes supportedYUVADataTypes(*mainContext.front().directContext());
    DDLPromiseImageHelper promiseImageHelper(supportedYUVADataTypes);

    sk_sp<SkPicture> skp = prep_data(FLAGS_src[0],
                                     mainContext.front().directContext(),
                                     &promiseImageHelper);

    promiseImageHelper.createCallbackContexts(mainContext.front().directContext());

    // TODO: do this later on a thread!
    promiseImageHelper.uploadAllToGPU(nullptr, mainContext.front().directContext());

    std::unique_ptr<SkExecutor2> fGPUExecutor(SkExecutor2::MakeFIFOThreadPool(1,
                                                                              mainContext,
                                                                              false));
    std::unique_ptr<SkExecutor2> fRecordingExecutor(SkExecutor2::MakeLIFOThreadPool(FLAGS_ddlNumRecordingThreads,
                                                                                    utilityContexts,
                                                                                    false));
    SkTaskGroup2 gpuTaskGroup(*fGPUExecutor);
    SkTaskGroup2 recordingTaskGroup(*fRecordingExecutor);

    set_up_context(utilityContexts.back().directContext(),
                   utilityContexts.back().testContext());

#if 1
    recordingTaskGroup.add([] {
                        GrDirectContext* bar = get_thread_local_context();
                        printf("thread: hello world %p\n", bar);
                      });

    gpuTaskGroup.add([] {
                        GrDirectContext* bar = get_thread_local_context();
                        printf("gpu: hello world %p\n", bar);
                      });

    recordingTaskGroup.wait();
    gpuTaskGroup.wait();

    GrDirectContext* foo = get_thread_local_context();

    printf("hello world %d %p\n", FLAGS_ddlNumRecordingThreads, foo);
#endif


    return 0;
}

