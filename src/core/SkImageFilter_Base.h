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
#include "include/private/SkTemplates.h"

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
     *  Calculate the smallest-possible required layer bounds that would provide sufficient
     *  information to correctly compute the image filter for every pixel in the desired output
     *  bounds. The 'desiredOutput' is intended to represent either the root render target bounds,
     *  or the device-space bounds of the current clip. If the bounds of the content that will be
     *  drawn into the layer is known, 'knownContentBounds' should be provided, since it can be
     *  used to restrict the size of the layer if the image filter DAG does not affect transparent
     *  black.
     *
     *  The returned rect is in the layer space defined by 'mapping', so it directly represents
     *  the size and location of the SkDevice created to rasterize the content prior to invoking the
     *  image filter (assuming its CTM and basis matrix are configured to match 'mapping').
     *
     *  While this operation transforms an device-space output bounds to a layer-space input bounds,
     *  it is not necessarily the inverse of getOutputBounds(). For instance, a blur needs to have
     *  an outset margin when reading pixels at the edge (to satisfy its kernel), thus it expands
     *  its required input rect to include every pixel that contributes to the desired output rect.

     *  @param mapping       The coordinate space mapping that defines both the transformation
     *                       between local and layer, and layer to root device space, that will be
     *                       used when the filter is later invoked.
     *  @param desiredOutput The desired output boundary that needs to be covered by the filter's
     *                       output (assuming that the filter is then invoked with a suitable input)
     *  @param knownContentBounds
     *                       Optional, the known layer-space bounds of the non-transparent content
     *                       that would be rasterized in the source input image.
     *
     * @return The layer-space bounding box to use for an SkDevice when drawing the source image.
     */
    skif::LayerSpace<SkIRect> getInputBounds(
            const skif::Mapping& mapping, const skif::DeviceSpace<SkIRect>& desiredOutput,
            const skif::ParameterSpace<SkRect>* knownContentBounds) const;

    /**
     *  Calculate the device-space bounds of the output of this filter DAG, if it were to process
     *  an image layer covering the 'contentBounds'. The 'mapping' defines how the content will be
     *  transformed to layer space when it is drawn, and how the output filter image is then
     *  transformed to the final device space (i.e. it specifies the mapping between the root device
     *  space and the parameter space of the initially provided content).
     *
     *  While this operation transforms a parameter-space input bounds to an device-space output
     *  bounds, it is not necessarily the inverse of getInputBounds(). For instance, a blur needs to
     *  have an outset margin when reading pixels at the edge (to satisfy its kernel), so it will
     *  generate a result larger than its input (so that the blur is visible) and, thus, expands its
     *  output to include every pixel that it will touch.
     *
     *  @param mapping       The coordinate space mapping that defines both the transformation
     *                       between local and layer, and layer to root device space, that will be
     *                       used when the filter is later invoked.
     *  @param contentBounds The local-space bounds of the non-transparent content that would be
     *                       drawn into the source image prior to filtering with this DAG,  i.e.
     *                       the same as 'knownContentBounds' in getInputBounds().
     *
     *  @return The root device-space bounding box of the filtered image, were it applied to
     *          content contained by 'contentBounds' and then drawn with 'mapping' to the root
     *          device (w/o any additional clipping).
     */
    skif::DeviceSpace<SkIRect> getOutputBounds(
            const skif::Mapping& mapping, const skif::ParameterSpace<SkRect>& contentBounds) const;

    // Returns true if this image filter graph transforms a source transparent black pixel to a
    // color other than transparent black.
    bool affectsTransparentBlack() const;

    /**
     *  Most ImageFilters can natively handle scaling and translate components in the CTM. Only
     *  some of them can handle affine (or more complex) matrices. Some may only handle translation.
     *  This call returns the maximum "kind" of CTM for a filter and all of its (non-null) inputs.
     */
    enum class MatrixCapability {
        kTranslate,
        kScaleTranslate,
        kComplex,
    };
    MatrixCapability getCTMCapability() const;

    uint32_t uniqueID() const { return fUniqueID; }

    static SkFlattenable::Type GetFlattenableType() {
        return kSkImageFilter_Type;
    }

    SkFlattenable::Type getFlattenableType() const override {
        return kSkImageFilter_Type;
    }

protected:
    // DEPRECATED: Will be removed once cropping is handled by a standalone image filter
    class CropRect {
    public:
        enum CropEdge {
            kHasLeft_CropEdge   = 0x01,
            kHasTop_CropEdge    = 0x02,
            kHasWidth_CropEdge  = 0x04,
            kHasHeight_CropEdge = 0x08,
            kHasAll_CropEdge    = 0x0F,
        };
        CropRect() : fFlags(0) {}
        explicit CropRect(const SkRect* rect)
            : fRect(rect ? *rect : SkRect::MakeEmpty()), fFlags(rect ? kHasAll_CropEdge : 0x0) {}

        // CropRect(const CropRect&) = default;

        uint32_t flags() const { return fFlags; }
        const SkRect& rect() const { return fRect; }

        /**
         *  Apply this cropRect to the imageBounds. If a given edge of the cropRect is not set, then
         *  the corresponding edge from imageBounds will be used. If "embiggen" is true, the crop
         *  rect is allowed to enlarge the size of the rect, otherwise it may only reduce the rect.
         *  Filters that can affect transparent black should pass "true", while all other filters
         *  should pass "false".
         *
         *  Note: imageBounds is in "device" space, as the output cropped rectangle will be, so the
         *  matrix is ignored for those. It is only applied to the cropRect's bounds.
         */
        void applyTo(const SkIRect& imageBounds, const SkMatrix& matrix, bool embiggen,
                     SkIRect* cropped) const;

    private:
        SkRect fRect;
        uint32_t fFlags;
    };

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

        const SkRect* cropRect() const {
            return fCropRect.flags() != 0x0 ? &fCropRect.rect() : nullptr;
        }
        int inputCount() const { return fInputs.count(); }
        sk_sp<SkImageFilter>* inputs() { return fInputs.begin(); }

        sk_sp<SkImageFilter> getInput(int index) { return fInputs[index]; }

    private:
        CropRect fCropRect;
        // most filters accept at most 2 input-filters
        SkSTArray<2, sk_sp<SkImageFilter>, true> fInputs;
    };

    // Whether or not to recurse to child input filters for certain operations that walk the DAG.
    enum class VisitChildren : bool {
        kNo  = false,
        kYes = true
    };

    SkImageFilter_Base(sk_sp<SkImageFilter> const* inputs, int inputCount,
                       const SkRect* cropRect);

    ~SkImageFilter_Base() override;

    void flatten(SkWriteBuffer&) const override;

    // DEPRECATED - Use the private context-only variant
    virtual sk_sp<SkSpecialImage> onFilterImage(const Context&, SkIPoint* offset) const = 0;

    // DEPRECATED - Override onGetOutputLayerBounds and onGetInputLayerBounds instead. The
    // node-specific and aggregation functions are no longer separated in the current API. A helper
    // function is provided to do the default recursion for the common filter case.
    virtual SkIRect onFilterBounds(const SkIRect&, const SkMatrix& ctm,
                                   MapDirection, const SkIRect* inputRect) const;
    virtual SkIRect onFilterNodeBounds(const SkIRect&, const SkMatrix& ctm,
                                       MapDirection, const SkIRect* inputRect) const;

    // DEPRECRATED - Call the Context-only getInputFilteredImage()
    sk_sp<SkSpecialImage> filterInput(int index, const Context& ctx, SkIPoint* offset) const {
        return this->getInputFilteredImage(index, ctx).imageAndOffset(offset);
    }

    // Helper function to visit each of this filter's child filters and call their
    // onGetInputLayerBounds with the provided 'desiredOutput' and 'contentBounds'. Automatically
    // handles null input filters. Returns the union of all of the children's input bounds.
    skif::LayerSpace<SkIRect> visitInputLayerBounds(
            const skif::Mapping& mapping, const skif::LayerSpace<SkIRect>& desiredOutput,
            const skif::LayerSpace<SkIRect>& contentBounds) const;
    // Helper function to visit each of this filter's child filters and call their
    // onGetOutputLayerBounds with the provided 'contentBounds'. Automatically handles null input
    // filters.
    skif::LayerSpace<SkIRect> visitOutputLayerBounds(
            const skif::Mapping& mapping, const skif::LayerSpace<SkIRect>& contentBounds) const;

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

    /**
     *  Returns whether any edges of the crop rect have been set. The crop
     *  rect is set at construction time, and determines which pixels from the
     *  input image will be processed, and which pixels in the output image will be allowed.
     *  The size of the crop rect should be
     *  used as the size of the destination image. The origin of this rect
     *  should be used to offset access to the input images, and should also
     *  be added to the "offset" parameter in onFilterImage.
     *
     *  DEPRECATED - Remove once cropping is handled by a separate filter
     */
    bool cropRectIsSet() const { return fCropRect.flags() != 0x0; }

    // DEPRECATED - Remove once cropping is handled by a separate filter
    CropRect getCropRect() const { return fCropRect; }

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
     *
     *  DEPRECATED - Remove once cropping is handled by a separate filter, although it may be
     *  necessary to provide a similar convenience function to compute the output bounds given the
     *  images returned by filterInput().
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
     *
     *  DEPRECATED - Remove once cropping is handled by a separate filter.
     */
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
                                            const SkSurfaceProps&,
                                            GrProtected isProtected = GrProtected::kNo);

    /**
     *  Returns a version of the passed-in image (possibly the original), that is in a colorspace
     *  with the same gamut as the one from the OutputProperties. This allows filters that do many
     *  texture samples to guarantee that any color space conversion has happened before running.
     */
    static sk_sp<SkSpecialImage> ImageToColorSpace(SkSpecialImage* src,
                                                   SkColorType colorType,
                                                   SkColorSpace* colorSpace,
                                                   const SkSurfaceProps&);
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

    // Configuration points for the filter implementation, marked private since they should not
    // need to be invoked by the subclasses. These refer to the node's specific behavior and are
    // not responsible for aggregating the behavior of the entire filter DAG.

    /**
     *  Return true (and returns a ref'd colorfilter) if this node in the DAG is just a colorfilter
     *  w/o CropRect constraints.
     */
    virtual bool onIsColorFilterNode(SkColorFilter** /*filterPtr*/) const { return false; }

    /**
     *  Return the most complex matrix type this filter can support (mapping from its parameter
     *  space to a layer space). If this returns anything less than kComplex, the filter only needs
     *  to worry about mapping from parameter to layer using a matrix that is constrained in that
     *  way (eg, scale+translate).
     */
    virtual MatrixCapability onGetCTMCapability() const {
        return MatrixCapability::kScaleTranslate;
    }

    /**
     *  Return true if this filter would transform transparent black pixels to a color other than
     *  transparent black. When false, optimizations can be taken to discard regions known to be
     *  transparent black and thus process fewer pixels.
     */
    virtual bool onAffectsTransparentBlack() const { return false; }

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

    /**
     *  Calculates the necessary input layer size in order for the final output of the filter to
     *  cover the desired output bounds. The provided 'desiredOutput' represents the requested
     *  input bounds for this node's parent filter node, i.e. this function answers "what does this
     *  node require for input in order to satisfy (as its own output), the input needs of its
     *  parent?".
     *
     *  If 'recurse' is true, this function is responsible for recursing to its child image filters
     *  and accounting for what they require to meet this filter's input requirements. It is up to
     *  the filter to determine how to aggregate these inputs, but a helper function is provided for
     *  the common case where the final required layer size is the union of the child filters'
     *  required inputs, evaluated on what this filter requires for itself. 'recurse' is kNo
     *  when mapping Contexts while actually filtering images, since the child recursion is
     *  happening at a higher level.
     *
     *  Unlike the public getInputBounds(), all internal bounds calculations are done in the shared
     *  layer space defined by 'mapping'.
     *
     *  The default implementation assumes that current filter requires an input equal to
     *  'desiredOutputBounds', and passes this down to its child filters, and returns the union of
     *  their required inputs.
     */
    virtual skif::LayerSpace<SkIRect> onGetInputLayerBounds(
            const skif::Mapping& mapping, const skif::LayerSpace<SkIRect>& desiredOutput,
            const skif::LayerSpace<SkIRect>& contentBounds,
            VisitChildren recurse = VisitChildren::kYes) const;

    /**
     *  Calculates the output bounds that this filter node would touch when processing an input
     *  sized to 'contentBounds'. This function is responsible for recursing to its child image
     *  filters and accounting for what they output. It is up to the filter to determine how to
     *  aggregate the outputs of its children, but a helper function is provided for the common
     *  case where the filter output is the union of its child outputs.
     *
     *  Unlike the public getOutputBounds(), all internal bounds calculations are done in the
     *  shared layer space defined by 'mapping'.
     *
     *  The default implementation assumes that the output of this filter is equal to the union of
     *  the outputs of its child filters evaluated with 'contentBounds'.
     */
    // TODO (michaelludwig) - When layerMatrix = I, this function could be used to implement
    // onComputeFastBounds() instead of making filters implement the essentially the same calcs x2
    virtual skif::LayerSpace<SkIRect> onGetOutputLayerBounds(
            const skif::Mapping& mapping, const skif::LayerSpace<SkIRect>& contentBounds) const;

    // The actual implementation of the protected getFilterInputX() functions, but don't expose the
    // flexible templating to subclasses so it can't be abused.
    template<skif::Usage kU>
    skif::FilterResult<kU> filterInput(int index, const skif::Context& ctx) const;

    SkAutoSTArray<2, sk_sp<SkImageFilter>> fInputs;

    bool fUsesSrcInput;
    CropRect fCropRect;
    uint32_t fUniqueID; // Globally unique

    using INHERITED = SkImageFilter;
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


/**
 * All image filter implementations defined for the include/effects/SkImageFilters.h factories
 * are entirely encapsulated within their own CPP files. SkFlattenable deserialization needs a hook
 * into these types, so their registration functions are exposed here.
 */
void SkRegisterAlphaThresholdImageFilterFlattenable();
void SkRegisterArithmeticImageFilterFlattenable();
void SkRegisterBlendImageFilterFlattenable();
void SkRegisterBlurImageFilterFlattenable();
void SkRegisterColorFilterImageFilterFlattenable();
void SkRegisterComposeImageFilterFlattenable();
void SkRegisterDisplacementMapImageFilterFlattenable();
void SkRegisterDropShadowImageFilterFlattenable();
void SkRegisterImageImageFilterFlattenable();
void SkRegisterLightingImageFilterFlattenables();
void SkRegisterMagnifierImageFilterFlattenable();
void SkRegisterMatrixConvolutionImageFilterFlattenable();
void SkRegisterMergeImageFilterFlattenable();
void SkRegisterMorphologyImageFilterFlattenables();
void SkRegisterOffsetImageFilterFlattenable();
void SkRegisterPictureImageFilterFlattenable();
#ifdef SK_ENABLE_SKSL
void SkRegisterRuntimeImageFilterFlattenable();
#endif
void SkRegisterShaderImageFilterFlattenable();
void SkRegisterTileImageFilterFlattenable();

#endif // SkImageFilter_Base_DEFINED
