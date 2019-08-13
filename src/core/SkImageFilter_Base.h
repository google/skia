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
    // SK_USE_FLUENT_IMAGE_FILTER_TYPES_IN_CLASS
    using For = skif::Usage;
    using In  = skif::CoordSpace;
    // Help with backwards compatibility.
    // DEPRECATED: Use skif::Context directly.
    using Context = skif::Context;

    /**
     *  Request a new filtered image to be created, based off of the context that defines the
     *  inputs to the filtering, the coordinate space that filtering occurs in, and the output
     *  parameters of the filtering.
     *
     *  If the result image cannot be created, or the result would be transparent black, return
     *  null, in which case the offset parameter should be ignored by the caller.
     *
     *  TODO: Right now the imagefilters sometimes return empty result bitmaps/
     *        specialimages. That doesn't seem quite right.
     */
    skif::Image<For::kOutput> filterImage(const skif::Context& context) const;

    /**
     *  Calculate the required layer bounds that would provide sufficient information to correctly
     *  compute the image filter for every pixel in the target output bounds, where ‘target’ is
     *  likely defined by the current clip.
     *
     *  This maps the target output bounds in reverse through the entire filter DAG. Image filters
     *  must override the private onFilterLayerBounds() to specify the input bounds that the
     *  particular node requires in order to cover the target. Note also that both 'target' and (if
     *  provided) 'originalInput' have already been transformed into the layer space defined by the
     *  'layer' matrix. The layer matrix is provided so that the filter parameters can be mapped
     *  into the same layer space.
     *
     *  While this operation transforms an output bounds to an input bounds, it is not necessarily
     *  the inverse of onFilterOutputBounds(). For instance, a blur needs to have an outset margin
     *  when reading pixels at the edge (to satisfy its kernel), and it expands its output to
     *  include every pixel that had some contribution from the input content bounds.
     *
     *  @param target        The desired output boundary that should be covered by the filter's
     *                       output (assuming that the filter is then invoked with a suitable input)
     *  @param layer         The matrix defining the transformation from parameter to layer space.
     *  @param originalInput Optional, the known layer-space bounds of the nontransparent content
     *                       that would be defined in source input image, once filtering is invoked
     */
    skif::IRect<In::kLayer, For::kInput> filterLayerBounds(
            const skif::IRect<In::kLayer, For::kOutput>& target, const SkMatrix& layer,
            const skif::IRect<In::kLayer, For::kInput>* originalInput) const;

    /**
     *  Typesafe version of SkImagefilter::filterOutputBounds that is called after decomposing the
     *  total CTM into just the layer matrix that will be used to evaluate the filter.
     *
     *  This maps the input content bounds forward through the entire filter DAG. Image filters must
     *  override the private onFilterOutputBounds() to specify the output pixels that would be
     *  touched by filtering 'content'. Note that 'content' has already been transformed into the
     *  layer space defined by the 'layer' matrix. The layer matrix is provided so that the filter
     *  parameters can be mapped into the same layer space.

     *  While this operation transforms an input bounds to an output bounds, it is not necessarily
     *  the inverse of onFilterLayerBounds(). For instance, a blur needs to have an outset margin
     *  when reading pixels at the edge (to satisfy its kernel), and it expands its output to
     *  include every pixel that had some contribution from the input content bounds.
     *
     *  @param content The layer space bounds of the nontransparent content in the input image, i.e.
     *                 the same as 'originalInput' in filterLayerBounds().
     *  @param layer   The matrix defining the transformation from parameter to layer space.
     */
    skif::IRect<In::kLayer, For::kOutput> filterOutputBounds(
            const skif::IRect<In::kLayer, For::kInput>& content, const SkMatrix& layer) const;

    /**
     *  Returns whether any edges of the crop rect have been set. The crop
     *  rect is set at construction time, and determines which pixels from the
     *  input image will be processed, and which pixels in the output image will be allowed.
     *  The size of the crop rect should be
     *  used as the size of the destination image. The origin of this rect
     *  should be used to offset access to the input images, and should also
     *  be added to the "offset" parameter in onFilterImage.
     */
    // DEPRECATED - Remove once cropping is handled by a separate filter
    bool cropRectIsSet() const { return fCropRect.flags() != 0x0; }

    // DEPRECATED - Remove once cropping is handled by a separate filter
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
    // DEPRECATED - Should draw the results of filterImage() directly with the remainder matrix.
    sk_sp<SkImageFilter> applyCTM(const SkMatrix& ctm, SkMatrix* remainder) const;
    /**
     * Similar to SkApplyCTMToFilter except this assumes the input content is an existing backdrop
     * image to be filtered. As such,  the input to this filter will also be transformed by B^-1 if
     * the filter can't support complex CTMs, since backdrop content is already in device space and
     * must be transformed back into the CTM's local space.
     */
    // DEPRECATED - Should draw the results of filterImage() directly with the remainder matrix.
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

    // Subclasses that override flatten must call INHERITED::flatten()
    void flatten(SkWriteBuffer&) const override;

    // DEPRECATED - Use the private context-only variant
    virtual sk_sp<SkSpecialImage> onFilterImage(const Context&, SkIPoint* offset) const = 0;

    // DEPRECATED - Override onNodeOutputBounds and onNodeLayerBounds instead for isolated node
    // behavior, or onFilterOutputBounds/onFilterLayerBounds for aggregate behavior.
    virtual SkIRect onFilterNodeBounds(const SkIRect&, const SkMatrix& ctm,
                                       MapDirection, const SkIRect* inputRect) const;

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
    skif::Image<For::kInput> getInputFilteredImage(int index, const skif::Context& context) const {
        return this->filterInput<For::kInput>(index, context);
    }
    // Convenience that calls filterInput with index = 0 and the most specific usage.
    skif::Image<For::kInput0> getInputFilteredImage0(const skif::Context& context) const {
        return this->filterInput<For::kInput0>(0, context);
    }
    // Convenience that calls filterInput with index = 1 and the most specific usage.
    skif::Image<For::kInput1> getInputFilteredImage1(const skif::Context& context) const {
        return this->filterInput<For::kInput1>(1, context);
    }

    // DEPRECATED - Remove once cropping is handled by a separate filter
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
    // DEPRECATED - Remove once cropping is handled by a separate filter, although it may be
    // necessary to provide a similar convenience function to compute the output bounds given the
    // images returned by filterInput().
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
    // DEPRECATED - Remove once cropping is handled by a separate filter.
    sk_sp<SkSpecialImage> applyCropRectAndPad(const Context&, SkSpecialImage* src,
                                              SkIPoint* srcOffset, SkIRect* bounds) const;

    /**
     *  Creates a modified Context for use when recursing up the image filter DAG.
     *  The clip bounds are adjusted to accommodate any margins that this
     *  filter requires by calling this node's
     *  onFilterNodeBounds(..., kReverse_MapDirection).
     */
    // TODO (michaelludwig) - I don't think this is necessary to keep as protected. Other than the
    // real use case in recursing through the DAG for filterInput(), it feels wrong for blur and
    // other filters to need to call it.
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
    // DEPRECATED - Remove once cropping is handled by a separate filter, that can also handle all
    // tile modes (including repeat) properly
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

    // The actual implementation of the protected getFilterInputX() functions, but don't expose the
    // flexible templating to subclasses so it can't be abused.
    template<skif::Usage kU>
    skif::Image<kU> filterInput(int index, const skif::Context& ctx) const;

    // Configuration points for the filter implementation, marked private since they should not
    // need to be invoked by the subclasses. These refer to the node's specific behavior and are
    // not responsible for aggregating the behavior of the entire filter DAG.

    /**
     * Return true if this filter can map from its parameter space to a layer space described by an
     * arbitrary transformation matrix. If this returns false, the filter only needs to worry about
     * mapping from parameter to layer using a scale+translate matrix.
     */
    virtual bool onCanHandleComplexCTM() const { return false; }

    /**
     * Return true (and returns a ref'd colorfilter) if this node in the DAG is just a colorfilter
     *  w/o CropRect constraints.
     */
    virtual bool onIsColorFilterNode(SkColorFilter** /*filterPtr*/) const { return false; }

    /**
     * Return true if this filter would transform transparent black pixels to a color other than
     * transparent black. When false, optimizations can be taken to discard regions known to be
     * transparent black and thus process fewer pixels.
     */
    virtual bool affectsTransparentBlack() const { return false; }

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
    virtual skif::Image<For::kOutput> onFilterImage(const skif::Context& context) const {
        // Default to using the old onFilterImage, as filters are updated one by one.
        SkIPoint origin;
        auto image = this->onFilterImage(context, &origin);
        return skif::Image<For::kOutput>(std::move(image),
                                         skif::IPoint<In::kLayer, For::kOutput>(origin));
    }

    /**
     *  Calculates the output bounds that this filter node would touch when processing an input
     *  sized to 'contentBounds'. This function is only responsible for specifying this node's
     *  output behavior. The provided content bounds is the union of the output bounds from each
     *  of this filter's inputs, as determined by onFilterOutputBounds(). If more complex
     *  aggregation logic is needed for a filters inputs, override onFilterOutputBounds() instead of
     *  onNodeOutputBounds() to take full control over recursing up the DAG.
     *
     *  The default implementation assumes that the output matches the input.
     */
    // TODO (michaelludwig) - When layerMatrix = I, this function could be used to implement
    // onComputeFastBounds() instead of making filters implement the essentially the same calcs x2
    virtual skif::IRect<In::kLayer, For::kOutput> onNodeOutputBounds(
            const skif::IRect<In::kLayer, For::kInput>& contentBounds,
            const SkMatrix& layerMatrix) const {
        // return skif::LayerCast<For::kOutput>(contentBounds);
        SkIRect output = this->onFilterNodeBounds(SkIRect(contentBounds), layerMatrix,
                                              kForward_MapDirection, nullptr);
        return skif::IRect<In::kLayer, For::kOutput>(output);
    }

    /**
     *  Similar to onNodeOutputBounds() but is responsible for determining the output that includes
     *  the effects of the child input filters. The default implementation invokes
     *  filterOutputBounds() on each child, and passes the union of those output bounds as the net
     *  input into this filter's onNodeOutputBounds() function.
     *
     *  Only one of onNodeOutputBounds() or onFilterOutputBounds() should be overridden, and in
     *  most cases onNodeOutputBounds is sufficient.
     */
    virtual skif::IRect<In::kLayer, For::kOutput> onFilterOutputBounds(
            const skif::IRect<In::kLayer, For::kInput>& contentBounds,
            const SkMatrix& layerMatrix) const;
    /**
     *  Calculates the necessary input layer size in order for the final output of the filter to
     *  cover the target output bounds. This function is only responsible for specifying this node's
     *  behavior. The provided 'targetOutputBounds' represents the requested input bounds for this node's
     *  parent filter node, i.e. this function answers "what does this node require for input in
     *  order to satisfy (as its own output), the input needs of its parent?". The rectangle this
     *  function returns is then passed to each child input filter to determine the final input
     *  size, as determined by onFilterLayerBounds(). If more complex logic is needed, override
     *  onFilterLayerBounds() to take full control over recursing up the DAG.
     *
     *  The default implementation assumes that the necessary input size matches the output.
     */
    virtual skif::IRect<In::kLayer, For::kInput> onNodeLayerBounds(
            const skif::IRect<In::kLayer, For::kOutput>& targetOutputBounds,
            const SkMatrix& layerMatrix,
            const skif::IRect<In::kLayer, For::kInput>& originalInput) const;

    /**
     *  Similar to onNodeLayerBounds() but is responsible for determining the input suitable for
     *  this filter's children as well. The default implementation invokes onNodeLayerBounds() on
     *  this filter, and then passes that to each child's filterLayerBounds() function, returning
     *  the union of those rectangles. (Or simple the result of onNodeLayerBounds() if the filter
     *  has no children).
     *
     *  Only one of onNodeLayerBounds() or onFilterLayerBounds() should be overridden, and in
     *  most cases onNodeLayerBounds is sufficient.
     */
    virtual skif::IRect<In::kLayer, For::kInput> onFilterLayerBounds(
            const skif::IRect<In::kLayer, For::kOutput>& targetOutputBounds,
            const SkMatrix& layerMatrix,
            const skif::IRect<In::kLayer, For::kInput>& originalInput) const;

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
