/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file
 */

#ifndef SkSpecialImage_DEFINED
#define SkSpecialImage_DEFINED

#include "SkNextID.h"
#include "SkRefCnt.h"
#include "SkSurfaceProps.h"

// remove this when internal_getProxy goes away (see skbug.com/4965)
#include "SkImageFilter.h"

#include "SkImageInfo.h" // for SkAlphaType

class GrContext;
class GrTexture;
class SkBitmap;
class SkCanvas;
class SkImage;
struct SkImageInfo;
class SkPaint;
class SkPixmap;
class SkSpecialSurface;
class SkSurface;

enum {
    kNeedNewImageUniqueID_SpecialImage = 0
};

/**
 * This is a restricted form of SkImage solely intended for internal use. It
 * differs from SkImage in that:
 *      - it can only be backed by raster or gpu (no generators)
 *      - it can be backed by a GrTexture larger than its nominal bounds
 *      - it can't be drawn tiled
 *      - it can't be drawn with MIPMAPs
 * It is similar to SkImage in that it abstracts how the pixels are stored/represented.
 *
 * Note: the contents of the backing storage outside of the subset rect are undefined.
 */
class SkSpecialImage : public SkRefCnt {
public:
    typedef void* ReleaseContext;
    typedef void(*RasterReleaseProc)(void* pixels, ReleaseContext);

    const SkSurfaceProps& props() const { return fProps; }

    int width() const { return fSubset.width(); }
    int height() const { return fSubset.height(); }
    const SkIRect& subset() const { return fSubset; }

    uint32_t uniqueID() const { return fUniqueID; }
    virtual bool isOpaque() const { return false; }
    virtual size_t getSize() const = 0;

    /**
     *  Ensures that a special image is backed by a texture (when GrContext is non-null). If no
     *  transformation is required, the returned image may be the same as this special image.
     *  If this special image is from a different GrContext, this will fail.
     */
    sk_sp<SkSpecialImage> makeTextureImage(SkImageFilter::Proxy*, GrContext*);

    /**
     *  Draw this SpecialImage into the canvas.
     */
    void draw(SkCanvas*, SkScalar x, SkScalar y, const SkPaint*) const;

    static sk_sp<SkSpecialImage> MakeFromImage(SkImageFilter::Proxy*,
                                               const SkIRect& subset,
                                               sk_sp<SkImage>,
                                               const SkSurfaceProps* = nullptr);
    static sk_sp<SkSpecialImage> MakeFromRaster(SkImageFilter::Proxy*,
                                                const SkIRect& subset,
                                                const SkBitmap&,
                                                const SkSurfaceProps* = nullptr);
    static sk_sp<SkSpecialImage> MakeFromGpu(SkImageFilter::Proxy*,
                                             const SkIRect& subset,
                                             uint32_t uniqueID,
                                             GrTexture*,
                                             const SkSurfaceProps* = nullptr,
                                             SkAlphaType at = kPremul_SkAlphaType);
    static sk_sp<SkSpecialImage> MakeFromPixmap(SkImageFilter::Proxy*,
                                                const SkIRect& subset,
                                                const SkPixmap&,
                                                RasterReleaseProc,
                                                ReleaseContext,
                                                const SkSurfaceProps* = nullptr);

    /**
     *  Create a new special surface with a backend that is compatible with this special image.
     */
    sk_sp<SkSpecialSurface> makeSurface(const SkImageInfo&) const;

    /**
     * Create a new surface with a backend that is compatible with this special image.
     * TODO: switch this to makeSurface once we resolved the naming issue
     */
    sk_sp<SkSurface> makeTightSurface(const SkImageInfo&) const;

    /**
     * Extract a subset of this special image and return it as a special image.
     * It may or may not point to the same backing memory.
     */
    sk_sp<SkSpecialImage> makeSubset(const SkIRect& subset) const;

    /**
     * Extract a subset of this special image and return it as an SkImage.
     * It may or may not point to the same backing memory.
     * TODO: switch this to makeSurface once we resolved the naming issue
     */
    sk_sp<SkImage> makeTightSubset(const SkIRect& subset) const;

    // These three internal methods will go away (see skbug.com/4965)
    bool internal_getBM(SkBitmap* result);
    static sk_sp<SkSpecialImage> internal_fromBM(SkImageFilter::Proxy*, const SkBitmap&,
                                                 const SkSurfaceProps*);
    SkImageFilter::Proxy* internal_getProxy() const;

    // TODO: hide this when GrLayerHoister uses SkSpecialImages more fully (see skbug.com/5063)
    /**
     *  If the SpecialImage is backed by a gpu texture, return that texture.
     *  The active portion of the texture can be retrieved via 'subset'.
     */
    GrTexture* peekTexture() const;

    // TODO: hide this whe the imagefilter all have a consistent draw path (see skbug.com/5063)
    /**
     *  If the SpecialImage is backed by cpu pixels, return the const address
     *  of those pixels and, if not null, the ImageInfo, rowBytes, and, if present,
     *  the color table. The returned address(es) is/are only valid while the image object
     *  is in scope.
     *
     *  The returned ImageInfo represents the backing memory. Use 'subset'
     *  to get the active portion's dimensions.
     *
     *  On failure, return false and ignore the pixmap parameter.
     */
    bool peekPixels(SkPixmap*) const;

protected:
    SkSpecialImage(SkImageFilter::Proxy*, const SkIRect& subset, uint32_t uniqueID,
                   const SkSurfaceProps*);

    // The following 2 are for testing and shouldn't be used.
    friend class TestingSpecialImageAccess;
    friend class TestingSpecialSurfaceAccess;

    // This entry point is for testing only. It does a readback from VRAM for
    // GPU-backed special images.
    bool testingOnlyGetROPixels(SkBitmap*) const;

    // TODO: remove this ASAP (see skbug.com/4965)
    SkImageFilter::Proxy* proxy() const { return fProxy; }

private:
    const SkSurfaceProps fProps;
    const SkIRect        fSubset;
    const uint32_t       fUniqueID;

    // TODO: remove this ASAP (see skbug.com/4965)
    SkImageFilter::Proxy* fProxy;

    typedef SkRefCnt INHERITED;
};

#endif
