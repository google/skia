/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/window/WindowContext.h"

#include "include/gpu/GrDirectContext.h"
#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#endif

namespace skwindow {

WindowContext::WindowContext(const DisplayParams& params)
        : fDisplayParams(params) {}

WindowContext::~WindowContext() {}

void WindowContext::swapBuffers() {
    this->onSwapBuffers();
}

#if defined(SK_GRAPHITE)
void WindowContext::snapRecordingAndSubmit() {
    if (fGraphiteContext) {
        SkASSERT(fGraphiteRecorder);
        std::unique_ptr<skgpu::graphite::Recording> recording = fGraphiteRecorder->snap();
        if (recording) {
            skgpu::graphite::InsertRecordingInfo info;
            info.fRecording = recording.get();
            fGraphiteContext->insertRecording(info);
            fGraphiteContext->submit(skgpu::graphite::SyncToCpu::kNo);
        }
    }
}
#endif

}  // namespace skwindow
