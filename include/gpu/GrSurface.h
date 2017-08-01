/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrSurface_DEFINED
#define GrSurface_DEFINED

#include "GrTypes.h"
#include "GrGpuResource.h"
#include "SkImageInfo.h"
#include "SkRect.h"

class GrRenderTarget;
class GrSurfacePriv;
class GrTexture;

class SK_API GrSurface : public GrGpuResource {
public:
    /**
     * Retrieves the width of the surface.
     */
    int width() const { return fWidth; }

    /**
     * Retrieves the height of the surface.
     */
    int height() const { return fHeight; }

    /**
     * Helper that gets the width and height of the surface as a bounding rectangle.
     */
    SkRect getBoundsRect() const { return SkRect::MakeIWH(this->width(), this->height()); }

    GrSurfaceOrigin origin() const {
        SkASSERT(kTopLeft_GrSurfaceOrigin == fOrigin || kBottomLeft_GrSurfaceOrigin == fOrigin);
        return fOrigin;
    }

    /**
     * Retrieves the pixel config specified when the surface was created.
     * For render targets this can be kUnknown_GrPixelConfig
     * if client asked us to render to a target that has a pixel
     * config that isn't equivalent with one of our configs.
     */
    GrPixelConfig config() const { return fConfig; }

    /**
     * @return the texture associated with the surface, may be null.
     */
    virtual GrTexture* asTexture() { return nullptr; }
    virtual const GrTexture* asTexture() const { return nullptr; }

    /**
     * @return the render target underlying this surface, may be null.
     */
    virtual GrRenderTarget* asRenderTarget() { return nullptr; }
    virtual const GrRenderTarget* asRenderTarget() const { return nullptr; }

    /** Access methods that are only to be used within Skia code. */
    inline GrSurfacePriv surfacePriv();
    inline const GrSurfacePriv surfacePriv() const;

    static size_t WorstCaseSize(const GrSurfaceDesc& desc, bool useNextPow2 = false);
    static size_t ComputeSize(GrPixelConfig config, int width, int height, int colorSamplesPerPixel,
                              bool hasMIPMaps, bool useNextPow2 = false);

protected:
    // Methods made available via GrSurfacePriv
    bool hasPendingRead() const;
    bool hasPendingWrite() const;
    bool hasPendingIO() const;

    // Provides access to methods that should be public within Skia code.
    friend class GrSurfacePriv;

    GrSurface(GrGpu* gpu, const GrSurfaceDesc& desc)
            : INHERITED(gpu)
            , fConfig(desc.fConfig)
            , fWidth(desc.fWidth)
            , fHeight(desc.fHeight)
            , fOrigin(desc.fOrigin) {}
    ~GrSurface() override {}


    void onRelease() override;
    void onAbandon() override;

private:
    GrPixelConfig        fConfig;
    int                  fWidth;
    int                  fHeight;
    GrSurfaceOrigin      fOrigin;

    typedef GrGpuResource INHERITED;
};

#endif
