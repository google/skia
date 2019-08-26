/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageFilter_Base_DEFINED
#define SkImageFilter_Base_DEFINED

#include "include/core/SkColorSpace.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkImageInfo.h"
#include "include/private/SkTArray.h"

#include "src/core/SkImageFilterTypes.h"

class GrFragmentProcessor;
class GrRecordingContext;

// True base class that all SkImageFilter implementations need to extend from. This provides the
// actual API surface that Skia will use to compute the filtered images.
class SkImageFilter_Base : public SkImageFilter {
public:
    SK_USE_FLUENT_IMAGE_FILTER_TYPES_IN_CLASS

    // DEPRECATED - Use skif::Context directly.
    using Context = skif::Context;

    /**
     *  Request a new filtered image to be created from the src image. The returned skif::Image
     *  provides both the pixel data and the origin point that it should be drawn at, relative to
     *  the layer space defined by the provided context.
     *
     *  If the result image cannot be created, or the result would be transparent black, returns
     *  a skif::Image that has a null special image, in which its origin should be ignored.
     *
     *  TODO: Right now the imagefilters sometimes return empty result bitmaps/
     *        specialimages. That doesn't seem quite right.
     */
    skif::FilterResult<For::kOutput> filterImage(const skif::Context& context) const;

    /**
     *  Returns whether any edges of the crop rect have been set. The crop
     *  rect is set at construction time, and determines which pixels from the
     *  input image will be processed, and which pixels in the output image will be allowed.
     *  The size of the crop rect should be
     *  used as the size of the destination image. The origin of this rect
     *  should be used to offset access to the input images, and should also
     *  be added to the "offset" parameter in onFilterImage.
     */
    bool cropRectIsSet() const { return fCropRect.flags() != 0x0; }

    CropRect getCropRect() const { return fCropRect; }

    // Expose isolated node bounds behavior for SampleImageFilterDAG and debugging
    SkIRect filterNodeBounds(const SkIRect& srcRect, const SkMatrix& ctm,
                             MapDirection dir, const SkIRect* inputRect) const {
        return this->onFilterNodeBounds(srcRect, ctm, dir, inputRect);
    }

    /**
     *  ImageFilters can natively handle scaling and translate components in the CTM. Only some of
     *  them can handle affine (or more complex) matrices. This call returns true iff the filter
     *  and all of its (non-null) inputs can handle these more complex matrices.
     */
    bool canHandleComplexCTM() const;

    /**
     * Return an image filter representing this filter applied with the given ctm. This will modify
     * the DAG as needed if this filter does not support complex CTMs and 'ctm' is not simple. The
     * ctm matrix will be decomposed such that ctm = A*B; B will be incorporated directly into the
     * DAG and A must be the ctm set on the context passed to filterImage(). 'remainder' will be set
     * to A.
     *
     * If this filter supports complex ctms, or 'ctm' is not complex, then A = ctm and B = I. When
     * the filter does not support complex ctms, and the ctm is complex, then A represents the
     * extracted simple portion of the ctm, and the complex portion is baked into a new DAG using a
     * matrix filter.
     *
     * This will never return null.
     */
    sk_sp<SkImageFilter> applyCTM(const SkMatrix& ctm, SkMatrix* remainder) const;
    /**
     * Similar to SkApplyCTMToFilter except this assumes the input content is an existing backdrop
     * image to be filtered. As such,  the input to this filter will also be transformed by B^-1 if
     * the filter can't support complex CTMs, since backdrop content is already in device space and
     * must be transformed back into the CTM's local space.
     */
    sk_sp<SkImageFilter> applyCTMForBackdrop(const SkMatrix& ctm, SkMatrix* remainder) const;

    uint32_t uniqueID() const { return fUniqueID; }

protected:
    class Common {
    public:
        /**
         *  Attempt to unflatten the cropRect and the expected number of input filters.
         *  If any number of input filters is valid, pass -1.
         *  If this fails (i.e. corrupt buffer or contents) then return false and common will
         *  be left uninitialized.
         *  If this returns true, then inputCount() is the number of found input filters, each
         *  of which may be NULL or a valid imagefilter.
         */
        bool unflatten(SkReadBuffer&, int expectedInputs);

        const CropRect& cropRect() const { return fCropRect; }
        int inputCount() const { return fInputs.count(); }
        sk_sp<SkImageFilter>* inputs() { return fInputs.begin(); }

        sk_sp<SkImageFilter> getInput(int index) { return fInputs[index]; }

    private:
        CropRect fCropRect;
        // most filters accept at most 2 input-filters
        SkSTArray<2, sk_sp<SkImageFilter>, true> fInputs;
    };

    SkImageFilter_Base(sk_sp<SkImageFilter> const* inputs, int inputCount,
                       const CropRect* cropRect);

    ~SkImageFilter_Base() override;

    void flatten(SkWriteBuffer&) const override;

    virtual bool affectsTransparentBlack() const { return false; }

    // DEPRECATED - Use the private context-only variant
    virtual sk_sp<SkSpecialImage> onFilterImage(const Context&, SkIPoint* offset) const = 0;

    /**
     * This function recurses into its inputs with the given rect (first
     * argument), calls filterBounds() with the given map direction on each,
     * and returns the union of those results. If a derived class has special
     * recursion requirements (e.g., it has an input which does not participate
     * in bounds computation), it can be overridden here.
     * In kReverse mode, 'inputRect' is the device-space bounds of the input pixels. In kForward
     * mode it should always be null. If 'inputRect' is null in kReverse mode the resulting
     * answer may be incorrect.
     *
     * Note that this function is *not* responsible for mapping the rect for
     * this node's filter bounds requirements (i.e., calling
     * onFilterNodeBounds()); that is handled by filterBounds().
     */
    virtual SkIRect onFilterBounds(const SkIRect&, const SkMatrix& ctm,
                                   MapDirection, const SkIRect* inputRect) const;

    /**
     * Performs a forwards or reverse mapping of the given rect to accommodate
     * this filter's margin requirements. kForward_MapDirection is used to
     * determine the destination pixels which would be touched by filtering
     * the given source rect (e.g., given source bitmap bounds,
     * determine the optimal bounds of the filtered offscreen bitmap).
     * kReverse_MapDirection is used to determine which pixels of the
     * input(s) would be required to fill the given destination rect
     * (e.g., clip bounds). NOTE: these operations may not be the
     * inverse of the other. For example, blurring expands the given rect
     * in both forward and reverse directions. Unlike
     * onFilterBounds(), this function is non-recursive.
     * In kReverse mode, 'inputRect' will be the device space bounds of the input pixels. In
     * kForward mode, 'inputRect' should always be null. If 'inputRect' is null in kReverse mode
     * the resulting answer may be incorrect.
     */
    virtual SkIRect onFilterNodeBounds(const SkIRect&, const SkMatrix& ctm,
                                       MapDirection, const SkIRect* inputRect) const;

    /**
     *  Return true (and returns a ref'd colorfilter) if this node in the DAG is just a
     *  colorfilter w/o CropRect constraints.
     */
    virtual bool onIsColorFilterNode(SkColorFilter** /*filterPtr*/) const {
        return false;
    }

    /**
     *  Override this to describe the behavior of your subclass - as a leaf node. The caller will
     *  take care of calling your inputs (and return false if any of them could not handle it).
     */
    virtual bool onCanHandleComplexCTM() const { return false; }

    // DEPRECRATED - Call the Context-only getInputFilteredImage()
    sk_sp<SkSpecialImage> filterInput(int index, const Context& ctx, SkIPoint* offset) const {
        return this->getInputFilteredImage(index, ctx).imageAndOffset(offset);
    }

    // Helper function to help with recursing through the filter DAG. It invokes filter processing
    // set to null, it returns the dynamic source image on the Context instead.
    //
    // Implementations must handle cases when the input filter was unable to compute an image and
    // the returned skif::Image has a null SkSpecialImage. If the filter affect transparent black
    // should explicitly handle nullptr results and press on. In the error case this behavior will
    // produce a better result than nothing and is necessary for the clipped out case.
    skif::FilterResult<For::kInput> getInputFilteredImage(int index,
                                                          const skif::Context& context) const {
        return this->filterInput<For::kInput>(index, context);
    }
    // Convenience that calls filterInput with index = 0 and the most specific usage.
    skif::FilterResult<For::kInput0> getInputFilteredImage0(const skif::Context& context) const {
        return this->filterInput<For::kInput0>(0, context);
    }
    // Convenience that calls filterInput with index = 1 and the most specific usage.
    skif::FilterResult<For::kInput1> getInputFilteredImage1(const skif::Context& context) const {
        return this->filterInput<For::kInput1>(1, context);
    }

    const CropRect* getCropRectIfSet() const {
        return this->cropRectIsSet() ? &fCropRect : nullptr;
    }

    /** Given a "srcBounds" rect, computes destination bounds for this filter.
     *  "dstBounds" are computed by transforming the crop rect by the context's
     *  CTM, applying it to the initial bounds, and intersecting the result with
     *  the context's clip bounds.  "srcBounds" (if non-null) are computed by
     *  intersecting the initial bounds with "dstBounds", to ensure that we never
     *  sample outside of the crop rect (this restriction may be relaxed in the
     *  future).
     */
    bool applyCropRect(const Context&, const SkIRect& srcBounds, SkIRect* dstBounds) const;

    /** A variant of the above call which takes the original source bitmap and
     *  source offset. If the resulting crop rect is not entirely contained by
     *  the source bitmap's bounds, it creates a new bitmap in "result" and
     *  pads the edges with transparent black. In that case, the srcOffset is
     *  modified to be the same as the bounds, since no further adjustment is
     *  needed by the caller. This version should only be used by filters
     *  which are not capable of processing a smaller source bitmap into a
     *  larger destination.
     */
    sk_sp<SkSpecialImage> applyCropRectAndPad(const Context&, SkSpecialImage* src,
                                              SkIPoint* srcOffset, SkIRect* bounds) const;

    /**
     *  Creates a modified Context for use when recursing up the image filter DAG.
     *  The clip bounds are adjusted to accommodate any margins that this
     *  filter requires by calling this node's
     *  onFilterNodeBounds(..., kReverse_MapDirection).
     */
    Context mapContext(const Context& ctx) const;

#if SK_SUPPORT_GPU
    static sk_sp<SkSpecialImage> DrawWithFP(GrRecordingContext* context,
                                            std::unique_ptr<GrFragmentProcessor> fp,
                                            const SkIRect& bounds,
                                            SkColorType colorType,
                                            const SkColorSpace* colorSpace,
                                            GrProtected isProtected = GrProtected::kNo);

    /**
     *  Returns a version of the passed-in image (possibly the original), that is in a colorspace
     *  with the same gamut as the one from the OutputProperties. This allows filters that do many
     *  texture samples to guarantee that any color space conversion has happened before running.
     */
    static sk_sp<SkSpecialImage> ImageToColorSpace(SkSpecialImage* src,
                                                   SkColorType colorType,
                                                   SkColorSpace* colorSpace);
#endif

    // If 'srcBounds' will sample outside the border of 'originalSrcBounds' (i.e., the sample
    // will wrap around to the other side) we must preserve the far side of the src along that
    // axis (e.g., if we will sample beyond the left edge of the src, the right side must be
    // preserved for the repeat sampling to work).
    static SkIRect DetermineRepeatedSrcBound(const SkIRect& srcBounds,
                                             const SkIVector& filterOffset,
                                             const SkISize& filterSize,
                                             const SkIRect& originalSrcBounds);

private:
    friend class SkImageFilter;
    // For PurgeCache()
    friend class SkGraphics;

    static void PurgeCache();

    void init(sk_sp<SkImageFilter> const* inputs, int inputCount, const CropRect* cropRect);

    // Configuration points for the filter implementation, marked private since they should not
    // need to be invoked by the subclasses. These refer to the node's specific behavior and are
    // not responsible for aggregating the behavior of the entire filter DAG.

    /**
     *  This is the virtual which should be overridden by the derived class to perform image
     *  filtering. Subclasses are responsible for recursing to their input filters, although the
     *  getFilteredInputX() functions are provided to handle all necessary details of this. If the
     *  filter has a fixed number of inputs, the getFilterInput0() and getFilteredInput1() functions
     *  ensure the returned filtered Images have the most specific input usage.
     *
     *  If the image cannot be created (either because of an error or if the result would be empty
     *  because it was clipped out), this should return a filtered Image with a null SkSpecialImage.
     *  In these situations, callers that do not affect transparent black can end early, since the
     *  "transparent" implicit image would be unchanged. Callers that affect transparent black need
     *  to safely handle these null and empty images and return an image filling the context's clip
     *  bounds as if its input filtered image were transparent black.
     */
    virtual skif::FilterResult<For::kOutput> onFilterImage(const skif::Context& context) const;

    // The actual implementation of the protected getFilterInputX() functions, but don't expose the
    // flexible templating to subclasses so it can't be abused.
    template<skif::Usage kU>
    skif::FilterResult<kU> filterInput(int index, const skif::Context& ctx) const;

    SkAutoSTArray<2, sk_sp<SkImageFilter>> fInputs;

    bool fUsesSrcInput;
    CropRect fCropRect;
    uint32_t fUniqueID; // Globally unique

    typedef SkImageFilter INHERITED;
};

static inline SkImageFilter_Base* as_IFB(SkImageFilter* filter) {
    return static_cast<SkImageFilter_Base*>(filter);
}

static inline SkImageFilter_Base* as_IFB(const sk_sp<SkImageFilter>& filter) {
    return static_cast<SkImageFilter_Base*>(filter.get());
}

static inline const SkImageFilter_Base* as_IFB(const SkImageFilter* filter) {
    return static_cast<const SkImageFilter_Base*>(filter);
}

/**
 *  Helper to unflatten the common data, and return nullptr if we fail.
 */
#define SK_IMAGEFILTER_UNFLATTEN_COMMON(localVar, expectedCount)    \
    Common localVar;                                                \
    do {                                                            \
        if (!localVar.unflatten(buffer, expectedCount)) {           \
            return nullptr;                                         \
        }                                                           \
    } while (0)

#endif // SkImageFilter_Base_DEFINED
