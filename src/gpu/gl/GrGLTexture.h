/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrGLTexture_DEFINED
#define GrGLTexture_DEFINED

#include "GrGpu.h"
#include "GrTexture.h"
#include "GrGLUtil.h"

class GrGLGpu;

class GrGLTexture : public GrTexture {

public:
    struct TexParams {
        GrGLenum fMinFilter;
        GrGLenum fMagFilter;
        GrGLenum fWrapS;
        GrGLenum fWrapT;
        GrGLenum fSwizzleRGBA[4];
        void invalidate() { memset(this, 0xff, sizeof(TexParams)); }
    };

    struct IDDesc {
        GrGLuint                    fTextureID;
        GrGpuResource::LifeCycle    fLifeCycle;
    };

    GrGLTexture(GrGLGpu*, const GrSurfaceDesc&, const IDDesc&);

    GrBackendObject getTextureHandle() const override;

    void textureParamsModified() override { fTexParams.invalidate(); }

    // These functions are used to track the texture parameters associated with the texture.
    const TexParams& getCachedTexParams(GrGpu::ResetTimestamp* timestamp) const {
        *timestamp = fTexParamsTimestamp;
        return fTexParams;
    }

    void setCachedTexParams(const TexParams& texParams,
                            GrGpu::ResetTimestamp timestamp) {
        fTexParams = texParams;
        fTexParamsTimestamp = timestamp;
    }

    GrGLuint textureID() const { return fTextureID; }

protected:
    // The public constructor registers this object with the cache. However, only the most derived
    // class should register with the cache. This constructor does not do the registration and
    // rather moves that burden onto the derived class.
    enum Derived { kDerived };
    GrGLTexture(GrGLGpu*, const GrSurfaceDesc&, const IDDesc&, Derived);

    void init(const GrSurfaceDesc&, const IDDesc&);

    void onAbandon() override;
    void onRelease() override;

private:
    TexParams                       fTexParams;
    GrGpu::ResetTimestamp           fTexParamsTimestamp;
    GrGLuint                        fTextureID;

    // We track this separately from GrGpuResource because this may be both a texture and a render
    // target, and the texture may be wrapped while the render target is not.
    LifeCycle                       fTextureIDLifecycle;

    typedef GrTexture INHERITED;
};

#endif
