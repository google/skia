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

// The context contains all necessary information to describe how the image filter should be
// computed (i.e. the current layer matrix and clip), and the color information of the output of
// a filter DAG. For now, this is just the color space (of the original requesting device). This
// is used when constructing intermediate rendering surfaces, so that we ensure we land in a
// surface that's similar/compatible to the final consumer of the DAG's output.
class Context {
public:
    // Creates a context with the given layer matrix and destination clip, reading from 'source'
    // with an origin of (0,0).
    Context(const SkMatrix& layerMatrix, const SkIRect& clipBounds, SkImageFilterCache* cache,
            SkColorType colorType, SkColorSpace* colorSpace, const SkSpecialImage* source)
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
#if SK_SUPPORT_GPU
    GrColorType grColorType() const { return SkColorTypeToGrColorType(fColorType); }
#endif
    // The output device's color space, so intermediate images can match, and so filtering can
    // be performed in the destination color space.
    SkColorSpace* colorSpace() const { return fColorSpace; }
    sk_sp<SkColorSpace> refColorSpace() const { return sk_ref_sp(fColorSpace); }
    // The default surface properties to use when making transient surfaces during filtering.
    const SkSurfaceProps* surfaceProps() const { return &fSource->props(); }

    // This is the image to use whenever an expected input filter has been set to null. In the
    // majority of cases, this is the original source image for the image filter DAG so it comes
    // from the SkDevice that holds either the saveLayer or the temporary rendered result. The
    // exception is composing two image filters (via SkImageFilters::Compose), which must use
    // the output of the inner DAG as the "source" for the outer DAG.
    const SkSpecialImage* sourceImage() const { return fSource; }

    // True if image filtering should occur on the GPU if possible.
    bool gpuBacked() const { return fSource->isTextureBacked(); }
    // The recording context to use when computing the filter with the GPU.
    GrRecordingContext* getContext() const { return fSource->getContext(); }

    /**
     *  Since a context can be built directly, its constructor has no chance to "return null" if
     *  it's given invalid or unsupported inputs. Call this to know of the the context can be
     *  used.
     *
     *  The SkImageFilterCache Key, for example, requires a finite ctm (no infinities or NaN),
     *  so that test is part of isValid.
     */
    bool isValid() const { return fSource && fLayerMatrix.isFinite(); }

    // Create a surface of the given size, that matches the context's color type and color space
    // as closely as possible, and uses the same backend of the device that produced the source
    // image.
    sk_sp<SkSpecialSurface> makeSurface(const SkISize& size,
                                        const SkSurfaceProps* props = nullptr) const {
        return fSource->makeSurface(fColorType, fColorSpace, size, kPremul_SkAlphaType, props);
    }

    // Create a new context that matches this context, but with an overridden layer CTM matrix.
    Context withNewLayerMatrix(const SkMatrix& layerMatrix) const {
        return Context(layerMatrix, fClipBounds, fCache, fColorType, fColorSpace, fSource);
    }
    // Create a new context that matches this context, but with an overridden clip bounds rect.
    Context withNewClipBounds(const SkIRect& clipBounds) const {
        return Context(fLayerMatrix, clipBounds, fCache, fColorType, fColorSpace, fSource);
    }

private:
    SkMatrix               fLayerMatrix;
    SkIRect                fClipBounds;
    SkImageFilterCache*    fCache;
    SkColorType            fColorType;
    // These pointers are owned by the device controlling the filter process, and our
    // lifetime is bounded by the device, so they can be bare pointers.
    SkColorSpace*          fColorSpace;
    const SkSpecialImage*  fSource;
};

} // end namespace skif

#endif // SkImageFilterTypes_DEFINED
