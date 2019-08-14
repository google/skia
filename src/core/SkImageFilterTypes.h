/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageFilterTypes_DEFINED
#define SkImageFilterTypes_DEFINED

#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"

class GrRecordingContext;
class SkImageFilterCache;
class SkSpecialSurface;
class SkSurfaceProps;

// The skif (SKI[mage]F[ilter]) namespace contains types that are used for filter implementations.
// The defined types come in two groups: users of internal Skia types, and templates to help with
// readability. Image filters cannot be implemented without access to key internal types, such as
// SkSpecialImage. It is possible to avoid the use of the readability templates, although they are
// strongly encouraged.
namespace skif {

// Usage is a template tag to improve the readability of filter implementations. It is can be
// attached to images and geometry to group a collection of related variables and ensure that moving
// from one use case to another is explicit.
// NOTE: This can be aliased as 'For' when using the fluent type names.
enum class Usage {
    // Designates the semantic purpose of the bounds, coordinate, or image as being an input
    // to the image filter calculations. When this usage is used, it denotes a generic input,
    // such as the current input in a dynamic loop, or some aggregate of all inputs. Because
    // most image filters consume 1 or 2 filters only, the related kInput0 and kInput1 are
    // also defined.
    kInput,
    // A more specific version of kInput, this marks the tagged variable as attached to the
    // image filter of SkImageFilter_Base::getInput(0).
    kInput0,
    // A more specific version of kInput, this marks the tagged variable as attached to the
    // image filter of SkImageFilter_Base::getInput(1).
    kInput1,
    // Designates the purpose of the bounds, coordinate, or image as being the output of the
    // current image filter calculation. There is only ever one output for an image filter.
    kOutput,
};

// Convenience macros to add 'using' declarations that rename the above enums to provide a more
// fluent and readable API. This should only be used in a private or local scope to prevent leakage
// of the names. Use the IN_CLASS variant at the start of a class declaration in those scenarios.
// These macros enable the following simpler type names:
//   skif::Image<skif::Usage::kInput> -> Image<For::kInput>
#define SK_USE_FLUENT_IMAGE_FILTER_TYPES \
    using For = skif::Usage;

#define SK_USE_FLUENT_IMAGE_FILTER_TYPES_IN_CLASS \
    protected: SK_USE_FLUENT_IMAGE_FILTER_TYPES public:

// Wraps an SkSpecialImage and tags it with a corresponding usage, either as generic input (e.g. the
// source image), or a specific input image from a filter's connected inputs. It also includes the
// origin of the image in the layer space. This origin is used to draw the image in the correct
// location. The 'layerBounds' rectangle of the filtered image is the layer-space bounding box of
// the image. This has its top left corner at the 'origin' and has the same dimensions as the
// underlying special image subset. Transforming 'layerBounds' by the Context's layer matrix and
// painting it with the subset image will display the filtered image in the appropriate device-space
// region.
//
// When filter implementations are processing intermediate Image results, it can be assumed
// that all Images' layerBounds are in the same layer coordinate space defined by the shared
// skif::Context.
template<Usage kU>
class Image {
public:
    Image() : fImage(nullptr)
            , fOrigin({0, 0}) {}

    Image(sk_sp<SkSpecialImage> image, const SkIPoint& origin)
            : fImage(std::move(image))
            , fOrigin(origin) {}

    // Allow explicit moves/copies in order to cast from one use type to another, except kInput0
    // and kInput1 can only be cast to kOutput (e.g. as part of a noop image filter).
    template<Usage kI>
    explicit Image(Image<kI>&& image)
            : fImage(std::move(image.fImage))
            , fOrigin(std::move(image.fOrigin)) {
        static_assert((kU != Usage::kInput) || (kI != Usage::kInput0 && kI != Usage::kInput1),
                      "kInput0 and kInput1 cannot be moved to more generic kInput usage.");
        static_assert((kU != Usage::kInput0 && kU != Usage::kInput1) ||
                      (kI == kU || kI == Usage::kInput || kI == Usage::kOutput),
                      "Can only move to specific input from the generic kInput or kOutput usage.");
    }

    template<Usage kI>
    explicit Image(const Image<kI>& image)
            : fImage(image.fImage)
            , fOrigin(image.fOrigin) {
        static_assert((kU != Usage::kInput) || (kI != Usage::kInput0 && kI != Usage::kInput1),
                      "kInput0 and kInput1 cannot be copied to more generic kInput usage.");
        static_assert((kU != Usage::kInput0 && kU != Usage::kInput1) ||
                      (kI == kU || kI == Usage::kInput || kI == Usage::kOutput),
                      "Can only copy to specific input from the generic kInput usage.");
    }

    const SkSpecialImage* image() const { return fImage.get(); }
    sk_sp<SkSpecialImage> refImage() const { return fImage; }

    // TODO (michaelludwig) - the geometry types will be updated to tag the coordinate system
    // associated with each one.

    // Get the subset bounds of this image within its backing proxy. This will have the same
    // dimensions as the image.
    const SkIRect& subset() const { return fImage->subset(); }

    // Get the layer-space bounds of this image. This will have the same dimensions as the
    // image and its top left corner will be 'origin()'.
    const SkIRect& layerBounds() const {
        return SkIRect::MakeXYWH(fOrigin.x(), fOrigin.y(), fImage->width(), fImage->height());
    }

    // Get the layer-space coordinate of this image's top left pixel.
    const SkIPoint& origin() const { return fOrigin; }

    // Extract image and origin, safely when the image is null.
    // TODO (michaelludwig) - This is intended for convenience until all call sites of
    // SkImageFilter_Base::filterImage() have been updated to work in the new type system
    // (which comes later as SkDevice, SkCanvas, etc. need to be modified, and coordinate space
    // tagging needs to be added).
    sk_sp<SkSpecialImage> imageAndOffset(SkIPoint* offset) const {
        if (fImage) {
            *offset = fOrigin;
            return fImage;
        } else {
            *offset = {0, 0};
            return nullptr;
        }
    }

private:
    // Allow all Image templates access to each others members
    template<Usage kO>
    friend class Image;

    sk_sp<SkSpecialImage> fImage;
    SkIPoint fOrigin;
};

// The context contains all necessary information to describe how the image filter should be
// computed (i.e. the current layer matrix and clip), and the color information of the output of a
// filter DAG. For now, this is just the color space (of the original requesting device). This is
// used when constructing intermediate rendering surfaces, so that we ensure we land in a surface
// that's similar/compatible to the final consumer of the DAG's output.
class Context {
public:
    SK_USE_FLUENT_IMAGE_FILTER_TYPES_IN_CLASS

    // Creates a context with the given layer matrix and destination clip, reading from 'source'
    // with an origin of (0,0).
    Context(const SkMatrix& layerMatrix, const SkIRect& clipBounds, SkImageFilterCache* cache,
            SkColorType colorType, SkColorSpace* colorSpace, const SkSpecialImage* source)
        : fLayerMatrix(layerMatrix)
        , fClipBounds(clipBounds)
        , fCache(cache)
        , fColorType(colorType)
        , fColorSpace(colorSpace)
        , fSource(sk_ref_sp(source), {0, 0}) {}

    Context(const SkMatrix& layerMatrix, const SkIRect& clipBounds, SkImageFilterCache* cache,
            SkColorType colorType, SkColorSpace* colorSpace, const Image<For::kInput>& source)
        : fLayerMatrix(layerMatrix)
        , fClipBounds(clipBounds)
        , fCache(cache)
        , fColorType(colorType)
        , fColorSpace(colorSpace)
        , fSource(source) {}

    // The transformation from the local parameter space of the filters to the layer space where
    // filtering is computed. This may or may not be the total canvas CTM, depending on the
    // matrix type of the total CTM and whether or not the filter DAG supports complex CTMs. If
    // a node returns false from canHandleComplexCTM(), layerMatrix() will be at most a scale +
    // translate matrix and any remaining matrix will be handled by the canvas after filtering
    // is finished.
    const SkMatrix& layerMatrix() const { return fLayerMatrix; }
    // DEPRECATED: Use layerMatrix() instead
    const SkMatrix& ctm() const { return this->layerMatrix(); }
    // The bounds, in the layer space, that the filtered image will be clipped to. The output
    // from filterImage() must cover these clip bounds, except in areas where it will just be
    // transparent black, in which case a smaller output image can be returned.
    const SkIRect& clipBounds() const { return fClipBounds; }
    // The cache to use when recursing through the filter DAG, in order to avoid repeated
    // calculations of the same image.
    SkImageFilterCache* cache() const { return fCache; }
    // The output device's color type, which can be used for intermediate images to be
    // compatible with the eventual target of the filtered result.
    SkColorType colorType() const { return fColorType; }
    // The output device's color space, so intermediate images can match, and so filtering can
    // be performed in the destination color space.
    SkColorSpace* colorSpace() const { return fColorSpace; }
    // DEPRECATED: Use source() instead to get both the image and its origin.
    const SkSpecialImage* sourceImage() const { return fSource.image(); }
    // The dynamic source image to use when a filter's input filter has been set to null.
    const Image<For::kInput>& source() const { return fSource; }

    // True if image filtering should occur on the GPU if possible.
    bool useGPU() const { return fSource.image()->isTextureBacked(); }
    // The recording context to use for when computing the filter with the GPU.
    GrRecordingContext* getContext() const { return fSource.image()->getContext(); }
    // The default surface properties to use when making transient surfaces during filtering.
    const SkSurfaceProps* surfaceProps() const { return &fSource.image()->props(); }

    /**
     *  Since a context can be built directly, its constructor has no chance to "return null" if
     *  it's given invalid or unsupported inputs. Call this to know of the the context can be
     *  used.
     *
     *  The SkImageFilterCache Key, for example, requires a finite ctm (no infinities or NaN),
     *  so that test is part of isValid.
     */
    bool isValid() const { return fSource.image() != nullptr && fLayerMatrix.isFinite(); }

    // Create a surface of the given size, that matches the context's color type and color space
    // as closely as possible, and uses the same backend of the device that produced the source
    // image.
    sk_sp<SkSpecialSurface> makeSurface(const SkISize& size,
                                        const SkSurfaceProps* props = nullptr) const {
        return fSource.image()->makeSurface(fColorType, fColorSpace, size,
                                            kPremul_SkAlphaType, props);
    }

    // Create a new context that matches this context, but with an overridden layer CTM matrix.
    Context withLayerMatrix(const SkMatrix& layerMatrix) const {
        return Context(layerMatrix, fClipBounds, fCache, fColorType, fColorSpace, fSource);
    }
    // Create a new context that matches this context, but with an overridden clip bounds rect.
    Context withClipBounds(const SkIRect& clipBounds) const {
        return Context(fLayerMatrix, clipBounds, fCache, fColorType, fColorSpace, fSource);
    }

private:
    SkMatrix               fLayerMatrix;
    SkIRect                fClipBounds;
    SkImageFilterCache*    fCache;
    SkColorType            fColorType;
    // This will be a pointer that is owned by the device controlling the filter process, and our
    // lifetime is bounded by the device, so it can be a bare pointer.
    SkColorSpace*          fColorSpace;
    Image<For::kInput>     fSource;
};

} // end namespace skif

#endif // SkImageFilterTypes_DEFINED
