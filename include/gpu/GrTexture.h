
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTexture_DEFINED
#define GrTexture_DEFINED

#include "GrSurface.h"
#include "SkPoint.h"
#include "SkRefCnt.h"

class GrTextureParams;
class GrTexturePriv;

class GrTexture : virtual public GrSurface {
public:
    GrTexture* asTexture() override { return this; }
    const GrTexture* asTexture() const override { return this; }

    /**
     *  Return the native ID or handle to the texture, depending on the
     *  platform. e.g. on OpenGL, return the texture ID.
     */
    virtual GrBackendObject getTextureHandle() const = 0;

    /**
     * This function indicates that the texture parameters (wrap mode, filtering, ...) have been
     * changed externally to Skia.
     */
    virtual void textureParamsModified() = 0;

#ifdef SK_DEBUG
    void validate() const {
        this->INHERITED::validate();
        this->validateDesc();
    }
#endif

    /** Access methods that are only to be used within Skia code. */
    inline GrTexturePriv texturePriv();
    inline const GrTexturePriv texturePriv() const;

protected:
    GrTexture(GrGpu*, LifeCycle, const GrSurfaceDesc&);

    void validateDesc() const;

private:
    size_t onGpuMemorySize() const override;
    void dirtyMipMaps(bool mipMapsDirty);

    enum MipMapsStatus {
        kNotAllocated_MipMapsStatus,
        kAllocated_MipMapsStatus,
        kValid_MipMapsStatus
    };

    MipMapsStatus   fMipMapsStatus;
    // These two shift a fixed-point value into normalized coordinates	
    // for this texture if the texture is power of two sized.
    int             fShiftFixedX;
    int             fShiftFixedY;

    friend class GrTexturePriv;

    typedef GrSurface INHERITED;
};

#endif
