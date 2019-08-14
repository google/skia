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

#if SK_SUPPORT_GPU
#include "include/gpu/GrTypes.h"
#endif

#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"

class GrFragmentProcessor;
class GrRecordingContext;
class SkImageFilterCache;
struct SkImageFilterCacheKey;

// True base class that all SkImageFilter implementations need to extend from. This provides the
// actual API surface that Skia will use to compute the filtered images.
class SkImageFilter_Base : public SkImageFilter {
public:
    // The context contains all necessary information to describe how the image filter should be
    // computed (i.e. the current layer matrix and clip), and the color information of the output of
    // a filter DAG. For now, this is just the color space (of the original requesting device). This
    // is used when constructing intermediate rendering surfaces, so that we ensure we land in a
    // surface that's similar/compatible to the final consumer of the DAG's output.
    class Context {
    public:
        // Creates a context with the given layer matrix and destination clip, reading from 'source'
        // with an origin of (0,0).
        Context(const SkMatrix& ctm, const SkIRect& clipBounds, SkImageFilterCache* cache,
                SkColorType colorType, SkColorSpace* colorSpace)
            : fCTM(ctm)
            , fClipBounds(clipBounds)
            , fCache(cache)
            , fColorType(colorType)
            , fColorSpace(colorSpace) {}

        const SkMatrix& ctm() const { return fCTM; }
        const SkIRect& clipBounds() const { return fClipBounds; }
        SkImageFilterCache* cache() const { return fCache; }
        // The output device's color type, which can be used for intermediate images to be
        // compatible with the eventual target of the filtered result.
        SkColorType colorType() const { return fColorType; }
#if SK_SUPPORT_GPU
        GrColorType grColorType() const { return SkColorTypeToGrColorType(fColorType); }
#endif
        // The output device's color space, so intermediate images can match, and so filtering can
        // be performed in the destination color space.
        SkColorSpace* colorSpace() const { return fColorSpace; }
        sk_sp<SkColorSpace> refColorSpace() const { return sk_ref_sp(fColorSpace); }

        /**
         *  Since a context can be built directly, its constructor has no chance to "return null" if
         *  it's given invalid or unsupported inputs. Call this to know of the the context can be
         *  used.
         *
         *  The SkImageFilterCache Key, for example, requires a finite ctm (no infinities or NaN),
         *  so that test is part of isValid.
         */
        bool isValid() const { return fCTM.isFinite(); }

        // Create a surface of the given size, that matches the context's color type and color space
        // as closely as possible, and uses the same backend of the device that produced the source
        // image.
        sk_sp<SkSpecialSurface> makeSurface(const SkSpecialImage* source, const SkISize& size,
                                            const SkSurfaceProps* props = nullptr) const {
            return source->makeSurface(fColorType, fColorSpace, size, kPremul_SkAlphaType, props);
        }

    private:
        SkMatrix               fCTM;
        SkIRect                fClipBounds;
        SkImageFilterCache*    fCache;
        SkColorType            fColorType;
        // This pointer is owned by the device controlling the filter process, and our
        // lifetime is bounded by the device, so it can be a bare pointer.
        SkColorSpace*          fColorSpace;
    };

    /**
     *  Request a new filtered image to be created from the src image.
     *
     *  The context contains the environment in which the filter is occurring.
     *  It includes the clip bounds, CTM and cache.
     *
     *  Offset is the amount to translate the resulting image relative to the
     *  src when it is drawn. This is an out-param.
     *
     *  If the result image cannot be created, or the result would be
     *  transparent black, return null, in which case the offset parameter
     *  should be ignored by the caller.
     *
     *  TODO: Right now the imagefilters sometimes return empty result bitmaps/
     *        specialimages. That doesn't seem quite right.
     */
    sk_sp<SkSpecialImage> filterImage(SkSpecialImage* src, const Context& context,
                                      SkIPoint* offset) const;

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

    /**
     *  This is the virtual which should be overridden by the derived class
     *  to perform image filtering.
     *
     *  src is the original primitive bitmap. If the filter has a connected
     *  input, it should recurse on that input and use that in place of src.
     *
     *  The matrix is the matrix used to draw the geometry into the current
     *  layer that produced the 'src' image. This may be the total canvas'
     *  matrix, or part of its decomposition (depending on what the filter DAG
     *  is able to support).
     *
     *  Offset is the amount to translate the resulting image relative to the
     *  src when it is drawn. This is an out-param.
     *
     *  If the result image cannot be created (either because of error or if, say, the result
     *  is entirely clipped out), this should return nullptr.
     *  Callers that affect transparent black should explicitly handle nullptr
     *  results and press on. In the error case this behavior will produce a better result
     *  than nothing and is necessary for the clipped out case.
     *  If the return value is nullptr then offset should be ignored.
     */
    virtual sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* src, const Context&,
                                                SkIPoint* offset) const = 0;

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

    // Helper function which invokes filter processing on the input at the
    // specified "index". If the input is null, it returns "src" and leaves
    // "offset" untouched. If the input is non-null, it
    // calls filterImage() on that input, and returns the result.
    sk_sp<SkSpecialImage> filterInput(int index,
                                      SkSpecialImage* src,
                                      const Context&,
                                      SkIPoint* offset) const;

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
