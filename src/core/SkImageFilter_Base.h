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
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTemplates.h"

#include "src/core/SkImageFilterTypes.h"

#include <optional>

// True base class that all SkImageFilter implementations need to extend from. This provides the
// actual API surface that Skia will use to compute the filtered images.
class SkImageFilter_Base : public SkImageFilter {
public:
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
    skif::FilterResult filterImage(const skif::Context& context) const;

    /**
     * Create a filtered version of the 'src' image using this filter. This is basically a wrapper
     * around filterImage that prepares the skif::Context to filter the 'src' image directly,
     * for implementing the SkImages::MakeWithFilter API calls.
     */
    sk_sp<SkImage> makeImageWithFilter(sk_sp<skif::Backend> backend,
                                       sk_sp<SkImage> src,
                                       const SkIRect& subset,
                                       const SkIRect& clipBounds,
                                       SkIRect* outSubset,
                                       SkIPoint* offset) const;

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
     *                       that would be rasterized in the source input image. Assumes unbounded
     *                       content when not provided.
     *
     * @return The layer-space bounding box to use for an SkDevice when drawing the source image.
     */
    skif::LayerSpace<SkIRect> getInputBounds(
            const skif::Mapping& mapping,
            const skif::DeviceSpace<SkIRect>& desiredOutput,
            std::optional<skif::ParameterSpace<SkRect>> knownContentBounds) const;

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
     *  If the returned optional does not have a value, the caller should interpret this to mean
     *  that the output of the image filter will fill the entirety of whatever clipped device it's
     *  drawn into.
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
    std::optional<skif::DeviceSpace<SkIRect>> getOutputBounds(
            const skif::Mapping& mapping,
            const skif::ParameterSpace<SkRect>& contentBounds) const;

    // Returns true if this image filter graph transforms a source transparent black pixel to a
    // color other than transparent black.
    bool affectsTransparentBlack() const;

    // Returns true if this image filter graph references the Context's source image.
    bool usesSource() const { return fUsesSrcInput; }

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

    // TODO: CreateProcs for now-removed image filter subclasses need to hook into
    // SK_IMAGEFILTER_UNFLATTEN_COMMON, so this temporarily exposes it for the case where there's a
    // single input filter, and can be removed when the legacy CreateProcs are deleted.
    static std::pair<sk_sp<SkImageFilter>, std::optional<SkRect>>
    Unflatten(SkReadBuffer& buffer);

protected:
    class Common {
    public:
        /**
         *  Attempt to unflatten the expected number of input filters.
         *  If any number of input filters is valid, pass -1.
         *  If this fails (i.e. corrupt buffer or contents) then return false and common will
         *  be left uninitialized.
         *  If this returns true, then inputCount() is the number of found input filters, each
         *  of which may be NULL or a valid imagefilter.
         */
        bool unflatten(SkReadBuffer&, int expectedInputs);

        std::optional<SkRect> cropRect() const { return fCropRect; }

        int inputCount() const { return fInputs.size(); }
        sk_sp<SkImageFilter>* inputs() { return fInputs.begin(); }

        sk_sp<SkImageFilter> getInput(int index) { return fInputs[index]; }

    private:
        // Old SKPs (version less than kRemoveDeprecatedCropRect may have this set).
        std::optional<SkRect> fCropRect;

        // most filters accept at most 2 input-filters
        skia_private::STArray<2, sk_sp<SkImageFilter>, true> fInputs;
    };

    SkImageFilter_Base(sk_sp<SkImageFilter> const* inputs, int inputCount,
                       std::optional<bool> usesSrc = {});

    ~SkImageFilter_Base() override;

    void flatten(SkWriteBuffer&) const override;

    // Helper function to calculate the required input/output of a specific child filter,
    // automatically handling if the child filter is null.
    skif::LayerSpace<SkIRect> getChildInputLayerBounds(
            int index,
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& desiredOutput,
            std::optional<skif::LayerSpace<SkIRect>> contentBounds) const;
    std::optional<skif::LayerSpace<SkIRect>> getChildOutputLayerBounds(
            int index,
            const skif::Mapping& mapping,
            std::optional<skif::LayerSpace<SkIRect>> contentBounds) const;

    // Helper function for recursing through the filter DAG. It automatically evaluates the input
    // image filter at 'index' using the given context. If the input image filter is null, it
    // returns the context's dynamic source image.
    //
    // When an image filter requires a different output than what is requested in it's own Context
    // passed to onFilterImage(), it should explicitly pass in an updated Context via
    // `withNewDesiredOutput`.
    skif::FilterResult getChildOutput(int index, const skif::Context& ctx) const;

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
     *  w/o cropping constraints.
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
     * Return true if `affectsTransparentBlack()` should only be based on
     * `onAffectsTransparentBlack()` and ignore the transparency behavior of child input filters.
     */
    virtual bool ignoreInputsAffectsTransparentBlack() const { return false; }

    /**
     *  This is the virtual which should be overridden by the derived class to perform image
     *  filtering. Subclasses are responsible for recursing to their input filters, although the
     *  filterInput() function is provided to handle all necessary details of this.
     *
     *  If the image cannot be created (either because of an error or if the result would be empty
     *  because it was clipped out), this should return a filtered Image with a null SkSpecialImage.
     *  In these situations, callers that do not affect transparent black can end early, since the
     *  "transparent" implicit image would be unchanged. Callers that affect transparent black need
     *  to safely handle these null and empty images and return an image filling the context's clip
     *  bounds as if its input filtered image were transparent black.
     */
    virtual skif::FilterResult onFilterImage(const skif::Context& context) const = 0;

    /**
     *  Calculates the necessary input layer size in order for the final output of the filter to
     *  cover the desired output bounds. The provided 'desiredOutput' represents the requested
     *  input bounds for this node's parent filter node, i.e. this function answers "what does this
     *  node require for input in order to satisfy (as its own output), the input needs of its
     *  parent?".
     *
     *  'contentBounds' represents the bounds of the non-transparent content that will form the
     *  source image when the filter graph is invoked. If it's not instantiated, implementations
     *  should treat the content as extending infinitely. However, since the output is known and
     *  bounded, implementations should still be able to determine a finite input bounds under these
     *  circumstances.
     *
     *  Unlike the public getInputBounds(), all internal bounds calculations are done in the shared
     *  layer space defined by 'mapping'.
     */
    virtual skif::LayerSpace<SkIRect> onGetInputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& desiredOutput,
            std::optional<skif::LayerSpace<SkIRect>> contentBounds) const = 0;

    /**
     *  Calculates the output bounds that this filter node would touch when processing an input
     *  sized to 'contentBounds'. This function is responsible for recursing to its child image
     *  filters and accounting for what they output. It is up to the filter to determine how to
     *  aggregate the outputs of its children.
     *
     *  'contentBounds' represents the bounds of the non-transparent content that will form the
     *  source image when the filter graph is invoked. If it's not instantiated, implementations
     *  should treat the content as extending infinitely. However, since the output is known and
     *  bounded, implementations should still be able to determine a finite input bounds under these
     *  circumstances.
     *
     *  If the non-transparent output extends infinitely, subclasses should return an uninstantiated
     *  optional. Implementations must also be able to handle when their children return such
     *  unbounded "outputs" and react accordingly.
     *
     *  Unlike the public getOutputBounds(), all internal bounds calculations are done in the
     *  shared layer space defined by 'mapping'.
     */
    // TODO (michaelludwig) - When layerMatrix = I, this function could be used to implement
    // onComputeFastBounds() instead of making filters implement the essentially the same calcs x2
    virtual std::optional<skif::LayerSpace<SkIRect>> onGetOutputLayerBounds(
            const skif::Mapping& mapping,
            std::optional<skif::LayerSpace<SkIRect>> contentBounds) const = 0;

    skia_private::AutoSTArray<2, sk_sp<SkImageFilter>> fInputs;

    bool fUsesSrcInput;
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
void SkRegisterBlendImageFilterFlattenable();
void SkRegisterBlurImageFilterFlattenable();
void SkRegisterColorFilterImageFilterFlattenable();
void SkRegisterComposeImageFilterFlattenable();
void SkRegisterCropImageFilterFlattenable();
void SkRegisterDisplacementMapImageFilterFlattenable();
void SkRegisterImageImageFilterFlattenable();
void SkRegisterLightingImageFilterFlattenables();
void SkRegisterMagnifierImageFilterFlattenable();
void SkRegisterMatrixConvolutionImageFilterFlattenable();
void SkRegisterMatrixTransformImageFilterFlattenable();
void SkRegisterMergeImageFilterFlattenable();
void SkRegisterMorphologyImageFilterFlattenables();
void SkRegisterPictureImageFilterFlattenable();
void SkRegisterRuntimeImageFilterFlattenable();
void SkRegisterShaderImageFilterFlattenable();

// TODO(michaelludwig): These filters no longer have dedicated implementations, so their
// SkFlattenable create procs only need to remain to support old SkPictures.
void SkRegisterLegacyDropShadowImageFilterFlattenable();

#endif // SkImageFilter_Base_DEFINED
