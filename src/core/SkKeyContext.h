/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkKeyContext_DEFINED
#define SkKeyContext_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkTypes.h"

#ifdef SK_GRAPHITE_ENABLED
#include "include/core/SkM44.h"

namespace skgpu::graphite {
class Recorder;
class ShaderCodeDictionary;
}
#endif

#if SK_SUPPORT_GPU
class GrRecordingContext;
#endif


// The key context must always be able to provide a valid ShaderCodeDictionary. Depending
// on the calling context it can also supply a backend-specific resource providing
// object (e.g., a Recorder).
class SkKeyContext {
public:
#ifdef SK_GRAPHITE_ENABLED
    // Constructor for the pre-compile code path
    SkKeyContext(skgpu::graphite::ShaderCodeDictionary* dict) : fDictionary(dict) {}
    SkKeyContext(skgpu::graphite::Recorder*,
                 const SkM44& local2Dev);
    SkKeyContext(const SkKeyContext&);

    skgpu::graphite::Recorder* recorder() const { return fRecorder; }

    const SkM44& local2Dev() const { return fLocal2Dev; }
    const SkMatrix* localMatrix() const { return fLocalMatrix; }
#endif
#if SK_SUPPORT_GPU
    SkKeyContext(GrRecordingContext*);
    GrRecordingContext* recordingContext() const { return fRecordingContext; }
#endif

#ifdef SK_GRAPHITE_ENABLED
    skgpu::graphite::ShaderCodeDictionary* dict() const { return fDictionary; }
#endif

protected:
#ifdef SK_GRAPHITE_ENABLED
    skgpu::graphite::Recorder* fRecorder = nullptr;
    SkM44 fLocal2Dev;
#endif

#if SK_SUPPORT_GPU
    GrRecordingContext* fRecordingContext = nullptr;
#endif

    SkMatrix* fLocalMatrix;
#ifdef SK_GRAPHITE_ENABLED
    skgpu::graphite::ShaderCodeDictionary* fDictionary;
#endif
};

class SkKeyContextWithLocalMatrix : public SkKeyContext {
public:
    SkKeyContextWithLocalMatrix(const SkKeyContext& other, const SkMatrix& childLM)
            : SkKeyContext(other) {
        if (fLocalMatrix) {
            fStorage = SkMatrix::Concat(childLM, *fLocalMatrix);
        } else {
            fStorage = childLM;
        }

        fLocalMatrix = &fStorage;
    }

private:
    SkKeyContextWithLocalMatrix(const SkKeyContextWithLocalMatrix&) = delete;
    SkKeyContextWithLocalMatrix& operator=(const SkKeyContextWithLocalMatrix&) = delete;

    SkMatrix fStorage;
};

#endif // SkKeyContext_DEFINED
