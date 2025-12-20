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
#include "include/effects/SkGradient.h"
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

#include <algorithm>
#include <random>
#include <thread>

using namespace::skgpu::graphite;

namespace {

static constexpr int kMaxNumStops = 9;
static const SkColor4f gColors[kMaxNumStops] = {
    SkColors::kRed, SkColors::kGreen, SkColors::kBlue, SkColors::kCyan,
    SkColors::kMagenta, SkColors::kYellow, SkColors::kBlack,
    SkColor4f::FromColor(SK_ColorDKGRAY), SkColor4f::FromColor(SK_ColorLTGRAY)
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

static SkGradient grad(size_t count) {
    return {{{gColors, count}, {gOffsets, count}, SkTileMode::kClamp}, {}};
}

std::pair<SkPaint, PaintOptions> linear(size_t numStops) {
    SkASSERT(numStops <= kMaxNumStops);

    PaintOptions paintOptions;
    paintOptions.setShaders({{ PrecompileShaders::LinearGradient() }});
    paintOptions.setBlendModes(SKSPAN_INIT_ONE( SkBlendMode::kSrcOver ));

    SkPaint paint;
    paint.setShader(SkShaders::LinearGradient(gPts, grad(numStops)));
    paint.setBlendMode(SkBlendMode::kSrcOver);

    return { paint, paintOptions };
}

std::pair<SkPaint, PaintOptions> radial(size_t numStops) {
    SkASSERT(numStops <= kMaxNumStops);

    PaintOptions paintOptions;
    paintOptions.setShaders({{ PrecompileShaders::RadialGradient() }});
    paintOptions.setBlendModes(SKSPAN_INIT_ONE( SkBlendMode::kSrcOver ));

    SkPaint paint;
    paint.setShader(SkShaders::RadialGradient(/* center= */ {0, 0}, /* radius= */ 100,
                                                 grad(numStops)));
    paint.setBlendMode(SkBlendMode::kSrcOver);

    return { paint, paintOptions };
}

std::pair<SkPaint, PaintOptions> sweep(size_t numStops) {
    SkASSERT(numStops <= kMaxNumStops);

    PaintOptions paintOptions;
    paintOptions.setShaders({{ PrecompileShaders::SweepGradient() }});
    paintOptions.setBlendModes(SKSPAN_INIT_ONE( SkBlendMode::kSrcOver ));

    SkPaint paint;
    paint.setShader(SkShaders::SweepGradient({0, 0}, 0, 359, grad(numStops)));
    paint.setBlendMode(SkBlendMode::kSrcOver);

    return { paint, paintOptions };
}

std::pair<SkPaint, PaintOptions> conical(size_t numStops) {
    SkASSERT(numStops <= kMaxNumStops);

    PaintOptions paintOptions;
    paintOptions.setShaders({{ PrecompileShaders::TwoPointConicalGradient() }});
    paintOptions.setBlendModes(SKSPAN_INIT_ONE( SkBlendMode::kSrcOver ));

    SkPaint paint;
    paint.setShader(SkShaders::TwoPointConicalGradient(/* start= */ {100, 100},
                                                          /* startRadius= */ 100,
                                                          /* end= */ {-100, -100},
                                                          /* endRadius= */ 100,
                                                          grad(numStops)));
    paint.setBlendMode(SkBlendMode::kSrcOver);

    return { paint, paintOptions };
}

// The 12 comes from 4 types of gradient times 3 combinations (i.e., 4,8,N) for each one.
static constexpr int kNumDiffPipelines = 12;
// With the current PipelineManager's behavior we expect two Pipeline Cache searches
// per Pipeline
static constexpr int kNumExpectedCacheSearchesPerPipeline = 2;

typedef std::pair<SkPaint, PaintOptions> (*GradientCreationFunc)(size_t numStops);

struct Combo {
    GradientCreationFunc fCreateOptionsMtd;
    size_t fNumStops;
};

void precompile_gradients(std::unique_ptr<PrecompileContext> precompileContext,
                          bool permute,
                          skiatest::Reporter* /* reporter */,
                          int /* threadID */) {
    std::array<Combo, 4> combos;

    // numStops doesn't influence the paintOptions
    combos[0] = { linear,  /* fNumStops= */ 2 };
    combos[1] = { radial,  /* fNumStops= */ 2 };
    combos[2] = { sweep,   /* fNumStops= */ 2 };
    combos[3] = { conical, /* fNumStops= */ 2 };

    if (permute) {
        std::random_device rd;
        std::mt19937 g(rd());

        std::shuffle(combos.begin(), combos.end(), g);
    }

    const RenderPassProperties kProps = { DepthStencilFlags::kDepth,
                                          kBGRA_8888_SkColorType,
                                          /* dstColorSpace= */ nullptr,
                                          /* requiresMSAA= */ false };

    for (auto c : combos) {
        auto [_, paintOptions] = c.fCreateOptionsMtd(c.fNumStops);
        Precompile(precompileContext.get(),
                   paintOptions,
                   DrawTypeFlags::kBitmapText_Mask,
                   { &kProps, 1 });
    }

    precompileContext.reset();
}

void purge_on_thread(std::unique_ptr<PrecompileContext> precompileContext,
                     std::atomic_bool* keepLooping,
                     skiatest::Reporter* /* reporter */,
                     int /* threadID */) {
    const auto kSleepDuration = std::chrono::milliseconds(1);

    while (*keepLooping) {
        std::this_thread::sleep_for(kSleepDuration);

        precompileContext->purgePipelinesNotUsedInMs(kSleepDuration);
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
                       bool permute,
                       skiatest::Reporter* /* reporter */,
                       int /* threadID */) {
    std::array<Combo, kNumDiffPipelines> combos;

    int i = 0;
    for (auto createOptionsMtd : { linear, radial, sweep, conical }) {
        for (size_t numStops: { 2, 7, kMaxNumStops }) {
            combos[i++] = { createOptionsMtd, numStops };
        }
    }

    if (permute) {
        std::random_device rd;
        std::mt19937 g(rd());

        std::shuffle(combos.begin(), combos.end(), g);
    }

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

    for (auto c : combos) {
        auto [paint, _] = c.fCreateOptionsMtd(c.fNumStops);

        canvas->drawTextBlob(blob, 0, 16, paint);

        // This will trigger pipeline creation via TaskList::prepareResources
        std::unique_ptr<skgpu::graphite::Recording> recording = recorder->snap();

        listener->addRecording(std::move(recording));
    }

    listener->deregister();
}

void run_test(Context* context,
              skiatest::Reporter* reporter,
              int numPurgingThreads,
              int numRecordingThreads,
              int numPrecompileThreads,
              bool permute) {
    const int totNumThreads = numPurgingThreads + numRecordingThreads + numPrecompileThreads;

    sk_sp<Listener> listener;
    if (numRecordingThreads) {
        listener = sk_make_sp<Listener>(numRecordingThreads);
    }

    std::atomic_bool keepPurging = true; // controls the looping in the purging thread(s)

    std::vector<std::thread> threads;
    threads.reserve(totNumThreads);

    int threadID = 0;
    for (int i = 0; i < numPurgingThreads; ++i, ++threadID) {
        std::unique_ptr<PrecompileContext> precompileContext = context->makePrecompileContext();

        threads.push_back(std::thread(purge_on_thread,
                                      std::move(precompileContext),
                                      &keepPurging,
                                      reporter,
                                      threadID));
    }
    for (int i = 0; i < numRecordingThreads; ++i, ++threadID) {
        std::unique_ptr<Recorder> recorder = context->makeRecorder();

        threads.push_back(std::thread(compile_gradients,
                                      std::move(recorder),
                                      listener,
                                      permute,
                                      reporter,
                                      threadID));
    }
    for (int i = 0; i < numPrecompileThreads; ++i, ++threadID) {
        std::unique_ptr<PrecompileContext> precompileContext = context->makePrecompileContext();

        threads.push_back(std::thread(precompile_gradients,
                                      std::move(precompileContext),
                                      permute,
                                      reporter,
                                      threadID));
    }

    // Process the work generated by the recording threads
    if (listener) {
        listener->insertRecordings(context);
    }

    keepPurging = false; // stop the loops in the purging thread(s)

    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    context->submit(SyncToCpu::kYes);
}

[[maybe_unused]] void dump_stats(skgpu::BackendApi api, const GlobalCache::PipelineStats& stats) {
    SkDebugf("%s ------------------------------------------------------------------------------\n"
             "CacheHits: %d\n"
             "CacheMisses: %d\n"
             "CacheAdditions: %d\n"
             "Races: %d\n"
             "Purges: %d\n",
             BackendApiToStr(api),
             stats.fGraphicsCacheHits,
             stats.fGraphicsCacheMisses,
             stats.fGraphicsCacheAdditions,
             stats.fGraphicsRaces,
             stats.fGraphicsPurges);
}

} // anonymous namespace

// This test precompiles all four flavors of gradient sequentially but on multiple
// threads with the goal of creating cache races.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(ThreadedPipelinePrecompileTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kNever) {
    constexpr int kNumPurgingThreads = 0;
    constexpr int kNumRecordingThreads = 0;
    constexpr int kNumPrecompileThreads = 4;
    constexpr bool kDontPermute = false;

    run_test(context, reporter, kNumPurgingThreads, kNumRecordingThreads, kNumPrecompileThreads,
             kDontPermute);

    const GlobalCache::PipelineStats stats = context->priv().globalCache()->getStats();
    const PipelineManager::Stats& mgrStats =
            context->priv().sharedContext()->pipelineManager()->getStats();

    // The 48 comes from:
    //     4 gradient flavors (linear, radial, ...) *
    //     3 types of each flavor (4, 8, N) *
    //     4 precompile threads
    constexpr int kExpectedPipelines = kNumDiffPipelines *
                                       (kNumPrecompileThreads + kNumRecordingThreads);  // 48

    REPORTER_ASSERT(reporter, stats.fGraphicsCacheAdditions == kNumDiffPipelines);
    REPORTER_ASSERT(reporter, stats.fGraphicsRaces > 0);
    REPORTER_ASSERT(reporter, stats.fGraphicsPurges == 0);

    int numCacheProbes = stats.fGraphicsCacheHits + stats.fGraphicsCacheMisses;

    // This test says that every expected Pipeline will incur at least one cache probe.
    // If a task is involved then an additional cache probe is incurred. Basically, it
    // is just checking that the Pipeline Mgr is interacting as expected with the Pipeline cache.
    REPORTER_ASSERT(reporter, numCacheProbes == kExpectedPipelines +
                                                mgrStats.fNumTaskCreationRaces +
                                                mgrStats.fNumTasksCreated);
}

// This test runs two threads compiling the gradient flavours and two threads
// pre-compiling the gradient flavors. This is to exercise the tracking of the
// various race combinations (i.e., Normal vs Precompile, Normal vs. Normal, etc.).
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(ThreadedPipelinePrecompileCompileTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kNever) {
    constexpr int kNumPurgingThreads = 0;
    constexpr int kNumRecordingThreads = 2;
    constexpr int kNumPrecompileThreads = 2;
    constexpr bool kDontPermute = false;

    run_test(context, reporter, kNumPurgingThreads, kNumRecordingThreads, kNumPrecompileThreads,
             kDontPermute);

    const GlobalCache::PipelineStats stats = context->priv().globalCache()->getStats();
    const PipelineManager::Stats& mgrStats =
            context->priv().sharedContext()->pipelineManager()->getStats();

    // The 48 comes from:
    //     4 gradient flavors (linear, radial, ...) *
    //     3 types of each flavor (4, 8, N) *
    //     (2 normal-compile threads + 2 pre-compile threads)
    constexpr int kExpectedPipelines = kNumDiffPipelines *
                                       (kNumPrecompileThreads + kNumRecordingThreads);  // 48

    REPORTER_ASSERT(reporter, stats.fGraphicsCacheAdditions == kNumDiffPipelines);
    REPORTER_ASSERT(reporter, stats.fGraphicsRaces > 0);
    REPORTER_ASSERT(reporter, stats.fGraphicsPurges == 0);

    int numCacheProbes = stats.fGraphicsCacheHits + stats.fGraphicsCacheMisses;

    // This test says that every expected Pipeline will incur at least one cache probe.
    // If a task is involved then an additional cache probe is incurred. Basically, it
    // is just checking that the Pipeline Mgr is interacting as expected with the Pipeline cache
    REPORTER_ASSERT(reporter, numCacheProbes == kExpectedPipelines +
                                                mgrStats.fNumTaskCreationRaces +
                                                mgrStats.fNumTasksCreated);
}

// This test compiles the gradient flavors on a thread and then tests out the time-based
// purging.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(ThreadedPipelineCompilePurgingTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kNever) {
    constexpr int kNumPurgingThreads = 0;
    constexpr int kNumRecordingThreads = 1;
    constexpr int kNumPrecompileThreads = 0;
    constexpr bool kDontPermute = false;

    std::unique_ptr<PrecompileContext> precompileContext = context->makePrecompileContext();

    auto begin = std::chrono::steady_clock::now();

    run_test(context, reporter, kNumPurgingThreads, kNumRecordingThreads, kNumPrecompileThreads,
             kDontPermute);

    auto end = std::chrono::steady_clock::now();

    auto deltaMS = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);

    precompileContext->purgePipelinesNotUsedInMs(2*deltaMS);

    GlobalCache::PipelineStats stats = context->priv().globalCache()->getStats();

    REPORTER_ASSERT(reporter, stats.fGraphicsCacheHits == 0);
    REPORTER_ASSERT(reporter, stats.fGraphicsCacheMisses == kNumExpectedCacheSearchesPerPipeline *
                                                            kNumDiffPipelines);
    REPORTER_ASSERT(reporter, stats.fGraphicsCacheAdditions == kNumDiffPipelines);
    REPORTER_ASSERT(reporter, stats.fGraphicsRaces == 0);
    // Every created Pipeline should've been used since the start of this test
    REPORTER_ASSERT(reporter, stats.fGraphicsPurges == 0, "num purges: %d", stats.fGraphicsPurges);

    //--------------------------------------------------------------------------------------------
    const auto kSleepDuration = std::chrono::milliseconds(1);

    std::this_thread::sleep_for(kSleepDuration);

    precompileContext->purgePipelinesNotUsedInMs(kSleepDuration);

    stats = context->priv().globalCache()->getStats();

    REPORTER_ASSERT(reporter, stats.fGraphicsCacheHits == 0);
    REPORTER_ASSERT(reporter, stats.fGraphicsCacheMisses == kNumExpectedCacheSearchesPerPipeline *
                                                            kNumDiffPipelines);
    REPORTER_ASSERT(reporter, stats.fGraphicsCacheAdditions == kNumDiffPipelines);
    REPORTER_ASSERT(reporter, stats.fGraphicsRaces == 0);
    // None of the created Pipelines should've been used since we started to sleep - so they
    // all get purged.
    REPORTER_ASSERT(reporter, stats.fGraphicsPurges == kNumDiffPipelines);
}

// This test *precompiles* the gradient flavors on a thread and then tests out the time-based
// purging.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(ThreadedPipelinePrecompilePurgingTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kNever) {
    constexpr int kNumPurgingThreads = 0;
    constexpr int kNumRecordingThreads = 0;
    constexpr int kNumPrecompileThreads = 1;
    constexpr bool kDontPermute = false;

    std::unique_ptr<PrecompileContext> precompileContext = context->makePrecompileContext();

    auto begin = std::chrono::steady_clock::now();

    run_test(context, reporter, kNumPurgingThreads, kNumRecordingThreads, kNumPrecompileThreads,
             kDontPermute);

    auto end = std::chrono::steady_clock::now();

    auto deltaMS = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);

    precompileContext->purgePipelinesNotUsedInMs(deltaMS);

    GlobalCache::PipelineStats stats = context->priv().globalCache()->getStats();

    REPORTER_ASSERT(reporter, stats.fGraphicsCacheHits == 0);
    REPORTER_ASSERT(reporter, stats.fGraphicsCacheMisses == kNumExpectedCacheSearchesPerPipeline *
                                                            kNumDiffPipelines);
    REPORTER_ASSERT(reporter, stats.fGraphicsCacheAdditions == kNumDiffPipelines);
    REPORTER_ASSERT(reporter, stats.fGraphicsRaces == 0);
    // Precompilation doesn't count as a use so all the Pipelines will be purged even though
    // they were created w/in 'deltaMS'
    REPORTER_ASSERT(reporter, stats.fGraphicsPurges == kNumDiffPipelines);
}

// This test fires off two compilation threads, two precompilation threads and one
// purging thread. This is intended to stress test the Pipeline cache's thread safety and
// the purging behavior.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(ThreadedPipelinePrecompileCompilePurgingTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kNever) {
    constexpr int kNumPurgingThreads = 1;
    constexpr int kNumRecordingThreads = 2;
    constexpr int kNumPrecompileThreads = 2;
    constexpr bool kPermute = true;

    run_test(context, reporter, kNumPurgingThreads, kNumRecordingThreads, kNumPrecompileThreads,
             kPermute);

    GlobalCache::PipelineStats stats = context->priv().globalCache()->getStats();

    // Given the use of both purging and permutations there is little that can be
    // definitely tested here besides not crashing and no TSAN races.

    // Purges can force recreation of a Pipeline
    REPORTER_ASSERT(reporter, stats.fGraphicsCacheAdditions >= kNumDiffPipelines);
}

#endif // SK_GRAPHITE
