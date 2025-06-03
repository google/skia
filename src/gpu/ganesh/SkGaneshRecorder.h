/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGaneshRecorder_DEFINED
#define SkGaneshRecorder_DEFINED

#include "include/core/SkCPURecorder.h"
#include "include/core/SkRecorder.h"
#include "include/gpu/ganesh/GrRecordingContext.h"

class GrDirectContext;

class SkGaneshRecorder final : public SkRecorder {
public:
    SkGaneshRecorder(GrRecordingContext* ctx) : fGaneshCtx(ctx) {}

    Type type() const override { return SkRecorder::Type::kGanesh; }

    GrRecordingContext* recordingContext() const { return fGaneshCtx; }

    GrDirectContext* directContext() const { return GrAsDirectContext(fGaneshCtx); }

    skcpu::Recorder* cpuRecorder() override {
        return skcpu::Recorder::TODO();
    }

private:
    GrRecordingContext* fGaneshCtx;
};

inline SkGaneshRecorder* AsGaneshRecorder(SkRecorder* recorder) {
    if (!recorder) {
        return nullptr;
    }
    if (recorder->type() != SkRecorder::Type::kGanesh) {
        return nullptr;
    }
    return static_cast<SkGaneshRecorder*>(recorder);
}

#endif
