/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "fuzz/Fuzz.h"
#include "fuzz/FuzzCommon.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkExecutor.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/base/SkDeque.h"
#include "include/private/base/SkMutex.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkThreadID.h"
#include "include/private/chromium/GrDeferredDisplayList.h"
#include "include/private/chromium/GrDeferredDisplayListRecorder.h"
#include "include/private/chromium/GrPromiseImageTexture.h"
#include "include/private/chromium/SkImageChromium.h"
#include "src/core/SkTaskGroup.h"
#include "src/gpu/ganesh/image/SkImage_Ganesh.h"
#include "tools/gpu/GrContextFactory.h"

#include <atomic>
#include <memory>
#include <queue>

using namespace skia_private;
using ContextType = sk_gpu_test::GrContextFactory::ContextType;

// be careful: `foo(make_fuzz_t<T>(f), make_fuzz_t<U>(f))` is undefined.
// In fact, all make_fuzz_foo() functions have this potential problem.
// Use sequence points!
template <typename T>
inline T make_fuzz_t(Fuzz* fuzz) {
    T t;
    fuzz->next(&t);
    return t;
}

class DDLFuzzer;

// This class stores the state of a given promise image owned by the fuzzer. It acts as the
// context for the callback procs of the promise image.
class PromiseImageInfo : public SkNVRefCnt<PromiseImageInfo> {
public:
    enum class State : int {
        kInitial,
        kTriedToFulfill,
        kDone
    };

    PromiseImageInfo() = default;
    ~PromiseImageInfo() {
        // If we hit this, then the image or the texture will outlive this object which is bad.
        SkASSERT_RELEASE(!fImage || fImage->unique());
        SkASSERT_RELEASE(!fTexture || fTexture->unique());
        fImage.reset();
        fTexture.reset();
        State s = fState;
        SkASSERT_RELEASE(!fDrawn || s == State::kDone);
    }

    // Make noncopyable
    PromiseImageInfo(PromiseImageInfo&) = delete;
    PromiseImageInfo& operator=(PromiseImageInfo&) = delete;

    DDLFuzzer* fFuzzer = nullptr;
    sk_sp<SkImage> fImage;
    // At the moment, the atomicity of this isn't used because all our promise image callbacks
    // happen on the same thread. See the TODO below about them unreffing them off the GPU thread.
    std::atomic<State> fState{State::kInitial};
    std::atomic<bool> fDrawn{false};

    sk_sp<GrPromiseImageTexture> fTexture;
};

static constexpr int kPromiseImageCount = 8;
static constexpr SkISize kPromiseImageSize{16, 16};
static constexpr int kPromiseImagesPerDDL = 4;
static constexpr int kRecordingThreadCount = 4;
static constexpr int kIterationCount = 10000;

// A one-shot runner object for fuzzing our DDL threading. It creates an array of promise images,
// and concurrently records DDLs that reference them, playing each DDL back on the GPU thread.
// The backing textures for promise images may be recycled into a pool, or not, for each case
// as determined by the fuzzing data.
class DDLFuzzer {
public:
    DDLFuzzer(Fuzz*, ContextType);
    DDLFuzzer() = delete;
    // Make noncopyable
    DDLFuzzer(DDLFuzzer&) = delete;
    DDLFuzzer& operator=(DDLFuzzer&) = delete;

    void run();

    sk_sp<GrPromiseImageTexture> fulfillPromiseImage(PromiseImageInfo&);
    void releasePromiseImage(PromiseImageInfo&);
private:
    void initPromiseImage(int index);
    void recordAndPlayDDL();
    bool isOnGPUThread() const { return SkGetThreadID() == fGpuThread; }
    bool isOnMainThread() const { return SkGetThreadID() == fMainThread; }

    Fuzz* fFuzz = nullptr;
    GrDirectContext* fContext = nullptr;
    AutoTArray<PromiseImageInfo> fPromiseImages{kPromiseImageCount};
    sk_sp<SkSurface> fSurface;
    GrSurfaceCharacterization fSurfaceCharacterization;
    std::unique_ptr<SkExecutor> fGpuExecutor = SkExecutor::MakeFIFOThreadPool(1, false);
    std::unique_ptr<SkExecutor> fRecordingExecutor =
        SkExecutor::MakeFIFOThreadPool(kRecordingThreadCount, false);
    SkTaskGroup fGpuTaskGroup{*fGpuExecutor};
    SkTaskGroup fRecordingTaskGroup{*fRecordingExecutor};
    SkThreadID fGpuThread = kIllegalThreadID;
    SkThreadID fMainThread = SkGetThreadID();
    std::queue<sk_sp<GrPromiseImageTexture>> fReusableTextures;
    sk_gpu_test::GrContextFactory fContextFactory;
};

DDLFuzzer::DDLFuzzer(Fuzz* fuzz, ContextType contextType) : fFuzz(fuzz) {
    sk_gpu_test::ContextInfo ctxInfo = fContextFactory.getContextInfo(contextType);
    sk_gpu_test::TestContext* testCtx = ctxInfo.testContext();
    fContext = ctxInfo.directContext();
    if (!fContext) {
        return;
    }
    SkISize canvasSize = kPromiseImageSize;
    canvasSize.fWidth *= kPromiseImagesPerDDL;
    SkImageInfo ii = SkImageInfo::Make(canvasSize, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    fSurface = SkSurfaces::RenderTarget(fContext, skgpu::Budgeted::kNo, ii);
    if (!fSurface || !fSurface->characterize(&fSurfaceCharacterization)) {
        return;
    }

    testCtx->makeNotCurrent();
    fGpuTaskGroup.add([&]{
        testCtx->makeCurrent();
        fGpuThread = SkGetThreadID();
    });
    fGpuTaskGroup.wait();
    for (int i = 0; i < kPromiseImageCount; ++i) {
        this->initPromiseImage(i);
    }
}

sk_sp<GrPromiseImageTexture> DDLFuzzer::fulfillPromiseImage(PromiseImageInfo& promiseImage) {
    using State = PromiseImageInfo::State;
    if (!this->isOnGPUThread()) {
        fFuzz->signalBug();
    }
    bool success = make_fuzz_t<bool>(fFuzz);
    State prior = promiseImage.fState.exchange(State::kTriedToFulfill, std::memory_order_relaxed);
    if (prior != State::kInitial || promiseImage.fTexture != nullptr) {
        fFuzz->signalBug();
    }
    if (!success) {
        return nullptr;
    }

    // Try reusing an existing texture if we can and if the fuzzer wills it.
    if (!fReusableTextures.empty() && make_fuzz_t<bool>(fFuzz)) {
        promiseImage.fTexture = std::move(fReusableTextures.front());
        fReusableTextures.pop();
        return promiseImage.fTexture;
    }

    bool finishedBECreate = false;
    auto markFinished = [](void* context) {
        *(bool*)context = true;
    };

    GrBackendTexture backendTex =
            fContext->createBackendTexture(kPromiseImageSize.width(),
                                           kPromiseImageSize.height(),
                                           kRGBA_8888_SkColorType,
                                           SkColors::kRed,
                                           skgpu::Mipmapped::kNo,
                                           GrRenderable::kYes,
                                           GrProtected::kNo,
                                           markFinished,
                                           &finishedBECreate,
                                           /*label=*/"DDLFuzzer_FulFillPromiseImage");
    SkASSERT_RELEASE(backendTex.isValid());
    while (!finishedBECreate) {
        fContext->checkAsyncWorkCompletion();
    }

    promiseImage.fTexture = GrPromiseImageTexture::Make(backendTex);

    return promiseImage.fTexture;
}

void DDLFuzzer::releasePromiseImage(PromiseImageInfo& promiseImage) {
    using State = PromiseImageInfo::State;
    // TODO: This requirement will go away when we unref promise images off the GPU thread.
    if (!this->isOnGPUThread()) {
        fFuzz->signalBug();
    }

    State old = promiseImage.fState.exchange(State::kDone, std::memory_order_relaxed);
    if (promiseImage.fDrawn && old != State::kTriedToFulfill) {
        fFuzz->signalBug();
    }

    // If we failed to fulfill, then nothing to be done.
    if (!promiseImage.fTexture) {
        return;
    }

    bool reuse = make_fuzz_t<bool>(fFuzz);
    if (reuse) {
        fReusableTextures.push(std::move(promiseImage.fTexture));
    } else {
        fContext->deleteBackendTexture(promiseImage.fTexture->backendTexture());
    }
    promiseImage.fTexture = nullptr;
}

static sk_sp<GrPromiseImageTexture> fuzz_promise_image_fulfill(void* ctxIn) {
    PromiseImageInfo& fuzzPromiseImage = *(PromiseImageInfo*)ctxIn;
    return fuzzPromiseImage.fFuzzer->fulfillPromiseImage(fuzzPromiseImage);
}

static void fuzz_promise_image_release(void* ctxIn) {
    PromiseImageInfo& fuzzPromiseImage = *(PromiseImageInfo*)ctxIn;
    fuzzPromiseImage.fFuzzer->releasePromiseImage(fuzzPromiseImage);
}

void DDLFuzzer::initPromiseImage(int index) {
    PromiseImageInfo& promiseImage = fPromiseImages[index];
    promiseImage.fFuzzer = this;
    GrBackendFormat backendFmt = fContext->defaultBackendFormat(kRGBA_8888_SkColorType,
                                                                GrRenderable::kYes);
    promiseImage.fImage = SkImages::PromiseTextureFrom(fContext->threadSafeProxy(),
                                                       backendFmt,
                                                       kPromiseImageSize,
                                                       skgpu::Mipmapped::kNo,
                                                       kTopLeft_GrSurfaceOrigin,
                                                       kRGBA_8888_SkColorType,
                                                       kUnpremul_SkAlphaType,
                                                       SkColorSpace::MakeSRGB(),
                                                       &fuzz_promise_image_fulfill,
                                                       &fuzz_promise_image_release,
                                                       &promiseImage);
}

void DDLFuzzer::recordAndPlayDDL() {
    SkASSERT(!this->isOnGPUThread() && !this->isOnMainThread());
    GrDeferredDisplayListRecorder recorder(fSurfaceCharacterization);
    SkCanvas* canvas = recorder.getCanvas();
    // Draw promise images in a strip
    for (int i = 0; i < kPromiseImagesPerDDL; i++) {
        int xOffset = i * kPromiseImageSize.width();
        int j;
        // Pick random promise images to draw.
        fFuzz->nextRange(&j, 0, kPromiseImageCount - 1);
        fPromiseImages[j].fDrawn = true;
        canvas->drawImage(fPromiseImages[j].fImage, xOffset, 0);
    }
    sk_sp<GrDeferredDisplayList> ddl = recorder.detach();
    fGpuTaskGroup.add([ddl{std::move(ddl)}, this] {
        bool success = skgpu::ganesh::DrawDDL(fSurface, std::move(ddl));
        if (!success) {
            fFuzz->signalBug();
        }
    });
}

void DDLFuzzer::run() {
    if (!fSurface) {
        return;
    }
    fRecordingTaskGroup.batch(kIterationCount, [this](int i) { this->recordAndPlayDDL(); });
    fRecordingTaskGroup.wait();

    fGpuTaskGroup.add([this] { fContext->flushAndSubmit(fSurface.get(), GrSyncCpu::kYes); });

    fGpuTaskGroup.wait();

    fGpuTaskGroup.add([this] {
        while (!fReusableTextures.empty()) {
            sk_sp<GrPromiseImageTexture> gpuTexture = std::move(fReusableTextures.front());
            fContext->deleteBackendTexture(gpuTexture->backendTexture());
            fReusableTextures.pop();
        }
        fContextFactory.destroyContexts();
        // TODO: Release promise images not on the GPU thread.
        fPromiseImages.reset(0);
    });
    fGpuTaskGroup.wait();
}

DEF_FUZZ(DDLThreadingGL, fuzz) {
    DDLFuzzer(fuzz, skgpu::ContextType::kGL).run();
}
