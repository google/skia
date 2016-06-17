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
        GrGLenum fMaxMipMapLevel;
        GrGLenum fSwizzleRGBA[4];
        GrGLenum fSRGBDecode;
        void invalidate() { memset(this, 0xff, sizeof(TexParams)); }
    };

    struct IDDesc {
        GrGLTextureInfo             fInfo;
        GrBackendObjectOwnership    fOwnership;
    };
    GrGLTexture(GrGLGpu*, SkBudgeted, const GrSurfaceDesc&, const IDDesc&);
    GrGLTexture(GrGLGpu*, SkBudgeted, const GrSurfaceDesc&, const IDDesc&,
                bool wasMipMapDataProvided);

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

    GrGLuint textureID() const { return fInfo.fID; }

    GrGLenum target() const { return fInfo.fTarget; }

    static GrGLTexture* CreateWrapped(GrGLGpu*, const GrSurfaceDesc&, const IDDesc&);
protected:
    // Constructor for subclasses.
    GrGLTexture(GrGLGpu*, const GrSurfaceDesc&, const IDDesc&);

    enum Wrapped { kWrapped };
    // Constructor for instances wrapping backend objects.
    GrGLTexture(GrGLGpu*, Wrapped, const GrSurfaceDesc&, const IDDesc&);

    void init(const GrSurfaceDesc&, const IDDesc&);

    void onAbandon() override;
    void onRelease() override;
    void setMemoryBacking(SkTraceMemoryDump* traceMemoryDump,
                          const SkString& dumpName) const override;

private:
    TexParams                       fTexParams;
    GrGpu::ResetTimestamp           fTexParamsTimestamp;
    // Holds the texture target and ID. A pointer to this may be shared to external clients for
    // direct interaction with the GL object.
    GrGLTextureInfo                 fInfo;
    GrBackendObjectOwnership        fTextureIDOwnership;

    typedef GrTexture INHERITED;
};

#endif
