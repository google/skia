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

    virtual SkISize backingStoreDimensions() const = 0;

    virtual size_t getSize() const = 0;

    bool isExactFit() const { return fSubset == SkIRect::MakeSize(this->backingStoreDimensions()); }

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
              const SkPaint* paint,
              bool strict = true) const;
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
     * Create an SkImage view of the contents of this special image, pointing to the same
     * underlying memory.
     *
     * TODO: If SkImages::MakeFiltered were to return an SkShader that accounted for the subset
     * constraint and offset, then this could move to a private virtual for use in draw() and
     * asShader().
     */
    virtual sk_sp<SkImage> asImage() const = 0;

    /**
     * Create an SkShader that samples the contents of this special image, applying tile mode for
     * any sample that falls outside its internal subset.
     *
     * 'strict' defaults to true and applies shader-based tiling to the subset. If the subset is
     * the same as the backing store dimensions, it is automatically degraded to non-strict
     * (HW tiling and sampling). 'strict' can be set to false if it's known that the subset
     * boundaries aren't visible AND the texel data in adjacent rows/cols is valid to be included
     * by the given sampling options.
     */
    virtual sk_sp<SkShader> asShader(SkTileMode,
                                     const SkSamplingOptions&,
                                     const SkMatrix& lm,
                                     bool strict=true) const;

    /**
     *  If the SpecialImage is backed by a gpu texture, return true.
     */
    virtual bool isGaneshBacked() const { return false; }
    virtual bool isGraphiteBacked() const { return false; }

    /**
     * Return the GrRecordingContext if the SkSpecialImage is GrTexture-backed
     */
    virtual GrRecordingContext* getContext() const { return nullptr; }

protected:
    SkSpecialImage(const SkIRect& subset,
                   uint32_t uniqueID,
                   const SkColorInfo&,
                   const SkSurfaceProps&);

    // This subset is relative to the backing store's coordinate frame, it has already been mapped
    // from the content rect by the non-virtual makeSubset().
    virtual sk_sp<SkSpecialImage> onMakeSubset(const SkIRect& subset) const = 0;

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

bool AsBitmap(const SkSpecialImage* img, SkBitmap*);

}  // namespace SkSpecialImages

#endif // SkSpecialImage_DEFINED
