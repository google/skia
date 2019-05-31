/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "include/gpu/gl/GrGLTypes.h"

#ifndef GrGLTypesPriv_DEFINED
#define GrGLTypesPriv_DEFINED

class GrGLTextureParameters : public SkNVRefCnt<GrGLTextureParameters> {
public:
    // We currently consider texture parameters invalid on all textures
    // GrContext::resetContext(). We use this type to track whether instances of
    // GrGLTextureParameters were updated before or after the most recent resetContext(). At 10
    // resets / frame and 60fps a 64bit timestamp will overflow in about a billion years.
    // TODO: Require clients to use GrBackendTexture::glTextureParametersModified() to invalidate
    // texture parameters and get rid of timestamp checking.
    using ResetTimestamp = uint64_t;

    // This initializes the params to have an expired timestamp. They'll be considered invalid the
    // first time the texture is used unless set() is called.
    GrGLTextureParameters() = default;

    struct SamplerParams {
        SamplerParams();
        void invalidate();

        GrGLenum fMinFilter;
        GrGLenum fMagFilter;
        GrGLenum fWrapS;
        GrGLenum fWrapT;
        GrGLfloat fMinLOD;
        GrGLfloat fMaxLOD;
        // We always want the border color to be transparent black, so no need to store 4 floats.
        // Just track if it's been invalidated and no longer the default
        bool fBorderColorInvalid;
    };

    // Texture state that does not overlap with sampler object state.
    struct NonSamplerParams {
        NonSamplerParams();
        void invalidate();

        uint32_t fSwizzleKey;
        GrGLint fBaseMipMapLevel;
        GrGLint fMaxMipMapLevel;
    };

    void invalidate();

    ResetTimestamp resetTimestamp() const { return fResetTimestamp; }
    const SamplerParams& samplerParams() const { return fSamplerParams; }
    const NonSamplerParams& nonSamplerParams() const { return fNonSamplerParams; }

    // SamplerParams are optional because we don't track them when we're using sampler objects.
    void set(const SamplerParams* samplerParams,
             const NonSamplerParams& nonSamplerParams,
             ResetTimestamp currTimestamp);

private:
    static constexpr ResetTimestamp kExpiredTimestamp = 0;

    SamplerParams fSamplerParams;
    NonSamplerParams fNonSamplerParams;
    ResetTimestamp fResetTimestamp = kExpiredTimestamp;
};

class GrGLBackendTextureInfo {
public:
    GrGLBackendTextureInfo(const GrGLTextureInfo& info, GrGLTextureParameters* params)
            : fInfo(info), fParams(params) {}
    GrGLBackendTextureInfo(const GrGLBackendTextureInfo&) = delete;
    GrGLBackendTextureInfo& operator=(const GrGLBackendTextureInfo&) = delete;
    const GrGLTextureInfo& info() const { return fInfo; }
    GrGLTextureParameters* parameters() const { return fParams; }
    sk_sp<GrGLTextureParameters> refParameters() const { return sk_ref_sp(fParams); }

    void assign(const GrGLBackendTextureInfo&, bool thisIsValid);

private:
    GrGLTextureInfo fInfo;
    GrGLTextureParameters* fParams;
};

#endif
