/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageFilterTypes_DEFINED
#define SkImageFilterTypes_DEFINED

#include "include/core/SkMatrix.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"

class GrRecordingContext;
class SkImageFilter;
class SkImageFilterCache;
class SkSpecialSurface;
class SkSurfaceProps;

// The skif (SKI[mage]F[ilter]) namespace contains types that are used for filter implementations.
// The defined types come in two groups: users of internal Skia types, and templates to help with
// readability. Image filters cannot be implemented without access to key internal types, such as
// SkSpecialImage. It is possible to avoid the use of the readability templates, although they are
// strongly encouraged.
namespace skif {

// skif::IVector and skif::Vector represent plain-old-data types for storing direction vectors, so
// that the coordinate-space templating system defined below can have a separate type id for
// directions vs. points, and specialize appropriately. As such, all operations with direction
// vectors are defined on the LayerSpace specialization, since that is the intended point of use.
struct IVector {
    int32_t fX;
    int32_t fY;

    IVector() = default;
    IVector(int32_t x, int32_t y) : fX(x), fY(y) {}
    explicit IVector(const SkIVector& v) : fX(v.fX), fY(v.fY) {}
};

struct Vector {
    SkScalar fX;
    SkScalar fY;

    Vector() = default;
    Vector(SkScalar x, SkScalar y) : fX(x), fY(y) {}
    explicit Vector(const SkVector& v) : fX(v.fX), fY(v.fY) {}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Coordinate Space Tagging
// - In order to enforce correct coordinate spaces in image filter implementations and use,
//   geometry is wrapped by templated structs to declare in the type system what coordinate space
//   the coordinates are defined in.
// - Currently there is ParameterSpace and DeviceSpace that are data-only wrappers around
//   coordinates, and the primary LayerSpace that provides all operative functionality for image
//   filters. It is intended that all logic about image bounds and access be conducted in the shared
//   layer space.
// - The LayerSpace struct has type-safe specializations for SkIRect, SkRect, SkIPoint, SkPoint,
//   skif::IVector (to distinguish SkIVector from SkIPoint), skif::Vector, SkISize, and SkSize.
// - A Mapping object provides type safe coordinate conversions between these spaces, and
//   automatically does the "right thing" for each geometric type.
///////////////////////////////////////////////////////////////////////////////////////////////////

// ParameterSpace is a data-only wrapper around Skia's geometric types such as SkIPoint, and SkRect.
// Parameter space is the same as the local coordinate space of an SkShader, or the coordinates
// passed into SkCanvas::drawX calls, but "local" is avoided due to the alliteration with layer
// space. SkImageFilters are defined in terms of ParameterSpace<T> geometry and must use the Mapping
// on Context to transform the parameters into LayerSpace to evaluate the filter in the shared
// coordinate space of the entire filter DAG.
//
// A value of ParameterSpace<SkIRect> implies that its wrapped SkIRect is defined in the local
// parameter space.
template<typename T>
class ParameterSpace {
public:
    explicit ParameterSpace(const T& data) : fData(data) {}
    explicit ParameterSpace(T&& data) : fData(std::move(data)) {}

    explicit operator const T&() const { return fData; }

private:
    T fData;
};

// DeviceSpace is a data-only wrapper around Skia's geometric types. It is similar to
// 'ParameterSpace' except that it is used to represent geometry that has been transformed or
// defined in the root device space (i.e. the final pixels of drawn content). Much of what SkCanvas
// tracks, such as its clip bounds are defined in this space and DeviceSpace provides a
// type-enforced mechanism for the canvas to pass that information into the image filtering system,
// using the Mapping of the filtering context.
template<typename T>
class DeviceSpace {
public:
    explicit DeviceSpace(const T& data) : fData(data) {}
    explicit DeviceSpace(T&& data) : fData(std::move(data)) {}

    explicit operator const T&() const { return fData; }

private:
    T fData;
};

// LayerSpace is a geometric wrapper that specifies the geometry is defined in the shared layer
// space where image filters are evaluated. For a given Context (and its Mapping), the image filter
// DAG operates in the same coordinate space. This space may be different from the local coordinate
// space that defined the image filter parameters (such as blur sigma), and it may be different
// from the total CTM of the SkCanvas.
//
// To encourage correct filter use and implementation, the bulk of filter logic should be performed
// in layer space (e.g. determining what portion of an input image to read, or what the output
// region is). LayerSpace specializations for the six common Skia math types (Sk[I]Rect, Sk[I]Point,
// and Sk[I]Size), and skif::[I]Vector (to allow vectors to be specialized separately from points))
// are provided that mimic their APIs but preserve the coordinate space and enforce type semantics.
template<typename T>
class LayerSpace {};

// Layer-space specialization for integerized direction vectors.
template<>
class LayerSpace<IVector> {
public:
    LayerSpace() = default;
    explicit LayerSpace(const IVector& geometry) : fData(geometry) {}
    explicit LayerSpace(IVector&& geometry) : fData(std::move(geometry)) {}
    explicit operator const IVector&() const { return fData; }

    explicit operator SkIVector() const { return SkIVector::Make(fData.fX, fData.fY); }

    int32_t x() const { return fData.fX; }
    int32_t y() const { return fData.fY; }

    LayerSpace<IVector> operator-() const { return LayerSpace<IVector>({-fData.fX, -fData.fY}); }

    LayerSpace<IVector> operator+(const LayerSpace<IVector>& v) const {
        LayerSpace<IVector> sum = *this;
        sum += v;
        return sum;
    }
    LayerSpace<IVector> operator-(const LayerSpace<IVector>& v) const {
        LayerSpace<IVector> diff = *this;
        diff -= v;
        return diff;
    }

    void operator+=(const LayerSpace<IVector>& v) {
        fData.fX += v.fData.fX;
        fData.fY += v.fData.fY;
    }
    void operator-=(const LayerSpace<IVector>& v) {
        fData.fX -= v.fData.fX;
        fData.fY -= v.fData.fY;
    }

private:
    IVector fData;
};

// Layer-space specialization for floating point direction vectors.
template<>
class LayerSpace<Vector> {
public:
    LayerSpace() = default;
    explicit LayerSpace(const Vector& geometry) : fData(geometry) {}
    explicit LayerSpace(Vector&& geometry) : fData(std::move(geometry)) {}
    explicit operator const Vector&() const { return fData; }

    explicit operator SkVector() const { return SkVector::Make(fData.fX, fData.fY); }

    SkScalar x() const { return fData.fX; }
    SkScalar y() const { return fData.fY; }

    SkScalar length() const { return SkVector::Length(fData.fX, fData.fY); }

    LayerSpace<Vector> operator-() const { return LayerSpace<Vector>({-fData.fX, -fData.fY}); }

    LayerSpace<Vector> operator*(SkScalar s) const {
        LayerSpace<Vector> scaled = *this;
        scaled *= s;
        return scaled;
    }

    LayerSpace<Vector> operator+(const LayerSpace<Vector>& v) const {
        LayerSpace<Vector> sum = *this;
        sum += v;
        return sum;
    }
    LayerSpace<Vector> operator-(const LayerSpace<Vector>& v) const {
        LayerSpace<Vector> diff = *this;
        diff -= v;
        return diff;
    }

    void operator*=(SkScalar s) {
        fData.fX *= s;
        fData.fY *= s;
    }
    void operator+=(const LayerSpace<Vector>& v) {
        fData.fX += v.fData.fX;
        fData.fY += v.fData.fY;
    }
    void operator-=(const LayerSpace<Vector>& v) {
        fData.fX -= v.fData.fX;
        fData.fY -= v.fData.fY;
    }

    friend LayerSpace<Vector> operator*(SkScalar s, const LayerSpace<Vector>& b) {
        return b * s;
    }

private:
    Vector fData;
};

// Layer-space specialization for integer 2D coordinates (treated as positions, not directions).
template<>
class LayerSpace<SkIPoint> {
public:
    LayerSpace() = default;
    explicit LayerSpace(const SkIPoint& geometry)  : fData(geometry) {}
    explicit LayerSpace(SkIPoint&& geometry) : fData(std::move(geometry)) {}
    explicit operator const SkIPoint&() const { return fData; }

    // Parrot the SkIPoint API while preserving coordinate space.
    int32_t x() const { return fData.fX; }
    int32_t y() const { return fData.fY; }

    // Offsetting by direction vectors produce more points
    LayerSpace<SkIPoint> operator+(const LayerSpace<IVector>& v) {
        return LayerSpace<SkIPoint>(fData + SkIVector(v));
    }
    LayerSpace<SkIPoint> operator-(const LayerSpace<IVector>& v) {
        return LayerSpace<SkIPoint>(fData - SkIVector(v));
    }

    void operator+=(const LayerSpace<IVector>& v) {
        fData += SkIVector(v);
    }
    void operator-=(const LayerSpace<IVector>& v) {
        fData -= SkIVector(v);
    }

    // Subtracting another point makes a direction between them
    LayerSpace<IVector> operator-(const LayerSpace<SkIPoint>& p) {
        return LayerSpace<IVector>(IVector(fData - p.fData));
    }

private:
    SkIPoint fData;
};

// Layer-space specialization for floating point 2D coordinates (treated as positions)
template<>
class LayerSpace<SkPoint> {
public:
    LayerSpace() = default;
    explicit LayerSpace(const SkPoint& geometry) : fData(geometry) {}
    explicit LayerSpace(SkPoint&& geometry) : fData(std::move(geometry)) {}
    explicit operator const SkPoint&() const { return fData; }

    // Parrot the SkPoint API while preserving coordinate space.
    SkScalar x() const { return fData.fX; }
    SkScalar y() const { return fData.fY; }

    SkScalar distanceToOrigin() const { return fData.distanceToOrigin(); }

    // Offsetting by direction vectors produce more points
    LayerSpace<SkPoint> operator+(const LayerSpace<Vector>& v) {
        return LayerSpace<SkPoint>(fData + SkVector(v));
    }
    LayerSpace<SkPoint> operator-(const LayerSpace<Vector>& v) {
        return LayerSpace<SkPoint>(fData - SkVector(v));
    }

    void operator+=(const LayerSpace<Vector>& v) {
        fData += SkVector(v);
    }
    void operator-=(const LayerSpace<Vector>& v) {
        fData -= SkVector(v);
    }

    // Subtracting another point makes a direction between them
    LayerSpace<Vector> operator-(const LayerSpace<SkPoint>& p) {
        return LayerSpace<Vector>(Vector(fData - p.fData));
    }

private:
    SkPoint fData;
};

// Layer-space specialization for integer dimensions
template<>
class LayerSpace<SkISize> {
public:
    LayerSpace() = default;
    explicit LayerSpace(const SkISize& geometry) : fData(geometry) {}
    explicit LayerSpace(SkISize&& geometry) : fData(std::move(geometry)) {}
    explicit operator const SkISize&() const { return fData; }

    int32_t width() const { return fData.width(); }
    int32_t height() const { return fData.height(); }

    bool isEmpty() const { return fData.isEmpty(); }

private:
    SkISize fData;
};

// Layer-space specialization for floating point dimensions
template<>
class LayerSpace<SkSize> {
public:
    LayerSpace() = default;
    explicit LayerSpace(const SkSize& geometry) : fData(geometry) {}
    explicit LayerSpace(SkSize&& geometry) : fData(std::move(geometry)) {}
    explicit operator const SkSize&() const { return fData; }

    SkScalar width() const { return fData.width(); }
    SkScalar height() const { return fData.height(); }

    bool isEmpty() const { return fData.isEmpty(); }
    bool isZero() const { return fData.isZero(); }

    LayerSpace<SkISize> round() const { return LayerSpace<SkISize>(fData.toRound()); }
    LayerSpace<SkISize> ceil() const { return LayerSpace<SkISize>(fData.toCeil()); }
    LayerSpace<SkISize> floor() const { return LayerSpace<SkISize>(fData.toFloor()); }

private:
    SkSize fData;
};

// Layer-space specialization for axis-aligned integer bounding boxes.
template<>
class LayerSpace<SkIRect> {
public:
    LayerSpace() = default;
    explicit LayerSpace(const SkIRect& geometry) : fData(geometry) {}
    explicit LayerSpace(SkIRect&& geometry) : fData(std::move(geometry)) {}
    explicit operator const SkIRect&() const { return fData; }

    // Parrot the SkIRect API while preserving coord space
    int32_t left() const { return fData.fLeft; }
    int32_t top() const { return fData.fTop; }
    int32_t right() const { return fData.fRight; }
    int32_t bottom() const { return fData.fBottom; }

    int32_t width() const { return fData.width(); }
    int32_t height() const { return fData.height(); }

    LayerSpace<SkIPoint> topLeft() const { return LayerSpace<SkIPoint>(fData.topLeft()); }
    LayerSpace<SkISize> size() const { return LayerSpace<SkISize>(fData.size()); }

    bool intersect(const LayerSpace<SkIRect>& r) { return fData.intersect(r.fData); }
    void join(const LayerSpace<SkIRect>& r) { fData.join(r.fData); }
    void offset(const LayerSpace<IVector>& v) { fData.offset(SkIVector(v)); }
    void outset(const LayerSpace<SkISize>& delta) { fData.outset(delta.width(), delta.height()); }

private:
    SkIRect fData;
};

// Layer-space specialization for axis-aligned float bounding boxes.
template<>
class LayerSpace<SkRect> {
public:
    LayerSpace() = default;
    explicit LayerSpace(const SkRect& geometry) : fData(geometry) {}
    explicit LayerSpace(SkRect&& geometry) : fData(std::move(geometry)) {}
    explicit operator const SkRect&() const { return fData; }

    // Parrot the SkRect API while preserving coord space and usage
    SkScalar left() const { return fData.fLeft; }
    SkScalar top() const { return fData.fTop; }
    SkScalar right() const { return fData.fRight; }
    SkScalar bottom() const { return fData.fBottom; }

    SkScalar width() const { return fData.width(); }
    SkScalar height() const { return fData.height(); }

    LayerSpace<SkPoint> topLeft() const {
        return LayerSpace<SkPoint>(SkPoint::Make(fData.fLeft, fData.fTop));
    }
    LayerSpace<SkSize> size() const {
        return LayerSpace<SkSize>(SkSize::Make(fData.width(), fData.height()));
    }
    LayerSpace<SkIRect> roundOut() const { return LayerSpace<SkIRect>(fData.roundOut()); }

    bool intersect(const LayerSpace<SkRect>& r) { return fData.intersect(r.fData); }
    void join(const LayerSpace<SkRect>& r) { fData.join(r.fData); }
    void offset(const LayerSpace<Vector>& v) { fData.offset(SkVector(v)); }
    void outset(const LayerSpace<SkSize>& delta) { fData.outset(delta.width(), delta.height()); }

private:
    SkRect fData;
};

// Mapping is the primary definition of the shared layer space used when evaluating an image filter
// DAG. It encapsulates any needed decomposition of the total CTM into the parameter-to-layer matrix
// (that filters use to map their parameters to the layer space), and the layer-to-device matrix
// (that canvas uses to map the output layer-space image into its root device space). Mapping
// defines functions to transform ParameterSpace and DeviceSpace types to and from their LayerSpace
// variants, which can then be used and reasoned about by SkImageFilter implementations.
class Mapping {
public:
    // This constructor allows the decomposition to be explicitly provided
    Mapping(const SkMatrix& layerToDev, const SkMatrix& paramToLayer)
            : fLayerToDevMatrix(layerToDev)
            , fParamToLayerMatrix(paramToLayer) {}

    // Make the default decomposition Mapping, given the total CTM and the root image filter.
    static Mapping Make(const SkMatrix& ctm, const SkImageFilter* filter);

    // Return a new Mapping object whose parameter-to-layer matrix is equal to this->layerMatrix() *
    // local, but both share the same layer-to-device matrix.
    Mapping concatLocal(const SkMatrix& local) const {
        return Mapping(fLayerToDevMatrix, SkMatrix::Concat(fParamToLayerMatrix, local));
    }

    const SkMatrix& deviceMatrix() const { return fLayerToDevMatrix; }
    const SkMatrix& layerMatrix() const { return fParamToLayerMatrix; }
    SkMatrix totalMatrix() const {
        return SkMatrix::Concat(fLayerToDevMatrix, fParamToLayerMatrix);
    }

    template<typename T>
    LayerSpace<T> paramToLayer(const ParameterSpace<T>& paramGeometry) const {
        return LayerSpace<T>(map(static_cast<const T&>(paramGeometry), fParamToLayerMatrix));
    }

    template<typename T>
    LayerSpace<T> deviceToLayer(const DeviceSpace<T>& devGeometry) const {
        // The mapping from device space to layer space is defined by the inverse of the
        // layer-to-device matrix
        SkMatrix devToLayerMatrix;
        if (!fLayerToDevMatrix.invert(&devToLayerMatrix)) {
            // Punt and just pass through the geometry unmodified...
            return LayerSpace<T>(static_cast<const T&>(devGeometry));
        } else {
            return LayerSpace<T>(map(static_cast<const T&>(devGeometry), devToLayerMatrix));
        }
    }

    template<typename T>
    DeviceSpace<T> layerToDevice(const LayerSpace<T>& layerGeometry) const {
        return DeviceSpace<T>(map(static_cast<const T&>(layerGeometry), fLayerToDevMatrix));
    }

private:
    // The image filter process decomposes the total CTM into layerToDev * paramToLayer and uses the
    // param-to-layer matrix to define the layer-space coordinate system. Depending on how it's
    // decomposed, either the layer matrix or the device matrix could be the identity matrix (but
    // sometimes neither).
    SkMatrix fLayerToDevMatrix;
    SkMatrix fParamToLayerMatrix;

    // Actual geometric mapping operations that work on coordinates and matrices w/o the type
    // safety of the coordinate space wrappers (hence these are private).
    template<typename T>
    static T map(const T& geom, const SkMatrix& matrix);
};

// Usage is a template tag to improve the readability of filter implementations. It is attached to
// images and geometry to group a collection of related variables and ensure that moving from one
// use case to another is explicit.
// NOTE: This can be aliased as 'For' when using the fluent type names.
// TODO (michaelludwig) - If the primary motivation for Usage--enforcing layer to image space
// transformations safely when multiple images are involved--can be handled entirely by helper
// functions on FilterResult, then Usage can go away and FilterResult will not need to be templated
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
// the image. It has its top left corner at 'origin' and has the same dimensions as the underlying
// special image subset. Transforming 'layerBounds' by the Context's layer matrix and painting it
// with the subset rectangle will display the filtered results in the appropriate device-space
// region.
//
// When filter implementations are processing intermediate FilterResult results, it can be assumed
// that all FilterResult' layerBounds are in the same layer coordinate space defined by the shared
// skif::Context.
//
// NOTE: This is named FilterResult since most instances will represent the output of an image
// filter (even if that is then cast to be the input to the next filter). The main exception is the
// source input used when an input filter is null, but from a data-standpoint it is the same since
// it is equivalent to the result of an identity filter.
template<Usage kU>
class FilterResult {
public:
    FilterResult() : fImage(nullptr), fOrigin(SkIPoint::Make(0, 0)) {}

    FilterResult(sk_sp<SkSpecialImage> image, const LayerSpace<SkIPoint>& origin)
            : fImage(std::move(image))
            , fOrigin(origin) {}

    // Allow explicit moves/copies in order to cast from one use type to another, except kInput0
    // and kInput1 can only be cast to kOutput (e.g. as part of a noop image filter).
    template<Usage kI>
    explicit FilterResult(FilterResult<kI>&& image)
            : fImage(std::move(image.fImage))
            , fOrigin(image.fOrigin) {
        static_assert((kU != Usage::kInput) || (kI != Usage::kInput0 && kI != Usage::kInput1),
                      "kInput0 and kInput1 cannot be moved to more generic kInput usage.");
        static_assert((kU != Usage::kInput0 && kU != Usage::kInput1) ||
                      (kI == kU || kI == Usage::kInput || kI == Usage::kOutput),
                      "Can only move to specific input from the generic kInput or kOutput usage.");
    }

    template<Usage kI>
    explicit FilterResult(const FilterResult<kI>& image)
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

    // Get the layer-space bounds of the result. This will have the same dimensions as the
    // image and its top left corner will be 'origin()'.
    LayerSpace<SkIRect> layerBounds() const {
        return LayerSpace<SkIRect>(SkIRect::MakeXYWH(fOrigin.x(), fOrigin.y(),
                                                     fImage->width(), fImage->height()));
    }

    // Get the layer-space coordinate of this image's top left pixel.
    const LayerSpace<SkIPoint>& layerOrigin() const { return fOrigin; }

    // Extract image and origin, safely when the image is null.
    // TODO (michaelludwig) - This is intended for convenience until all call sites of
    // SkImageFilter_Base::filterImage() have been updated to work in the new type system
    // (which comes later as SkDevice, SkCanvas, etc. need to be modified, and coordinate space
    // tagging needs to be added).
    sk_sp<SkSpecialImage> imageAndOffset(SkIPoint* offset) const {
        if (fImage) {
            *offset = SkIPoint(fOrigin);
            return fImage;
        } else {
            *offset = {0, 0};
            return nullptr;
        }
    }

private:
    // Allow all FilterResult templates access to each others members
    template<Usage kO>
    friend class FilterResult;

    sk_sp<SkSpecialImage> fImage;
    LayerSpace<SkIPoint> fOrigin;
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
        : fMapping(SkMatrix::I(), layerMatrix)
        , fDesiredOutput(clipBounds)
        , fCache(cache)
        , fColorType(colorType)
        , fColorSpace(colorSpace)
        , fSource(sk_ref_sp(source), LayerSpace<SkIPoint>({0, 0})) {}

    Context(const Mapping& mapping, const LayerSpace<SkIRect>& desiredOutput,
            SkImageFilterCache* cache, SkColorType colorType, SkColorSpace* colorSpace,
            const FilterResult<For::kInput>& source)
        : fMapping(mapping)
        , fDesiredOutput(desiredOutput)
        , fCache(cache)
        , fColorType(colorType)
        , fColorSpace(colorSpace)
        , fSource(source) {}

    // The mapping that defines the transformation from local parameter space of the filters to the
    // layer space where the image filters are evaluated, as well as the remaining transformation
    // from the layer space to the final device space. The layer space defined by the returned
    // Mapping may be the same as the root device space, or be an intermediate space that is
    // supported by the image filter DAG (depending on what it returns from canHandleComplexCTM()).
    // If a node returns false from canHandleComplexCTM(), the layer matrix of the mapping will be
    // at most a scale + translate, and the remaining matrix will be appropriately set to transform
    // the layer space to the final device space (applied by the SkCanvas when filtering is
    // finished).
    const Mapping& mapping() const { return fMapping; }
    // DEPRECATED: Use mapping() and its coordinate-space types instead
    const SkMatrix& ctm() const { return fMapping.layerMatrix(); }
    // The bounds, in the layer space, that the filtered image will be clipped to. The output
    // from filterImage() must cover these clip bounds, except in areas where it will just be
    // transparent black, in which case a smaller output image can be returned.
    const LayerSpace<SkIRect>& desiredOutput() const { return fDesiredOutput; }
    // DEPRECATED: Use desiredOutput() instead
    const SkIRect& clipBounds() const { return static_cast<const SkIRect&>(fDesiredOutput); }
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
    const SkSurfaceProps* surfaceProps() const { return &fSource.image()->props(); }

    // This is the image to use whenever an expected input filter has been set to null. In the
    // majority of cases, this is the original source image for the image filter DAG so it comes
    // from the SkDevice that holds either the saveLayer or the temporary rendered result. The
    // exception is composing two image filters (via SkImageFilters::Compose), which must use
    // the output of the inner DAG as the "source" for the outer DAG.
    const FilterResult<For::kInput>& source() const { return fSource; }
    // DEPRECATED: Use source() instead to get both the image and its origin.
    const SkSpecialImage* sourceImage() const { return fSource.image(); }

    // True if image filtering should occur on the GPU if possible.
    bool gpuBacked() const { return fSource.image()->isTextureBacked(); }
    // The recording context to use when computing the filter with the GPU.
    GrRecordingContext* getContext() const { return fSource.image()->getContext(); }

    /**
     *  Since a context can be built directly, its constructor has no chance to "return null" if
     *  it's given invalid or unsupported inputs. Call this to know of the the context can be
     *  used.
     *
     *  The SkImageFilterCache Key, for example, requires a finite ctm (no infinities or NaN),
     *  so that test is part of isValid.
     */
    bool isValid() const { return fSource.image() != nullptr && fMapping.layerMatrix().isFinite(); }

    // Create a surface of the given size, that matches the context's color type and color space
    // as closely as possible, and uses the same backend of the device that produced the source
    // image.
    sk_sp<SkSpecialSurface> makeSurface(const SkISize& size,
                                        const SkSurfaceProps* props = nullptr) const {
        return fSource.image()->makeSurface(fColorType, fColorSpace, size,
                                            kPremul_SkAlphaType, props);
    }

    // Create a new context that matches this context, but with an overridden layer space.
    Context withNewMapping(const Mapping& mapping) const {
        return Context(mapping, fDesiredOutput, fCache, fColorType, fColorSpace, fSource);
    }
    // Create a new context that matches this context, but with an overridden desired output rect.
    Context withNewDesiredOutput(const LayerSpace<SkIRect>& desiredOutput) const {
        return Context(fMapping, desiredOutput, fCache, fColorType, fColorSpace, fSource);
    }

private:
    Mapping                   fMapping;
    LayerSpace<SkIRect>       fDesiredOutput;
    SkImageFilterCache*       fCache;
    SkColorType               fColorType;
    // The pointed-to object is owned by the device controlling the filter process, and our lifetime
    // is bounded by the device, so this can be a bare pointer.
    SkColorSpace*             fColorSpace;
    FilterResult<For::kInput> fSource;
};

} // end namespace skif

#endif // SkImageFilterTypes_DEFINED
