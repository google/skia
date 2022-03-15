/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkKeyContext.h"

#ifdef SK_GRAPHITE_ENABLED
#include "experimental/graphite/src/RecorderPriv.h"
#include "experimental/graphite/src/ResourceProvider.h"

SkKeyContext::SkKeyContext(skgpu::Recorder* recorder) : fRecorder(recorder) {
    fDictionary = fRecorder->priv().resourceProvider()->shaderCodeDictionary();
}
#endif

#if SK_SUPPORT_GPU
#include "include/gpu/GrRecordingContext.h"

SkKeyContext::SkKeyContext(GrRecordingContext* rContext) : fRecordingContext(rContext) {
    // TODO: fill this out for Ganesh
    fDictionary = nullptr;
}
#endif
