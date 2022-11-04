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
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#endif

class GrColorInfo;
class GrRecordingContext;
class GrTextureProxy;
class SkBitmap;
class SkCanvas;
class SkImage;
struct SkImageInfo;
class SkMatrix;
class SkPaint;
class SkPixmap;
class SkShader;
class SkSpecialSurface;
class SkSurface;
enum class SkTileMode;

namespace skgpu::graphite {
class Recorder;
class TextureProxyView;
}

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
    SkISize dimensions() const { return { this->width(), this->height() }; }
    const SkIRect& subset() const { return fSubset; }

    uint32_t uniqueID() const { return fUniqueID; }

    virtual size_t getSize() const = 0;

    const SkColorInfo& colorInfo() const { return fColorInfo; }
    SkAlphaType alphaType() const { return fColorInfo.alphaType(); }
    SkColorType colorType() const { return fColorInfo.colorType(); }
    SkColorSpace* getColorSpace() const { return fColorInfo.colorSpace(); }

    /**
     *  Draw this SpecialImage into the canvas, automatically taking into account the image's subset
     */
    void draw(SkCanvas* canvas,
              SkScalar x, SkScalar y,
              const SkSamplingOptions& sampling,
              const SkPaint* paint) const {
        return this->onDraw(canvas, x, y, sampling, paint);
    }
    void draw(SkCanvas* canvas, SkScalar x, SkScalar y) const {
        this->draw(canvas, x, y, SkSamplingOptions(), nullptr);
    }

    static sk_sp<SkSpecialImage> MakeFromImage(GrRecordingContext*,
                                               const SkIRect& subset,
                                               sk_sp<SkImage>,
                                               const SkSurfaceProps&);
    static sk_sp<SkSpecialImage> MakeFromRaster(const SkIRect& subset,
                                                const SkBitmap&,
                                                const SkSurfaceProps&);
    static sk_sp<SkSpecialImage> CopyFromRaster(const SkIRect& subset,
                                                const SkBitmap&,
                                                const SkSurfaceProps&);
#if SK_SUPPORT_GPU
    static sk_sp<SkSpecialImage> MakeDeferredFromGpu(GrRecordingContext*,
                                                     const SkIRect& subset,
                                                     uint32_t uniqueID,
                                                     GrSurfaceProxyView,
                                                     const GrColorInfo&,
                                                     const SkSurfaceProps&);
#endif

#if SK_GRAPHITE_ENABLED
    static sk_sp<SkSpecialImage> MakeGraphite(skgpu::graphite::Recorder*,
                                              const SkIRect& subset,
                                              uint32_t uniqueID,
                                              skgpu::graphite::TextureProxyView,
                                              const SkColorInfo&,
                                              const SkSurfaceProps&);
#endif

    /**
     *  Create a new special surface with a backend that is compatible with this special image.
     */
    sk_sp<SkSpecialSurface> makeSurface(SkColorType,
                                        const SkColorSpace*,
                                        const SkISize& size,
                                        SkAlphaType,
                                        const SkSurfaceProps&) const;

    /**
     * Create a new surface with a backend that is compatible with this special image.
     * TODO: switch this to makeSurface once we resolved the naming issue
     * TODO (michaelludwig) - This is only used by SkTileImageFilter, which appears should be
     * updated to work correctly with subsets and then makeTightSurface() can go away entirely.
     */
    sk_sp<SkSurface> makeTightSurface(SkColorType,
                                      const SkColorSpace*,
                                      const SkISize& size,
                                      SkAlphaType = kPremul_SkAlphaType) const;

    /**
     * Extract a subset of this special image and return it as a special image.
     * It may or may not point to the same backing memory. The input 'subset' is relative to the
     * special image's content rect.
     */
    sk_sp<SkSpecialImage> makeSubset(const SkIRect& subset) const {
        SkIRect absolute = subset.makeOffset(this->subset().topLeft());
        return this->onMakeSubset(absolute);
    }

    /**
     * Create an SkImage from the contents of this special image optionally extracting a subset.
     * It may or may not point to the same backing memory.
     * Note: when no 'subset' parameter is specified the the entire SkSpecialImage will be
     * returned - including whatever extra padding may have resulted from a loose fit!
     * When the 'subset' parameter is specified the returned image will be tight even if that
     * entails a copy! The 'subset' is relative to this special image's content rect.
     */
    // TODO: The only version that uses the subset is the tile image filter, and that doesn't need
    // to if it can be rewritten to use asShader() and SkTileModes. Similarly, the only use case of
    // asImage() w/o a subset is SkImage::makeFiltered() and that could/should return an SkShader so
    // that users don't need to worry about correctly applying the subset, etc.
    sk_sp<SkImage> asImage(const SkIRect* subset = nullptr) const;

    /**
     * Create an SkShader that samples the contents of this special image, applying tile mode for
     * any sample that falls outside its internal subset.
     */
    sk_sp<SkShader> asShader(SkTileMode, const SkSamplingOptions&, const SkMatrix& lm) const;
    sk_sp<SkShader> asShader(const SkSamplingOptions& sampling) const;
    sk_sp<SkShader> asShader(const SkSamplingOptions& sampling, const SkMatrix& lm) const;

    /**
     *  If the SpecialImage is backed by a gpu texture, return true.
     */
    bool isTextureBacked() const { return SkToBool(this->onGetContext()); }

    /**
     * Return the GrRecordingContext if the SkSpecialImage is GrTexture-backed
     */
    GrRecordingContext* getContext() const { return this->onGetContext(); }

#if SK_SUPPORT_GPU
    /**
     * Regardless of how the underlying backing data is stored, returns the contents as a
     * GrSurfaceProxyView. The returned view's proxy represents the entire backing image, so texture
     * coordinates must be mapped from the content rect (e.g. relative to 'subset()') to the proxy's
     * space (offset by subset().topLeft()).
     */
    GrSurfaceProxyView view(GrRecordingContext* context) const { return this->onView(context); }
#endif

#if SK_GRAPHITE_ENABLED
    bool isGraphiteBacked() const;

    skgpu::graphite::TextureProxyView textureProxyView() const;
#endif

    /**
     *  Regardless of the underlying backing store, return the contents as an SkBitmap.
     *  The returned bitmap represents the subset accessed by this image, thus (0,0) refers to the
     *  top-left corner of 'subset'.
     */
    bool getROPixels(SkBitmap* bm) const {
        return this->onGetROPixels(bm);
    }

protected:
    SkSpecialImage(const SkIRect& subset,
                   uint32_t uniqueID,
                   const SkColorInfo&,
                   const SkSurfaceProps&);

    virtual void onDraw(SkCanvas*,
                        SkScalar x, SkScalar y,
                        const SkSamplingOptions&,
                        const SkPaint*) const = 0;

    virtual bool onGetROPixels(SkBitmap*) const = 0;

    virtual GrRecordingContext* onGetContext() const { return nullptr; }

#if SK_SUPPORT_GPU
    virtual GrSurfaceProxyView onView(GrRecordingContext*) const = 0;
#endif

#if SK_GRAPHITE_ENABLED
    virtual skgpu::graphite::TextureProxyView onTextureProxyView() const;
#endif

    // This subset is relative to the backing store's coordinate frame, it has already been mapped
    // from the content rect by the non-virtual makeSubset().
    virtual sk_sp<SkSpecialImage> onMakeSubset(const SkIRect& subset) const = 0;

    virtual sk_sp<SkSpecialSurface> onMakeSurface(SkColorType colorType,
                                                  const SkColorSpace* colorSpace,
                                                  const SkISize& size,
                                                  SkAlphaType at,
                                                  const SkSurfaceProps&) const = 0;

    // This subset (when not null) is relative to the backing store's coordinate frame, it has
    // already been mapped from the content rect by the non-virtual asImage().
    virtual sk_sp<SkImage> onAsImage(const SkIRect* subset) const = 0;

    virtual sk_sp<SkShader> onAsShader(SkTileMode,
                                       const SkSamplingOptions&,
                                       const SkMatrix&) const = 0;

    virtual sk_sp<SkSurface> onMakeTightSurface(SkColorType colorType,
                                                const SkColorSpace* colorSpace,
                                                const SkISize& size,
                                                SkAlphaType at) const = 0;

#ifdef SK_DEBUG
    static bool RectFits(const SkIRect& rect, int width, int height);
#endif

private:
    const SkIRect        fSubset;
    const uint32_t       fUniqueID;
    const SkColorInfo    fColorInfo;
    const SkSurfaceProps fProps;
};

#endif // SkSpecialImage_DEFINED
