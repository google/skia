/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file
 */

#ifndef SkSpecialImage_DEFINED
#define SkSpecialImage_DEFINED

#include "include/core/SkImageInfo.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurfaceProps.h"

#include <cstddef>
#include <cstdint>

class GrRecordingContext;
class SkBitmap;
class SkCanvas;
class SkColorSpace;
class SkImage;
class SkMatrix;
class SkPaint;
class SkShader;
enum SkAlphaType : int;
enum SkColorType : int;
enum class SkTileMode;

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
    virtual bool isGaneshBacked() const { return false; }
    virtual bool isGraphiteBacked() const { return false; }

    /**
     * Return the GrRecordingContext if the SkSpecialImage is GrTexture-backed
     */
    virtual GrRecordingContext* getContext() const { return nullptr; }

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

    // This subset is relative to the backing store's coordinate frame, it has already been mapped
    // from the content rect by the non-virtual makeSubset().
    virtual sk_sp<SkSpecialImage> onMakeSubset(const SkIRect& subset) const = 0;

    // This subset (when not null) is relative to the backing store's coordinate frame, it has
    // already been mapped from the content rect by the non-virtual asImage().
    virtual sk_sp<SkImage> onAsImage(const SkIRect* subset) const = 0;

    virtual sk_sp<SkShader> onAsShader(SkTileMode,
                                       const SkSamplingOptions&,
                                       const SkMatrix&) const = 0;

private:
    const SkIRect        fSubset;
    const uint32_t       fUniqueID;
    const SkColorInfo    fColorInfo;
    const SkSurfaceProps fProps;
};

namespace SkSpecialImages {

sk_sp<SkSpecialImage> MakeFromRaster(const SkIRect& subset, sk_sp<SkImage>, const SkSurfaceProps&);
sk_sp<SkSpecialImage> MakeFromRaster(const SkIRect& subset, const SkBitmap&, const SkSurfaceProps&);
sk_sp<SkSpecialImage> CopyFromRaster(const SkIRect& subset, const SkBitmap&, const SkSurfaceProps&);

}  // namespace SkSpecialImages

#endif // SkSpecialImage_DEFINED
