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
    ParameterSpace() = default;
    explicit ParameterSpace(const T& data) : fData(data) {}
    explicit ParameterSpace(T&& data) : fData(std::move(data)) {}

    explicit operator const T&() const { return fData; }

    static const ParameterSpace<T>* Optional(const T* ptr) {
        return static_cast<const ParameterSpace<T>*>(reinterpret_cast<const void*>(ptr));
    }
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
    DeviceSpace() = default;
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

    LayerSpace<IVector> operator-() const { return LayerSpace<IVector>({-fData.fX, -fData.fY}); }

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

    LayerSpace<Vector> operator-() const { return LayerSpace<Vector>({-fData.fX, -fData.fY}); }

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

    static LayerSpace<SkIRect> Empty() { return LayerSpace<SkIRect>(SkIRect::MakeEmpty()); }

    // Parrot the SkIRect API while preserving coord space
    bool isEmpty() const { return fData.isEmpty(); }

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

    static LayerSpace<SkRect> Empty() { return LayerSpace<SkRect>(SkRect::MakeEmpty()); }

    // Parrot the SkRect API while preserving coord space and usage
    bool isEmpty() const { return fData.isEmpty(); }

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

    LayerSpace<SkIRect> round() const { return LayerSpace<SkIRect>(fData.round()); }
    LayerSpace<SkIRect> roundIn() const { return LayerSpace<SkIRect>(fData.roundIn()); }
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
    Mapping() = default;

    // This constructor allows the decomposition to be explicitly provided, requires
    // layerToDev to be invertible.
    Mapping(const SkMatrix& layerToDev, const SkMatrix& paramToLayer)
            : fLayerToDevMatrix(layerToDev)
            , fParamToLayerMatrix(paramToLayer) {
        SkAssertResult(fLayerToDevMatrix.invert(&fDevToLayerMatrix));
    }

    // Sets this Mapping to the default decomposition of the canvas's total transform, given the
    // requirements of the 'filter'. Returns false if the decomposition failed or would produce an
    // invalid device matrix. Assumes 'ctm' is invertible.
    bool SK_WARN_UNUSED_RESULT decomposeCTM(const SkMatrix& ctm,
                                            const SkImageFilter* filter,
                                            const skif::ParameterSpace<SkPoint>& representativePt);

    // Update the mapping's parameter-to-layer matrix to be pre-concatenated with the specified
    // local space transformation. This changes the definition of parameter space, any
    // skif::ParameterSpace<> values are interpreted anew. Layer space and device space are
    // unchanged.
    void concatLocal(const SkMatrix& local) { fParamToLayerMatrix.preConcat(local); }

    // Update the mapping's layer space coordinate system by post-concatenating the given matrix
    // to it's parameter-to-layer transform, and pre-concatenating the inverse of the matrix with
    // it's layer-to-device transform. The net effect is that neither the parameter nor device
    // coordinate systems are changed, but skif::LayerSpace is adjusted.
    //
    // Returns false if the layer matrix cannot be inverted, and this mapping is left unmodified.
    bool adjustLayerSpace(const SkMatrix& layer);

    // Update the mapping's layer space so that the point 'origin' in the current layer coordinate
    // space maps to (0, 0) in the adjusted coordinate space.
    void applyOrigin(const LayerSpace<SkIPoint>& origin) {
        SkAssertResult(this->adjustLayerSpace(SkMatrix::Translate(-origin.x(), -origin.y())));
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
        return LayerSpace<T>(map(static_cast<const T&>(devGeometry), fDevToLayerMatrix));
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

    // Cached inverse of fLayerToDevMatrix
    SkMatrix fDevToLayerMatrix;

    // Actual geometric mapping operations that work on coordinates and matrices w/o the type
    // safety of the coordinate space wrappers (hence these are private).
    template<typename T>
    static T map(const T& geom, const SkMatrix& matrix);
};

// Wraps an SkSpecialImage and its location in the shared layer coordinate space. This origin is
// used to draw the image in the correct location. The 'layerBounds' rectangle of the filtered image
// is the layer-space bounding box of the image. It has its top left corner at 'origin' and has the
// same dimensions as the underlying special image subset. Transforming 'layerBounds' by the
// Context's layer-to-device matrix and painting it with the subset rectangle will display the
// filtered results in the appropriate device-space region.
//
// When filter implementations are processing intermediate FilterResult results, it can be assumed
// that all FilterResult' layerBounds are in the same layer coordinate space defined by the shared
// skif::Context.
//
// NOTE: This is named FilterResult since most instances will represent the output of an image
// filter (even if that is then used as an input to the next filter). The main exception is the
// source input used when an input filter is null, but from a data-standpoint it is the same since
// it is equivalent to the result of an identity filter.
class FilterResult {
public:
    FilterResult() : fImage(nullptr), fOrigin(SkIPoint::Make(0, 0)) {}

    FilterResult(sk_sp<SkSpecialImage> image, const LayerSpace<SkIPoint>& origin)
            : fImage(std::move(image))
            , fOrigin(origin) {}
    explicit FilterResult(sk_sp<SkSpecialImage> image)
            : fImage(std::move(image))
            , fOrigin{{0, 0}} {}

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

    // Produce a new FilterResult that can correctly cover 'newBounds' when it's drawn with its
    // tile mode at its origin. When possible, the returned FilterResult can reuse the same image
    // data and adjust its access subset, origin, and tile mode. If 'forcePad' is true or if the
    // 'newTileMode' that applies at the 'newBounds' geometry is incompatible with the current
    // bounds and tile mode, then a new image is created that resolves this image's data and tiling.
    // TODO (michaelludwig): All FilterResults are decal mode and there are no current usages that
    // require force-padding a decal FilterResult so these arguments aren't implemented yet.
    FilterResult resolveToBounds(const LayerSpace<SkIRect>& newBounds) const;
                                //  SkTileMode newTileMode=SkTileMode::kDecal,
                                //  bool forcePad=false) const;

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
    sk_sp<SkSpecialImage> fImage;
    LayerSpace<SkIPoint>  fOrigin;
    // SkTileMode         fTileMode = SkTileMode::kDecal;
};

// The context contains all necessary information to describe how the image filter should be
// computed (i.e. the current layer matrix and clip), and the color information of the output of a
// filter DAG. For now, this is just the color space (of the original requesting device). This is
// used when constructing intermediate rendering surfaces, so that we ensure we land in a surface
// that's similar/compatible to the final consumer of the DAG's output.
class Context {
public:
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
            const FilterResult& source)
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
    // supported by the image filter DAG (depending on what it returns from getCTMCapability()).
    // If a node returns something other than kComplex from getCTMCapability(), the layer matrix of
    // the mapping will respect that return value, and the remaining matrix will be appropriately
    // set to transform the layer space to the final device space (applied by the SkCanvas when
    // filtering is finished).
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
    const SkSurfaceProps& surfaceProps() const { return fSource.image()->props(); }

    // This is the image to use whenever an expected input filter has been set to null. In the
    // majority of cases, this is the original source image for the image filter DAG so it comes
    // from the SkDevice that holds either the saveLayer or the temporary rendered result. The
    // exception is composing two image filters (via SkImageFilters::Compose), which must use
    // the output of the inner DAG as the "source" for the outer DAG.
    const FilterResult& source() const { return fSource; }
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
        if (!props) {
             props = &this->surfaceProps();
        }
        return fSource.image()->makeSurface(fColorType, fColorSpace, size,
                                            kPremul_SkAlphaType, *props);
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
    Mapping             fMapping;
    LayerSpace<SkIRect> fDesiredOutput;
    SkImageFilterCache* fCache;
    SkColorType         fColorType;
    // The pointed-to object is owned by the device controlling the filter process, and our lifetime
    // is bounded by the device, so this can be a bare pointer.
    SkColorSpace*       fColorSpace;
    FilterResult        fSource;
};

} // end namespace skif

#endif // SkImageFilterTypes_DEFINED
