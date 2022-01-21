/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "fuzz/Fuzz.h"
#include "fuzz/FuzzCommon.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkDeferredDisplayList.h"
#include "include/core/SkDeferredDisplayListRecorder.h"
#include "include/core/SkExecutor.h"
#include "include/core/SkPromiseImageTexture.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/private/SkDeque.h"
#include "include/private/SkMutex.h"
#include "include/private/SkNoncopyable.h"
#include "include/private/SkTemplates.h"
#include "include/private/SkThreadID.h"
#include "src/core/SkTaskGroup.h"
#include "src/image/SkImage_Gpu.h"
#include "tools/gpu/GrContextFactory.h"

#include <atomic>
#include <memory>
#include <queue>

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
class PromiseImageInfo : public SkNVRefCnt<PromiseImageInfo>, SkNoncopyable {
public:
    enum class State : int {
        kInitial,
        kTriedToFulfill,
        kDone
    };
    ~PromiseImageInfo() {
        // If we hit this, then the image or the texture will outlive this object which is bad.
        SkASSERT_RELEASE(!fImage || fImage->unique());
        SkASSERT_RELEASE(!fTexture || fTexture->unique());
        fImage.reset();
        fTexture.reset();
        State s = fState;
        SkASSERT_RELEASE(s == State::kDone);
    }
    DDLFuzzer* fFuzzer = nullptr;
    sk_sp<SkImage> fImage;
    // At the moment, the atomicity of this isn't used because all our promise image callbacks
    // happen on the same thread. See the TODO below about them unreffing them off the GPU thread.
    std::atomic<State> fState{State::kInitial};
    sk_sp<SkPromiseImageTexture> fTexture;
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
class DDLFuzzer : SkNoncopyable {
public:
    DDLFuzzer(Fuzz*, ContextType);
    void run();

    sk_sp<SkPromiseImageTexture> fulfillPromiseImage(PromiseImageInfo&);
    void releasePromiseImage(PromiseImageInfo&);
private:
    void initPromiseImage(int index);
    void recordAndPlayDDL();
    bool isOnGPUThread() const { return SkGetThreadID() == fGpuThread; }
    bool isOnMainThread() const { return SkGetThreadID() == fMainThread; }

    Fuzz* fFuzz = nullptr;
    GrDirectContext* fContext = nullptr;
    SkAutoTArray<PromiseImageInfo> fPromiseImages{kPromiseImageCount};
    sk_sp<SkSurface> fSurface;
    SkSurfaceCharacterization fSurfaceCharacterization;
    std::unique_ptr<SkExecutor> fGpuExecutor = SkExecutor::MakeFIFOThreadPool(1, false);
    std::unique_ptr<SkExecutor> fRecordingExecutor =
        SkExecutor::MakeFIFOThreadPool(kRecordingThreadCount, false);
    SkTaskGroup fGpuTaskGroup{*fGpuExecutor};
    SkTaskGroup fRecordingTaskGroup{*fRecordingExecutor};
    SkThreadID fGpuThread = kIllegalThreadID;
    SkThreadID fMainThread = SkGetThreadID();
    std::queue<sk_sp<SkPromiseImageTexture>> fReusableTextures;
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
    fSurface = SkSurface::MakeRenderTarget(fContext, SkBudgeted::kNo, ii);
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

sk_sp<SkPromiseImageTexture> DDLFuzzer::fulfillPromiseImage(PromiseImageInfo& promiseImage) {
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

    GrBackendTexture backendTex = fContext->createBackendTexture(kPromiseImageSize.width(),
                                                                 kPromiseImageSize.height(),
                                                                 kRGBA_8888_SkColorType,
                                                                 SkColors::kRed,
                                                                 GrMipMapped::kNo,
                                                                 GrRenderable::kYes,
                                                                 GrProtected::kNo,
                                                                 markFinished,
                                                                 &finishedBECreate);
    SkASSERT_RELEASE(backendTex.isValid());
    while (!finishedBECreate) {
        fContext->checkAsyncWorkCompletion();
    }

    promiseImage.fTexture = SkPromiseImageTexture::Make(backendTex);

    return promiseImage.fTexture;
}

void DDLFuzzer::releasePromiseImage(PromiseImageInfo& promiseImage) {
    using State = PromiseImageInfo::State;
    // TODO: This requirement will go away when we unref promise images off the GPU thread.
    if (!this->isOnGPUThread()) {
        fFuzz->signalBug();
    }
    State old = promiseImage.fState.exchange(State::kInitial, std::memory_order_relaxed);
    if (old != State::kTriedToFulfill) {
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

static sk_sp<SkPromiseImageTexture> fuzz_promise_image_fulfill(void* ctxIn) {
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
    promiseImage.fImage = SkImage::MakePromiseTexture(fContext->threadSafeProxy(),
                                                      backendFmt,
                                                      kPromiseImageSize,
                                                      GrMipMapped::kNo,
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
    SkDeferredDisplayListRecorder recorder(fSurfaceCharacterization);
    SkCanvas* canvas = recorder.getCanvas();
    // Draw promise images in a strip
    for (int i = 0; i < kPromiseImagesPerDDL; i++) {
        int xOffset = i * kPromiseImageSize.width();
        int j;
        // Pick random promise images to draw.
        fFuzz->nextRange(&j, 0, kPromiseImageCount - 1);
        canvas->drawImage(fPromiseImages[j].fImage, xOffset, 0);
    }
    sk_sp<SkDeferredDisplayList> ddl = recorder.detach();
    fGpuTaskGroup.add([=, ddl{std::move(ddl)}]{
        bool success = fSurface->draw(std::move(ddl));
        if (!success) {
            fFuzz->signalBug();
        }
    });
}

void DDLFuzzer::run() {
    if (!fSurface) {
        return;
    }
    fRecordingTaskGroup.batch(kIterationCount, [=](int i) {
        this->recordAndPlayDDL();
    });
    fRecordingTaskGroup.wait();
    fGpuTaskGroup.add([=] {
        while (!fReusableTextures.empty()) {
            sk_sp<SkPromiseImageTexture> gpuTexture = std::move(fReusableTextures.front());
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
    DDLFuzzer(fuzz, ContextType::kGL_ContextType).run();
}
