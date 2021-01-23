/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file
 */

#ifndef SkSpecialImage_DEFINED
#define SkSpecialImage_DEFINED

#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSurfaceProps.h"
#include "src/core/SkNextID.h"

#if SK_SUPPORT_GPU
#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrSurfaceProxyView.h"
#endif

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
    virtual SkColorType colorType() const = 0;
    virtual size_t getSize() const = 0;

    /**
     *  Draw this SpecialImage into the canvas, automatically taking into account the image's subset
     */
    void draw(SkCanvas*, SkScalar x, SkScalar y, const SkSamplingOptions&, const SkPaint*) const;
    void draw(SkCanvas* canvas, SkScalar x, SkScalar y) const {
        this->draw(canvas, x, y, SkSamplingOptions(), nullptr);
    }

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
                                                     GrSurfaceProxyView,
                                                     GrColorType,
                                                     sk_sp<SkColorSpace>,
                                                     const SkSurfaceProps* = nullptr,
                                                     SkAlphaType at = kPremul_SkAlphaType);
#endif

    /**
     *  Create a new special surface with a backend that is compatible with this special image.
     */
    sk_sp<SkSpecialSurface> makeSurface(SkColorType colorType,
                                        const SkColorSpace* colorSpace,
                                        const SkISize& size,
                                        SkAlphaType at = kPremul_SkAlphaType,
                                        const SkSurfaceProps* props = nullptr) const;

    /**
     * Create a new surface with a backend that is compatible with this special image.
     * TODO: switch this to makeSurface once we resolved the naming issue
     * TODO (michaelludwig) - This is only used by SkTileImageFilter, which appears should be
     * updated to work correctly with subsets and then makeTightSurface() can go away entirely.
     */
    sk_sp<SkSurface> makeTightSurface(SkColorType colorType,
                                      const SkColorSpace* colorSpace,
                                      const SkISize& size,
                                      SkAlphaType at = kPremul_SkAlphaType) const;

    /**
     * Extract a subset of this special image and return it as a special image.
     * It may or may not point to the same backing memory. The input 'subset' is relative to the
     * special image's content rect.
     */
    sk_sp<SkSpecialImage> makeSubset(const SkIRect& subset) const;

    /**
     * Create an SkImage from the contents of this special image optionally extracting a subset.
     * It may or may not point to the same backing memory.
     * Note: when no 'subset' parameter is specified the the entire SkSpecialImage will be
     * returned - including whatever extra padding may have resulted from a loose fit!
     * When the 'subset' parameter is specified the returned image will be tight even if that
     * entails a copy! The 'subset' is relative to this special image's content rect.
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
     * Regardless of how the underlying backing data is stored, returns the contents as a
     * GrSurfaceProxyView. The returned view's proxy represents the entire backing image, so texture
     * coordinates must be mapped from the content rect (e.g. relative to 'subset()') to the proxy's
     * space (offset by subset().topLeft()).
     */
    GrSurfaceProxyView view(GrRecordingContext*) const;
#endif

    /**
     *  Regardless of the underlying backing store, return the contents as an SkBitmap.
     *  The returned bitmap represents the subset accessed by this image, thus (0,0) refers to the
     *  top-left corner of 'subset'.
     */
    bool getROPixels(SkBitmap*) const;

protected:
    SkSpecialImage(const SkIRect& subset, uint32_t uniqueID, const SkSurfaceProps*);

private:
    const SkSurfaceProps fProps;
    const SkIRect        fSubset;
    const uint32_t       fUniqueID;

    using INHERITED = SkRefCnt;
};

#endif
