/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#if defined(SK_GRAPHITE)

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkTextBlob.h"
#include "include/effects/SkGradientShader.h"
#include "include/gpu/graphite/PrecompileContext.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/precompile/PaintOptions.h"
#include "include/gpu/graphite/precompile/Precompile.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/graphite/UniqueKeyUtils.h"

#include <thread>

using namespace::skgpu::graphite;

namespace {

static constexpr int kMaxNumStops = 9;
static constexpr SkColor gColors[kMaxNumStops] = {
        SK_ColorRED,
        SK_ColorGREEN,
        SK_ColorBLUE,
        SK_ColorCYAN,
        SK_ColorMAGENTA,
        SK_ColorYELLOW,
        SK_ColorBLACK,
        SK_ColorDKGRAY,
        SK_ColorLTGRAY,
};
static constexpr SkPoint gPts[kMaxNumStops] = {
        { -100.0f, -100.0f },
        { -50.0f, -50.0f },
        { -25.0f, -25.0f },
        { -12.5f, -12.5f },
        { 0.0f, 0.0f },
        { 12.5f, 12.5f },
        { 25.0f, 25.0f },
        { 50.0f, 50.0f },
        { 100.0f, 100.0f }
};
static constexpr float gOffsets[kMaxNumStops] =
            { 0.0f, 0.125f, 0.25f, 0.375f, 0.5f, 0.625f, 0.75f, 0.875f, 1.0f };

std::pair<SkPaint, PaintOptions> linear(int numStops) {
    SkASSERT(numStops <= kMaxNumStops);

    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::LinearGradient() });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });

    SkPaint paint;
    paint.setShader(SkGradientShader::MakeLinear(gPts,
                                                 gColors, gOffsets, numStops,
                                                 SkTileMode::kClamp));
    paint.setBlendMode(SkBlendMode::kSrcOver);

    return { paint, paintOptions };
}

std::pair<SkPaint, PaintOptions> radial(int numStops) {
    SkASSERT(numStops <= kMaxNumStops);

    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::RadialGradient() });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });

    SkPaint paint;
    paint.setShader(SkGradientShader::MakeRadial(/* center= */ {0, 0}, /* radius= */ 100,
                                                 gColors, gOffsets, numStops,
                                                 SkTileMode::kClamp));
    paint.setBlendMode(SkBlendMode::kSrcOver);

    return { paint, paintOptions };
}

std::pair<SkPaint, PaintOptions> sweep(int numStops) {
    SkASSERT(numStops <= kMaxNumStops);

    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::SweepGradient() });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });

    SkPaint paint;
    paint.setShader(SkGradientShader::MakeSweep(/* cx= */ 0, /* cy= */ 0,
                                                gColors, gOffsets, numStops,
                                                SkTileMode::kClamp,
                                                /* startAngle= */ 0, /* endAngle= */ 359,
                                                /* flags= */ 0, /* localMatrix= */ nullptr));
    paint.setBlendMode(SkBlendMode::kSrcOver);

    return { paint, paintOptions };
}

std::pair<SkPaint, PaintOptions> conical(int numStops) {
    SkASSERT(numStops <= kMaxNumStops);

    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::TwoPointConicalGradient() });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });

    SkPaint paint;
    paint.setShader(SkGradientShader::MakeTwoPointConical(/* start= */ {100, 100},
                                                          /* startRadius= */ 100,
                                                          /* end= */ {-100, -100},
                                                          /* endRadius= */ 100,
                                                          gColors, gOffsets, numStops,
                                                          SkTileMode::kClamp));
    paint.setBlendMode(SkBlendMode::kSrcOver);

    return { paint, paintOptions };
}

void precompile_gradients(std::unique_ptr<PrecompileContext> precompileContext,
                          skiatest::Reporter* /* reporter */,
                          int /* threadID */) {
    constexpr RenderPassProperties kProps = { DepthStencilFlags::kDepth,
                                              kBGRA_8888_SkColorType,
                                              /* requiresMSAA= */ false };

    for (auto createOptionsMtd : { linear, radial, sweep, conical }) {
        // numStops doesn't influence the paintOptions
        auto [_, paintOptions] = createOptionsMtd(/* numStops= */ 2);
        Precompile(precompileContext.get(),
                   paintOptions,
                   DrawTypeFlags::kBitmapText_Mask,
                   { &kProps, 1 });
    }

    precompileContext.reset();
}

// A simple helper to call Context::insertRecording on the Recordings generated on the
// recorder threads. It collects (and keeps ownership) of all the generated Recordings.
class Listener : public SkRefCnt {
public:
    Listener(int numSenders) : fNumActiveSenders(numSenders) {}

    void addRecording(std::unique_ptr<Recording> recording) SK_EXCLUDES(fLock) {
        {
            SkAutoMutexExclusive lock(fLock);
            fRecordings.push_back(std::move(recording));
        }

        fWorkAvailable.signal(1);
    }

    void deregister() SK_EXCLUDES(fLock) {
        {
            SkAutoMutexExclusive lock(fLock);
            fNumActiveSenders--;
        }

        fWorkAvailable.signal(1);
    }

    void insertRecordings(Context* context) {
        do {
            fWorkAvailable.wait();
        } while (this->insertRecording(context));
    }

private:
    // This entry point is run in a loop waiting on the 'fWorkAvailable' semaphore until there
    // are no senders remaining (at which point it returns false) c.f. 'insertRecordings'.
    bool insertRecording(Context* context) SK_EXCLUDES(fLock) {
        Recording* recording = nullptr;
        int numSendersLeft;

        {
            SkAutoMutexExclusive lock(fLock);

            numSendersLeft = fNumActiveSenders;

            SkASSERT(fRecordings.size() >= fCurHandled);
            if (fRecordings.size() > fCurHandled) {
                recording = fRecordings[fCurHandled++].get();
            }
        }

        if (recording) {
            context->insertRecording({recording});
            return true;  // continue looping
        }

        return SkToBool(numSendersLeft); // continue looping if there are still active senders
    }

    SkMutex fLock;
    SkSemaphore fWorkAvailable;

    skia_private::TArray<std::unique_ptr<Recording>> fRecordings SK_GUARDED_BY(fLock);
    int fCurHandled SK_GUARDED_BY(fLock) = 0;
    int fNumActiveSenders SK_GUARDED_BY(fLock);
};

void compile_gradients(std::unique_ptr<Recorder> recorder,
                       sk_sp<Listener> listener,
                       skiatest::Reporter* /* reporter */,
                       int /* threadID */) {
    SkFont font(ToolUtils::DefaultPortableTypeface(), /* size= */ 16);

    const char text[] = "hambur1";
    sk_sp<SkTextBlob> blob = SkTextBlob::MakeFromText(text, strlen(text), font);

    SkImageInfo ii = SkImageInfo::Make(16, 16,
                                       kBGRA_8888_SkColorType,
                                       kPremul_SkAlphaType);

    sk_sp<SkSurface> surf = SkSurfaces::RenderTarget(recorder.get(), ii,
                                                     skgpu::Mipmapped::kNo,
                                                     /* surfaceProps= */ nullptr);
    SkCanvas* canvas = surf->getCanvas();

    for (auto createOptionsMtd : { linear, radial, sweep, conical }) {
        for (int numStops : { 2, 7, kMaxNumStops }) {
            auto [paint, _] = createOptionsMtd(numStops);

            canvas->drawTextBlob(blob, 0, 16, paint);

            // This will trigger pipeline creation via TaskList::prepareResources
            std::unique_ptr<skgpu::graphite::Recording> recording = recorder->snap();

            listener->addRecording(std::move(recording));
        }
    }

    listener->deregister();
}

} // anonymous namespace

// This test precompiles all four flavors of gradient sequentially but on multiple
// threads with the goal of creating cache races.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(ThreadedPrecompileTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kNever) {
    constexpr int kNumThreads = 4;


    std::thread threads[kNumThreads];
    for (int i = 0; i < kNumThreads; ++i) {
        std::unique_ptr<PrecompileContext> precompileContext = context->makePrecompileContext();

        threads[i] = std::thread(precompile_gradients, std::move(precompileContext), reporter, i);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    const GlobalCache::PipelineStats stats = context->priv().globalCache()->getStats();

    // Four types of gradient times three combinations (i.e., 4,8,N) for each one.
    REPORTER_ASSERT(reporter, stats.fGraphicsCacheAdditions == 12);
    REPORTER_ASSERT(reporter, stats.fGraphicsRaces > 0);
    REPORTER_ASSERT(reporter, stats.fGraphicsCacheMisses ==
                              stats.fGraphicsCacheAdditions + stats.fGraphicsRaces);
}

// This test runs two threads compiling the gradient flavours and two threads
// pre-compiling the gradient flavors. This is to exercise the tracking of the
// various race combinations (i.e., Normal vs Precompile, Normal vs. Normal, etc.).
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(ThreadedCompilePrecompileTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kNever) {

    constexpr int kNumRecordingThreads = 2;
    constexpr int kNumPrecompileThreads = 2;
    constexpr int kTotNumThreads = kNumRecordingThreads + kNumPrecompileThreads;

    sk_sp<Listener> listener = sk_make_sp<Listener>(kNumRecordingThreads);

    std::thread threads[kTotNumThreads];

    for (int i = 0; i < kNumRecordingThreads; ++i) {
        std::unique_ptr<Recorder> recorder = context->makeRecorder();

        threads[i] = std::thread(compile_gradients,
                                 std::move(recorder),
                                 listener,
                                 reporter,
                                 i);
    }
    for (int i = 0; i < kNumPrecompileThreads; ++i) {
        std::unique_ptr<PrecompileContext> precompileContext = context->makePrecompileContext();

        int threadID = kNumRecordingThreads+i;
        threads[threadID] = std::thread(precompile_gradients,
                                        std::move(precompileContext),
                                        reporter,
                                        threadID);
    }

    // Process the work generated by the recording threads
    listener->insertRecordings(context);

    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    context->submit(SyncToCpu::kYes);

    const GlobalCache::PipelineStats stats = context->priv().globalCache()->getStats();

    // Four types of gradient times three combinations (i.e., 4,8,N) for each one.
    REPORTER_ASSERT(reporter, stats.fGraphicsCacheAdditions == 12);
    REPORTER_ASSERT(reporter, stats.fGraphicsRaces > 0);
    REPORTER_ASSERT(reporter, stats.fGraphicsCacheMisses ==
                              stats.fGraphicsCacheAdditions + stats.fGraphicsRaces);

    // The 48 comes from:
    //     4 gradient flavors (linear, radial, ...) *
    //     3 types of each flavor (4, 8, N) *
    //     4 threads (2 normal-compile + 2 pre-compile)
    REPORTER_ASSERT(reporter, stats.fGraphicsCacheHits + stats.fGraphicsCacheMisses == 48);
}

#endif // SK_GRAPHITE
