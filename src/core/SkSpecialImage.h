/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file
 */

#ifndef SkSpecialImage_DEFINED
#define SkSpecialImage_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSurfaceProps.h"
#include "src/core/SkNextID.h"

#include "include/core/SkImageFilter.h"
#include "include/core/SkImageInfo.h"

class GrRecordingContext;
class GrTextureProxy;
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
 *      - it can be backed by a GrTextureProxy larger than its nominal bounds
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
    SkColorSpace* getColorSpace() const;

    uint32_t uniqueID() const { return fUniqueID; }
    virtual SkAlphaType alphaType() const = 0;
    virtual size_t getSize() const = 0;

    /**
     *  Ensures that a special image is backed by a texture (when GrRecordingContext is non-null).
     *  If no transformation is required, the returned image may be the same as this special image.
     *  If this special image is from a different GrRecordingContext, this will fail.
     */
    sk_sp<SkSpecialImage> makeTextureImage(GrRecordingContext*);

    /**
     *  Draw this SpecialImage into the canvas.
     */
    void draw(SkCanvas*, SkScalar x, SkScalar y, const SkPaint*) const;

    static sk_sp<SkSpecialImage> MakeFromImage(GrRecordingContext*,
                                               const SkIRect& subset,
                                               sk_sp<SkImage>,
                                               const SkSurfaceProps* = nullptr);
    static sk_sp<SkSpecialImage> MakeFromRaster(const SkIRect& subset,
                                                const SkBitmap&,
                                                const SkSurfaceProps* = nullptr);
    static sk_sp<SkSpecialImage> CopyFromRaster(const SkIRect& subset,
                                                const SkBitmap&,
                                                const SkSurfaceProps* = nullptr);
#if SK_SUPPORT_GPU
    static sk_sp<SkSpecialImage> MakeDeferredFromGpu(GrRecordingContext*,
                                                     const SkIRect& subset,
                                                     uint32_t uniqueID,
                                                     sk_sp<GrTextureProxy>,
                                                     sk_sp<SkColorSpace>,
                                                     const SkSurfaceProps* = nullptr,
                                                     SkAlphaType at = kPremul_SkAlphaType);
#endif

    /**
     *  Create a new special surface with a backend that is compatible with this special image.
     */
    sk_sp<SkSpecialSurface> makeSurface(const SkImageFilter::OutputProperties& outProps,
                                        const SkISize& size,
                                        SkAlphaType at = kPremul_SkAlphaType,
                                        const SkSurfaceProps* props = nullptr) const;

    /**
     * Create a new surface with a backend that is compatible with this special image.
     * TODO: switch this to makeSurface once we resolved the naming issue
     */
    sk_sp<SkSurface> makeTightSurface(const SkImageFilter::OutputProperties& outProps,
                                      const SkISize& size,
                                      SkAlphaType at = kPremul_SkAlphaType) const;

    /**
     * Extract a subset of this special image and return it as a special image.
     * It may or may not point to the same backing memory.
     */
    sk_sp<SkSpecialImage> makeSubset(const SkIRect& subset) const;

    /**
     * Create an SkImage from the contents of this special image optionally extracting a subset.
     * It may or may not point to the same backing memory.
     * Note: when no 'subset' parameter is specified the the entire SkSpecialImage will be
     * returned - including whatever extra padding may have resulted from a loose fit!
     * When the 'subset' parameter is specified the returned image will be tight even if that
     * entails a copy!
     */
    sk_sp<SkImage> asImage(const SkIRect* subset = nullptr) const;

    /**
     *  If the SpecialImage is backed by a gpu texture, return true.
     */
    bool isTextureBacked() const;

    /**
     * Return the GrRecordingContext if the SkSpecialImage is GrTexture-backed
     */
    GrRecordingContext* getContext() const;

#if SK_SUPPORT_GPU
    /**
     *  Regardless of the underlying backing store, return the contents as a GrTextureProxy.
     *  The active portion of the texture can be retrieved via 'subset'.
     */
    sk_sp<GrTextureProxy> asTextureProxyRef(GrRecordingContext*) const;
#endif

    /**
     *  Regardless of the underlying backing store, return the contents as an SkBitmap
     *
     *  The returned ImageInfo represents the backing memory. Use 'subset'
     *  to get the active portion's dimensions.
     */
    bool getROPixels(SkBitmap*) const;

protected:
    SkSpecialImage(const SkIRect& subset, uint32_t uniqueID, const SkSurfaceProps*);

private:
    const SkSurfaceProps fProps;
    const SkIRect        fSubset;
    const uint32_t       fUniqueID;

    typedef SkRefCnt INHERITED;
};

#endif
