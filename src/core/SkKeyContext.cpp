/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkKeyContext.h"

#ifdef SK_GRAPHITE_ENABLED
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"

SkKeyContext::SkKeyContext(skgpu::graphite::Recorder* recorder,
                           const SkM44& local2Dev)
        : fRecorder(recorder)
        , fLocal2Dev(local2Dev)
        , fLocalMatrix(nullptr) {
    fDictionary = fRecorder->priv().shaderCodeDictionary();
}

SkKeyContext::SkKeyContext(const SkKeyContext& other)
        : fRecorder(other.fRecorder)
        , fLocal2Dev(other.fLocal2Dev)
        , fLocalMatrix(other.fLocalMatrix)
        , fDictionary(other.fDictionary) {
}

#endif

#if SK_SUPPORT_GPU
#include "include/gpu/GrRecordingContext.h"

SkKeyContext::SkKeyContext(GrRecordingContext* rContext) : fRecordingContext(rContext) {}
#endif
