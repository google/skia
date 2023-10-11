/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/ganesh/gl/GrGLTypesPriv.h"

#include "include/core/SkScalar.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/gl/GrGLDefines.h"

GrGLTextureParameters::SamplerOverriddenState::SamplerOverriddenState()
        // These are the OpenGL defaults.
        : fMinFilter(GR_GL_NEAREST_MIPMAP_LINEAR)
        , fMagFilter(GR_GL_LINEAR)
        , fWrapS(GR_GL_REPEAT)
        , fWrapT(GR_GL_REPEAT)
        , fMinLOD(-1000.f)
        , fMaxLOD(1000.f)
        , fMaxAniso(1.f)
        , fBorderColorInvalid(false) {}

void GrGLTextureParameters::SamplerOverriddenState::invalidate() {
    fMinFilter = ~0U;
    fMagFilter = ~0U;
    fWrapS = ~0U;
    fWrapT = ~0U;
    fMinLOD = SK_ScalarNaN;
    fMaxLOD = SK_ScalarNaN;
    fMaxAniso = -1.f;
    fBorderColorInvalid = true;
}

GrGLTextureParameters::NonsamplerState::NonsamplerState()
        // These are the OpenGL defaults.
        : fBaseMipMapLevel(0), fMaxMipmapLevel(1000), fSwizzleIsRGBA(true) {}

void GrGLTextureParameters::NonsamplerState::invalidate() {
    fSwizzleIsRGBA = false;
    fBaseMipMapLevel = ~0;
    fMaxMipmapLevel = ~0;
}

void GrGLTextureParameters::invalidate() {
    fSamplerOverriddenState.invalidate();
    fNonsamplerState.invalidate();
}

void GrGLTextureParameters::set(const SamplerOverriddenState* samplerState,
                                const NonsamplerState& nonsamplerState,
                                ResetTimestamp currTimestamp) {
    if (samplerState) {
        fSamplerOverriddenState = *samplerState;
    }
    fNonsamplerState = nonsamplerState;
    fResetTimestamp = currTimestamp;
}

GrGLSurfaceInfo GrGLTextureSpecToSurfaceInfo(const GrGLTextureSpec& glSpec,
                                             uint32_t sampleCount,
                                             uint32_t levelCount,
                                             GrProtected isProtected) {
    GrGLSurfaceInfo info;
    // Shared info
    info.fSampleCount = sampleCount;
    info.fLevelCount = levelCount;
    info.fProtected = isProtected;

    // GL info
    info.fTarget = glSpec.fTarget;
    info.fFormat = glSpec.fFormat;

    return info;
}
