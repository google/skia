/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/window/WindowContext.h"

#if defined(SK_GANESH)
#include "include/gpu/ganesh/GrDirectContext.h"
#endif

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#endif

namespace skwindow {

WindowContext::WindowContext(std::unique_ptr<const DisplayParams> params)
        : fDisplayParams(std::move(params)) {}

WindowContext::~WindowContext() {}

void WindowContext::swapBuffers() {
    this->onSwapBuffers();
}

bool WindowContext::supportsGpuTimer() const {
#if defined(SK_GRAPHITE) || defined(SK_GANESH)
    auto flags = skgpu::GpuStatsFlags::kNone;
#if defined(SK_GRAPHITE)
    if (fGraphiteContext) {
        flags = fGraphiteContext->supportedGpuStats();
    }
#endif
#if defined(SK_GANESH)
    if (fContext) {
        flags = fContext->supportedGpuStats();
    }
#endif
    using T = std::underlying_type_t<skgpu::GpuStatsFlags>;
    return static_cast<T>(flags) & static_cast<T>(skgpu::GpuStatsFlags::kElapsedTime);
#else
    return false;
#endif
}

void WindowContext::submitToGpu(GpuTimerCallback statsCallback) {
#if defined(SK_GRAPHITE)
    if (fGraphiteContext) {
        SkASSERT(fGraphiteRecorder);
        std::unique_ptr<skgpu::graphite::Recording> recording = fGraphiteRecorder->snap();
        if (recording) {
            skgpu::graphite::InsertRecordingInfo info;
            if (statsCallback) {
                auto callback = std::make_unique<GpuTimerCallback>(std::move(statsCallback));
                info.fFinishedContext = callback.release();
                info.fFinishedWithStatsProc = [](skgpu::graphite::GpuFinishedContext context,
                                                 skgpu::CallbackResult,
                                                 const skgpu::GpuStats& stats) {
                    std::unique_ptr<GpuTimerCallback> callback{
                        static_cast<GpuTimerCallback*>(context)};
                    (*callback)(stats.elapsedTime);
                };
                info.fGpuStatsFlags = skgpu::GpuStatsFlags::kElapsedTime;
            }
            info.fRecording = recording.get();
            fGraphiteContext->insertRecording(info);
            fGraphiteContext->submit(skgpu::graphite::SyncToCpu::kNo);
        }
        return;
    }
#endif
#if defined(SK_GANESH)
    if (auto dc = this->directContext()) {
        GrFlushInfo info;
        if (statsCallback) {
            auto callback = std::make_unique<GpuTimerCallback>(std::move(statsCallback));
            info.fFinishedContext = callback.release();
            info.fFinishedWithStatsProc = [](GrGpuFinishedContext context,
                                             const skgpu::GpuStats& stats) {
                std::unique_ptr<GpuTimerCallback> callback{static_cast<GpuTimerCallback*>(context)};
                (*callback)(stats.elapsedTime);
            };
            info.fGpuStatsFlags = skgpu::GpuStatsFlags::kElapsedTime;
        }
        dc->flush(info);
        dc->submit();
        return;
    }
#endif
#if defined(SK_GANESH) || defined(SK_GRAPHITE)
    if (statsCallback) {
        statsCallback(0);
    }
#endif
}

}  // namespace skwindow
