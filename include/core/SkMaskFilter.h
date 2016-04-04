
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkMaskFilter_DEFINED
#define SkMaskFilter_DEFINED

#include "SkBlurTypes.h"
#include "SkFlattenable.h"
#include "SkMask.h"
#include "SkPaint.h"

class GrClip;
class GrDrawContext;
class GrPaint;
class GrRenderTarget;
class GrTextureProvider;
class SkBitmap;
class SkBlitter;
class SkCachedData;
class SkMatrix;
class SkPath;
class SkRasterClip;
class SkRRect;
class SkStrokeRec;

/** \class SkMaskFilter

    SkMaskFilter is the base class for object that perform transformations on
    an alpha-channel mask before drawing it. A subclass of SkMaskFilter may be
    installed into a SkPaint. Once there, each time a primitive is drawn, it
    is first scan converted into a SkMask::kA8_Format mask, and handed to the
    filter, calling its filterMask() method. If this returns true, then the
    new mask is used to render into the device.

    Blur and emboss are implemented as subclasses of SkMaskFilter.
*/
class SK_API SkMaskFilter : public SkFlattenable {
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
    virtual bool filterMask(SkMask* dst, const SkMask& src, const SkMatrix&,
                            SkIPoint* margin) const;

#if SK_SUPPORT_GPU
    /**
     *  Returns true if the filter can be expressed a single-pass GrProcessor without requiring an
     *  explicit input mask. Per-pixel, the effect receives the incoming mask's coverage as
     *  the input color and outputs the filtered covereage value. This means that each pixel's
     *  filtered coverage must only depend on the unfiltered mask value for that pixel and not on
     *  surrounding values.
     *
     * If effect is non-NULL, a new GrProcessor instance is stored in it. The caller assumes
     * ownership of the effect and must unref it.
     */
    virtual bool asFragmentProcessor(GrFragmentProcessor**, GrTexture*, const SkMatrix& ctm) const;

    /**
     *  If asFragmentProcessor() fails the filter may be implemented on the GPU by a subclass
     *  overriding filterMaskGPU (declared below). That code path requires constructing a
     *  src mask as input. Since that is a potentially expensive operation, the subclass must also
     *  override this function to indicate whether filterTextureMaskGPU would succeeed if the mask
     *  were to be created.
     *
     *  'maskRect' returns the device space portion of the mask that the filter needs. The mask
     *  passed into 'filterMaskGPU' should have the same extent as 'maskRect' but be
     *  translated to the upper-left corner of the mask (i.e., (maskRect.fLeft, maskRect.fTop)
     *  appears at (0, 0) in the mask).
     *
     * Logically, how this works is:
     *    canFilterMaskGPU is called
     *    if (it returns true)
     *        the returned mask rect is used for quick rejecting
     *        either directFilterMaskGPU or directFilterRRectMaskGPU is then called
     *        if (neither of them handle the blur)
     *            the mask rect is used to generate the mask
     *            filterMaskGPU is called to filter the mask
     *
     * TODO: this should work as:
     *    if (canFilterMaskGPU(devShape, ...)) // rect, rrect, drrect, path
     *        filterMaskGPU(devShape, ...)
     * this would hide the RRect special case and the mask generation
     */
    virtual bool canFilterMaskGPU(const SkRRect& devRRect,
                                  const SkIRect& clipBounds,
                                  const SkMatrix& ctm,
                                  SkRect* maskRect) const;

    /**
     *  Try to directly render the mask filter into the target.  Returns
     *  true if drawing was successful.
     */
    virtual bool directFilterMaskGPU(GrTextureProvider* texProvider,
                                     GrDrawContext* drawContext,
                                     GrPaint* grp,
                                     const GrClip&,
                                     const SkMatrix& viewMatrix,
                                     const SkStrokeRec& strokeRec,
                                     const SkPath& path) const;
    /**
     *  Try to directly render a rounded rect mask filter into the target.  Returns
     *  true if drawing was successful.
     */
    virtual bool directFilterRRectMaskGPU(GrTextureProvider* texProvider,
                                          GrDrawContext* drawContext,
                                          GrPaint* grp,
                                          const GrClip&,
                                          const SkMatrix& viewMatrix,
                                          const SkStrokeRec& strokeRec,
                                          const SkRRect& rrect) const;

    /**
     * This function is used to implement filters that require an explicit src mask. It should only
     * be called if canFilterMaskGPU returned true and the maskRect param should be the output from
     * that call. canOverwriteSrc indicates whether the implementation may treat src as a scratch
     * texture and overwrite its contents. When true it is also legal to return src as the result.
     * Implementations are free to get the GrContext from the src texture in order to create
     * additional textures and perform multiple passes.
     */
    virtual bool filterMaskGPU(GrTexture* src,
                               const SkMatrix& ctm,
                               const SkRect& maskRect,
                               GrTexture** result,
                               bool canOverwriteSrc) const;
#endif

    /**
     * The fast bounds function is used to enable the paint to be culled early
     * in the drawing pipeline. This function accepts the current bounds of the
     * paint as its src param and the filter adjust those bounds using its
     * current mask and returns the result using the dest param. Callers are
     * allowed to provide the same struct for both src and dest so each
     * implementation must accomodate that behavior.
     *
     *  The default impl calls filterMask with the src mask having no image,
     *  but subclasses may override this if they can compute the rect faster.
     */
    virtual void computeFastBounds(const SkRect& src, SkRect* dest) const;

    struct BlurRec {
        SkScalar        fSigma;
        SkBlurStyle     fStyle;
        SkBlurQuality   fQuality;
    };
    /**
     *  If this filter can be represented by a BlurRec, return true and (if not null) fill in the
     *  provided BlurRec parameter. If this effect cannot be represented as a BlurRec, return false
     *  and ignore the BlurRec parameter.
     */
    virtual bool asABlur(BlurRec*) const;

    SK_TO_STRING_PUREVIRT()
    SK_DEFINE_FLATTENABLE_TYPE(SkMaskFilter)

protected:
    SkMaskFilter() {}

    enum FilterReturn {
        kFalse_FilterReturn,
        kTrue_FilterReturn,
        kUnimplemented_FilterReturn
    };

    class NinePatch : ::SkNoncopyable {
    public:
        NinePatch() : fCache(nullptr) { }
        ~NinePatch();

        SkMask      fMask;      // fBounds must have [0,0] in its top-left
        SkIRect     fOuterRect; // width/height must be >= fMask.fBounds'
        SkIPoint    fCenter;    // identifies center row/col for stretching
        SkCachedData* fCache;
    };

    /**
     *  Override if your subclass can filter a rect, and return the answer as
     *  a ninepatch mask to be stretched over the returned outerRect. On success
     *  return kTrue_FilterReturn. On failure (e.g. out of memory) return
     *  kFalse_FilterReturn. If the normal filterMask() entry-point should be
     *  called (the default) return kUnimplemented_FilterReturn.
     *
     *  By convention, the caller will take the center rol/col from the returned
     *  mask as the slice it can replicate horizontally and vertically as we
     *  stretch the mask to fit inside outerRect. It is an error for outerRect
     *  to be smaller than the mask's bounds. This would imply that the width
     *  and height of the mask should be odd. This is not required, just that
     *  the caller will call mask.fBounds.centerX() and centerY() to find the
     *  strips that will be replicated.
     */
    virtual FilterReturn filterRectsToNine(const SkRect[], int count,
                                           const SkMatrix&,
                                           const SkIRect& clipBounds,
                                           NinePatch*) const;
    /**
     *  Similar to filterRectsToNine, except it performs the work on a round rect.
     */
    virtual FilterReturn filterRRectToNine(const SkRRect&, const SkMatrix&,
                                           const SkIRect& clipBounds,
                                           NinePatch*) const;

private:
    friend class SkDraw;

    /** Helper method that, given a path in device space, will rasterize it into a kA8_Format mask
     and then call filterMask(). If this returns true, the specified blitter will be called
     to render that mask. Returns false if filterMask() returned false.
     This method is not exported to java.
     */
    bool filterPath(const SkPath& devPath, const SkMatrix& ctm, const SkRasterClip&, SkBlitter*,
                    SkPaint::Style) const;

    /** Helper method that, given a roundRect in device space, will rasterize it into a kA8_Format
     mask and then call filterMask(). If this returns true, the specified blitter will be called
     to render that mask. Returns false if filterMask() returned false.
     */
    bool filterRRect(const SkRRect& devRRect, const SkMatrix& ctm, const SkRasterClip&,
                     SkBlitter*, SkPaint::Style style) const;

    typedef SkFlattenable INHERITED;
};

#endif
