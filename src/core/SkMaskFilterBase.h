/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMaskFilterBase_DEFINED
#define SkMaskFilterBase_DEFINED

#include "include/core/SkFlattenable.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStrokeRec.h"
#include "include/private/base/SkNoncopyable.h"
#include "src/core/SkMask.h"

#include <optional>
#include <utility>

class SkPaint;
class SkBlitter;
class SkImageFilter;
class SkCachedData;
class SkMatrix;
class SkResourceCache;
class SkPath;
class SkRRect;
class SkRasterClip;
enum SkBlurStyle : int;

class SkMaskFilterBase : public SkMaskFilter {
public:
    /** Returns the format of the resulting mask that this subclass will return
        when its filterMask() method is called.
    */
    virtual SkMask::Format getFormat() const = 0;

    /** Create a new mask by filter the src mask.
        If src.fImage == null, then do not allocate or create the dst image
        but do fill out the other fields in dstMask.
        If you do allocate a dst image, use SkMask::AllocImage()
        If this returns false, dst mask is ignored.
        @param  dst the result of the filter. If src.fImage == null, dst should not allocate its image
        @param src the original image to be filtered.
        @param matrix the CTM
        @param margin   if not null, return the buffer dx/dy need when calculating the effect. Used when
                        drawing a clipped object to know how much larger to allocate the src before
                        applying the filter. If returning false, ignore this parameter.
        @return true if the dst mask was correctly created.
    */
    virtual bool filterMask(SkMaskBuilder* dst, const SkMask& src, const SkMatrix&,
                            SkIPoint* margin) const = 0;

    enum class Type {
        kBlur,
        kEmboss,
        kSDF,
        kShader,
        kTable,
    };

    virtual Type type() const = 0;

    /**
     * The fast bounds function is used to enable the paint to be culled early
     * in the drawing pipeline. This function accepts the current bounds of the
     * paint as its src param and the filter adjust those bounds using its
     * current mask and returns the result using the dest param. Callers are
     * allowed to provide the same struct for both src and dest so each
     * implementation must accommodate that behavior.
     *
     *  The default impl calls filterMask with the src mask having no image,
     *  but subclasses may override this if they can compute the rect faster.
     */
    virtual void computeFastBounds(const SkRect& src, SkRect* dest) const;

    struct BlurRec {
        SkScalar        fSigma;
        SkBlurStyle     fStyle;
    };
    /**
     *  If this filter can be represented by a BlurRec, return true and (if not null) fill in the
     *  provided BlurRec parameter. If this effect cannot be represented as a BlurRec, return false
     *  and ignore the BlurRec parameter.
     */
    virtual bool asABlur(BlurRec*) const;

    /**
     * Return an SkImageFilter representation of this mask filter that SkCanvas can apply
     * to an alpha-only image to produce an equivalent effect to running the mask filter directly.
     *
     * Additionally, return a boolean that indicates if the image filter applies shading properties.
     * When restoring a layer, this affects whether to draw a rgba image or blend the coverage
     * mask (A8 image).
     *
     * The paint parameter can be used to apply shading. Some mask filters (e.g. EmbossMaskFilter)
     * may not produce correct results under these circumstances and different blend modes,
     * given that the coverage mask will be blended in the mask filter as image filter impl in
     * these cases.
     */
    virtual std::pair<sk_sp<SkImageFilter>, bool> asImageFilter(const SkMatrix& ctm,
                                                                const SkPaint& paint) const;

    static SkFlattenable::Type GetFlattenableType() {
        return kSkMaskFilter_Type;
    }

    SkFlattenable::Type getFlattenableType() const override {
        return kSkMaskFilter_Type;
    }

protected:
    SkMaskFilterBase() {}

    enum class FilterReturn {
        kFalse,
        kTrue,
        kUnimplemented,
    };

    class NinePatch final : ::SkNoncopyable {
    public:
        NinePatch(const SkMask& mask, SkIRect outerRect, SkIPoint center, SkCachedData* cache)
            : fMask(mask), fOuterRect(outerRect), fCenter(center), fCache(cache) {}
        NinePatch(NinePatch&&) = delete;  // the transfer of fCache makes this not work
        ~NinePatch();

        SkMask      fMask;      // fBounds must have [0,0] in its top-left
        SkIRect     fOuterRect; // width/height must be >= fMask.fBounds'
        SkIPoint    fCenter;    // identifies center row/col for stretching
        SkCachedData* fCache = nullptr;
    };

    /**
     *  As an optimization, some filters can be applied to a smaller nine-patch
     *  instead of the full-sized rectangle. These nine-patches are not only smaller,
     *  but more re-usable/cacheable. Then, when drawing/blitting, the ninepatch
     *  can be expanded to the desired size.
     *
     *  Override if your subclass can filter a rect, and return the answer as
     *  a ninepatch mask to be stretched over the returned outerRect. On success
     *  return FilterReturn::kTrue. On failure (e.g. out of memory) return
     *  FilterReturn::kFalse. If the normal filterMask() entry-point should be
     *  called (the default) return FilterReturn::kUnimplemented.
     *
     *  By convention, the caller will take the center rol/col from the returned
     *  mask as the slice it can replicate horizontally and vertically as we
     *  stretch the mask to fit inside outerRect. It is an error for outerRect
     *  to be smaller than the mask's bounds. This would imply that the width
     *  and height of the mask should be odd. This is not required, just that
     *  the caller will call mask.fBounds.centerX() and centerY() to find the
     *  strips that will be replicated.
     */
    virtual FilterReturn filterRectsToNine(SkSpan<const SkRect>,
                                           const SkMatrix&,
                                           const SkIRect& clipBounds,
                                           std::optional<NinePatch>*,
                                           SkResourceCache*) const;
    /**
     *  Similar to filterRectsToNine, except it performs the work on a round rect.
     */
    virtual std::optional<NinePatch> filterRRectToNine(const SkRRect&,
                                                       const SkMatrix&,
                                                       const SkIRect& clipBounds,
                                                       SkResourceCache*) const;

private:
    friend class SkDraw;
    friend class SkDrawBase;

    /** Helper method that, given a path in device space, will rasterize it into a kA8_Format mask
     and then call filterMask(). If this returns true, the specified blitter will be called
     to render that mask. Returns false if filterMask() returned false.
     This method is not exported to java.
     */
    bool filterPath(const SkPath& devPath,
                    const SkMatrix& ctm,
                    const SkRasterClip&,
                    SkBlitter*,
                    SkStrokeRec::InitStyle,
                    SkResourceCache*) const;

    /** Helper method that, given a roundRect in device space, will rasterize it into a kA8_Format
     mask and then call filterMask(). If this returns true, the specified blitter will be called
     to render that mask. Returns false if filterMask() returned false.
     */
    bool filterRRect(const SkRRect& devRRect,
                     const SkMatrix& ctm,
                     const SkRasterClip&,
                     SkBlitter*,
                     SkResourceCache*) const;
};

inline SkMaskFilterBase* as_MFB(SkMaskFilter* mf) {
    return static_cast<SkMaskFilterBase*>(mf);
}

inline const SkMaskFilterBase* as_MFB(const SkMaskFilter* mf) {
    return static_cast<const SkMaskFilterBase*>(mf);
}

inline const SkMaskFilterBase* as_MFB(const sk_sp<SkMaskFilter>& mf) {
    return static_cast<SkMaskFilterBase*>(mf.get());
}

// For RegisterFlattenables access to the blur mask filter implementation
extern void sk_register_blur_maskfilter_createproc();

#endif
