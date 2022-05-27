/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkKeyContext_DEFINED
#define SkKeyContext_DEFINED

#include "include/core/SkTypes.h"

#ifdef SK_GRAPHITE_ENABLED
#include "include/core/SkM44.h"

namespace skgpu::graphite { class Recorder; }
#endif

#if SK_SUPPORT_GPU
class GrRecordingContext;
#endif

class SkShaderCodeDictionary;

// The key context must always be able to provide a valid SkShaderCodeDictionary. Depending
// on the calling context it can also supply a backend-specific resource providing
// object (e.g., a Recorder).
class SkKeyContext {
public:
    // Constructor for the pre-compile code path
    SkKeyContext(SkShaderCodeDictionary* dict) : fDictionary(dict) {}
#ifdef SK_GRAPHITE_ENABLED
    SkKeyContext(skgpu::graphite::Recorder*, const SkM44& dev2Local);
    skgpu::graphite::Recorder* recorder() const { return fRecorder; }

    // TODO: it is expected that 'dev2Local' will go away once we switch to the actual
    // desired way of providing local coordinates to the fragment shaders.
    const SkM44& dev2Local() const { return fDev2Local; }
#endif
#if SK_SUPPORT_GPU
    SkKeyContext(GrRecordingContext*);
    GrRecordingContext* recordingContext() const { return fRecordingContext; }
#endif

    SkShaderCodeDictionary* dict() const { return fDictionary; }

private:
#ifdef SK_GRAPHITE_ENABLED
    skgpu::graphite::Recorder* fRecorder = nullptr;
    SkM44 fDev2Local;
#endif

#if SK_SUPPORT_GPU
    GrRecordingContext* fRecordingContext = nullptr;
#endif

    SkShaderCodeDictionary* fDictionary;
};

#endif // SkKeyContext_DEFINED
