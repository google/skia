/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageFilterTypes_DEFINED
#define SkImageFilterTypes_DEFINED

#include "include/core/SkColorFilter.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkSpan.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTPin.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkEnumBitMask.h"
#include "src/core/SkSpecialImage.h"

#include <cstdint>
#include <optional>
#include <utility>

class FilterResultTestAccess;  // for testing
class SkBitmap;
class SkBlender;
class SkDevice;
class SkImage;
class SkImageFilter;
class SkImageFilterCache;
class SkPicture;
class SkShader;
enum SkColorType : int;

// The skif (SKI[mage]F[ilter]) namespace contains types that are used for filter implementations.
// The defined types come in two groups: users of internal Skia types, and templates to help with
// readability. Image filters cannot be implemented without access to key internal types, such as
// SkSpecialImage. It is possible to avoid the use of the readability templates, although they are
// strongly encouraged.
namespace skif {

// Rounds in/out but with a tolerance.
SkIRect RoundOut(SkRect);
SkIRect RoundIn(SkRect);

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

    bool isFinite() const { return SkScalarsAreFinite(fX, fY); }
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

    LayerSpace<SkISize> round() const;
    LayerSpace<SkISize> ceil() const;
    LayerSpace<SkISize> floor() const;

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
    explicit LayerSpace(const SkISize& size) : fData(SkIRect::MakeSize(size)) {}
    explicit operator const SkIRect&() const { return fData; }

    static LayerSpace<SkIRect> Empty() { return LayerSpace<SkIRect>(SkIRect::MakeEmpty()); }

    static constexpr std::optional<LayerSpace<SkIRect>> Unbounded() { return {}; }

    // Utility function to iterate a collection of items that can map to LayerSpace<SkIRect> bounds
    // and returns the union of those bounding boxes. 'boundsFn' will be invoked with i = 0 to
    // boundsCount-1.
    template<typename BoundsFn>
    static LayerSpace<SkIRect> Union(int boundsCount, BoundsFn boundsFn) {
        if (boundsCount <= 0) {
            return LayerSpace<SkIRect>::Empty();
        }
        LayerSpace<SkIRect> output = boundsFn(0);
        for (int i = 1; i < boundsCount; ++i) {
            output.join(boundsFn(i));
        }
        return output;
    }

    // Parrot the SkIRect API while preserving coord space
    bool isEmpty() const { return fData.isEmpty64(); }
    bool contains(const LayerSpace<SkIRect>& r) const { return fData.contains(r.fData); }

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
    void inset(const LayerSpace<SkISize>& delta) { fData.inset(delta.width(), delta.height()); }

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
    explicit LayerSpace(const LayerSpace<SkIRect>& rect) : fData(SkRect::Make(SkIRect(rect))) {}
    explicit operator const SkRect&() const { return fData; }

    static LayerSpace<SkRect> Empty() { return LayerSpace<SkRect>(SkRect::MakeEmpty()); }

    // Parrot the SkRect API while preserving coord space and usage
    bool isEmpty() const { return fData.isEmpty(); }
    bool contains(const LayerSpace<SkRect>& r) const { return fData.contains(r.fData); }

    SkScalar left() const { return fData.fLeft; }
    SkScalar top() const { return fData.fTop; }
    SkScalar right() const { return fData.fRight; }
    SkScalar bottom() const { return fData.fBottom; }

    SkScalar width() const { return fData.width(); }
    SkScalar height() const { return fData.height(); }

    LayerSpace<SkPoint> topLeft() const {
        return LayerSpace<SkPoint>(SkPoint::Make(fData.fLeft, fData.fTop));
    }
    LayerSpace<SkPoint> center() const {
        return LayerSpace<SkPoint>(fData.center());
    }
    LayerSpace<SkSize> size() const {
        return LayerSpace<SkSize>(SkSize::Make(fData.width(), fData.height()));
    }

    LayerSpace<SkIRect> round() const { return LayerSpace<SkIRect>(fData.round()); }
    LayerSpace<SkIRect> roundIn() const { return LayerSpace<SkIRect>(RoundIn(fData)); }
    LayerSpace<SkIRect> roundOut() const { return LayerSpace<SkIRect>(RoundOut(fData)); }

    bool intersect(const LayerSpace<SkRect>& r) { return fData.intersect(r.fData); }
    void join(const LayerSpace<SkRect>& r) { fData.join(r.fData); }
    void offset(const LayerSpace<Vector>& v) { fData.offset(SkVector(v)); }
    void outset(const LayerSpace<SkSize>& delta) { fData.outset(delta.width(), delta.height()); }
    void inset(const LayerSpace<SkSize>& delta) { fData.inset(delta.width(), delta.height()); }

    LayerSpace<SkPoint> clamp(LayerSpace<SkPoint> pt) const {
        return LayerSpace<SkPoint>(SkPoint::Make(SkTPin(pt.x(), fData.fLeft, fData.fRight),
                                                 SkTPin(pt.y(), fData.fTop, fData.fBottom)));
    }

private:
    SkRect fData;
};

// A transformation that manipulates geometry in the layer-space coordinate system. Mathematically
// there's little difference from these matrices compared to what's stored in a skif::Mapping, but
// the intent differs. skif::Mapping's matrices map geometry from one coordinate space to another
// while these transforms move geometry w/o changing the coordinate space semantics.
// TODO(michaelludwig): Will be replaced with an SkM44 version when skif::Mapping works with SkM44.
template<>
class LayerSpace<SkMatrix> {
public:
    LayerSpace() = default;
    explicit LayerSpace(const SkMatrix& m) : fData(m) {}
    explicit LayerSpace(SkMatrix&& m) : fData(std::move(m)) {}
    explicit operator const SkMatrix&() const { return fData; }

    static LayerSpace<SkMatrix> RectToRect(const LayerSpace<SkRect>& from,
                                           const LayerSpace<SkRect>& to) {
        return LayerSpace<SkMatrix>(SkMatrix::RectToRect(SkRect(from), SkRect(to)));
    }

    // Parrot a limited selection of the SkMatrix API while preserving coordinate space.
    LayerSpace<SkRect> mapRect(const LayerSpace<SkRect>& r) const;

    // Effectively mapRect(SkRect).roundOut() but more accurate when the underlying matrix or
    // SkIRect has large floating point values.
    LayerSpace<SkIRect> mapRect(const LayerSpace<SkIRect>& r) const;

    LayerSpace<SkPoint> mapPoint(const LayerSpace<SkPoint>& p) const;

    LayerSpace<Vector> mapVector(const LayerSpace<Vector>& v) const;

    LayerSpace<SkSize> mapSize(const LayerSpace<SkSize>& s) const;

    LayerSpace<SkMatrix>& preConcat(const LayerSpace<SkMatrix>& m) {
        fData = SkMatrix::Concat(fData, m.fData);
        return *this;
    }

    LayerSpace<SkMatrix>& postConcat(const LayerSpace<SkMatrix>& m) {
        fData = SkMatrix::Concat(m.fData, fData);
        return *this;
    }

    bool invert(LayerSpace<SkMatrix>* inverse) const {
        return fData.invert(&inverse->fData);
    }

    // Transforms 'r' by the inverse of this matrix if it is invertible and stores it in 'out'.
    // Returns false if not invertible, in which case 'out' is undefined.
    bool inverseMapRect(const LayerSpace<SkRect>& r, LayerSpace<SkRect>* out) const;
    bool inverseMapRect(const LayerSpace<SkIRect>& r, LayerSpace<SkIRect>* out) const;

    float rc(int row, int col) const { return fData.rc(row, col); }
    float get(int i) const { return fData.get(i); }

private:
    SkMatrix fData;
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

    // Helper constructor that equates device and layer space to the same coordinate space.
    explicit Mapping(const SkMatrix& paramToLayer)
            : fLayerToDevMatrix(SkMatrix::I())
            , fParamToLayerMatrix(paramToLayer)
            , fDevToLayerMatrix(SkMatrix::I()) {}

    // This constructor allows the decomposition to be explicitly provided, assumes that
    // 'layerToDev's inverse has already been calculated in 'devToLayer'
    Mapping(const SkMatrix& layerToDev, const SkMatrix& devToLayer, const SkMatrix& paramToLayer)
            : fLayerToDevMatrix(layerToDev)
            , fParamToLayerMatrix(paramToLayer)
            , fDevToLayerMatrix(devToLayer) {}

    // Sets this Mapping to the default decomposition of the canvas's total transform, given the
    // requirements of the 'filter'. Returns false if the decomposition failed or would produce an
    // invalid device matrix. Assumes 'ctm' is invertible.
    [[nodiscard]] bool decomposeCTM(const SkMatrix& ctm,
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

    const SkMatrix& layerToDevice() const { return fLayerToDevMatrix; }
    const SkMatrix& deviceToLayer() const { return fDevToLayerMatrix; }
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
    friend class LayerSpace<SkMatrix>; // for map()

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

class Context; // Forward declare for FilterResult

// A FilterResult represents a lazy image anchored in the "layer" coordinate space of the current
// image filtering context. It's named Filter*Result* since most instances represent the output of
// a specific image filter (even if that is then used as an input to the next filter). FilterResults
// are lazy to allow certain operations to combine analytically instead of producing an offscreen
// image for every node in a filter graph. Helper functions are provided to modify FilterResults
// that manage this internally.
//
// Even though FilterResult represents a lazy image, it is always backed by a non-lazy source image
// that is then transformed, sampled, cropped, tiled, and/or color-filtered to produce the resolved
// image of the FilterResult. It is these actions applied to the source image that can be combined
// without producing a new intermediate "source" if it's determined that the combined actions
// rendered once would create an image close enough to the canonical output of rendering each action
// separately. Eliding offscreen renders in this way can introduce visually imperceptible pixel
// differences due to avoiding casting down to a lower precision pixel format or performing fewer
// image sampling sequences.
//
// The resolved image of a FilterResult is the output of rendering:
//
//   SkMatrix netTransform = RectToRect(fSrcRect, fDstRect);
//   netTransform.postConcat(fTransform);
//
//   SkPaint paint;
//   paint.setShader(fImage->makeShader(fTileMode, fSamplingOptions, &netTransform));
//   paint.setColorFilter(fColorFilter);
//   paint.setBlendMode(kSrc);
//
//   canvas->drawRect(fLayerBounds, paint);
//
// A FilterResult may represent the output of multiple operations affecting the different meta
// properties defined above. The operations are applied in order:
//   1. Tile the image using configured SkTileMode on the source rect.
//   2. Transform and sample (with configured SkSamplingOptions) from source rect up to the dest
//      rect and then any additional transform.
//   3. Apply any SkColorFilter to all pixels from #2 (including transparent black pixels resulting
//      from decal sampling).
//   4. Restrict the result to the layer bounds.
//
// If a new operation applied to a FilterResult does not respect this order, or cannot be modified
// to be re-ordered in place (e.g. modify fSrcRect/fDstRect instead of fLayerBounds for a crop),
// then the FilterResult must be resolved and the new operation applied to a clean slate. If it can
// be applied while respecting the order of operations than the action is free and no new
// intermediate image is produced.
//
// NOTE: The above comment reflects the end goal of the in-progress FilterResult. Currently
// SkSpecialImage is used, which internally has a subset property (its fSrcRect) and always has an
// fDstRect equal to (0,0,subset WH). Tile modes haven't been implemented yet and kDecal
// is always assumed; Color filters have also not been implemented yet.
class FilterResult {
public:
    FilterResult() : FilterResult(nullptr) {}

    explicit FilterResult(sk_sp<SkSpecialImage> image)
            : FilterResult(std::move(image), LayerSpace<SkIPoint>({0, 0})) {}

    FilterResult(std::pair<sk_sp<SkSpecialImage>, LayerSpace<SkIPoint>> imageAndOrigin)
            : FilterResult(std::move(std::get<0>(imageAndOrigin)), std::get<1>(imageAndOrigin)) {}

    FilterResult(sk_sp<SkSpecialImage> image, const LayerSpace<SkIPoint>& origin)
            : fImage(std::move(image))
            , fSamplingOptions(kDefaultSampling)
            , fTileMode(SkTileMode::kDecal)
            , fTransform(SkMatrix::Translate(origin.x(), origin.y()))
            , fColorFilter(nullptr)
            , fLayerBounds(
                    fTransform.mapRect(LayerSpace<SkIRect>(fImage ? fImage->dimensions()
                                                                  : SkISize{0, 0}))) {}

    // Renders the 'pic', clipped by 'cullRect', into an optimally sized surface (depending on
    // picture bounds and 'ctx's desired output). The picture is transformed by the context's
    // layer matrix. 'pic' must not be null.
    static FilterResult MakeFromPicture(const Context& ctx,
                                        sk_sp<SkPicture> pic,
                                        ParameterSpace<SkRect> cullRect);

    // Renders 'shader' into a surface that fills the context's desired output bounds, 'shader' must
    // not be null.
    // TODO: Update 'dither' to SkImageFilters::Dither, but that cannot be forward declared at the
    // moment because SkImageFilters is a class and not a namespace.
    static FilterResult MakeFromShader(const Context& ctx,
                                       sk_sp<SkShader> shader,
                                       bool dither);

    // Converts image to a FilterResult. If 'srcRect' is pixel-aligned it does so without rendering.
    // Otherwise it draws the src->dst sampling of 'image' into an optimally sized surface based
    // on the context's desired output. 'image' must not be null.
    static FilterResult MakeFromImage(const Context& ctx,
                                      sk_sp<SkImage> image,
                                      const SkRect& srcRect,
                                      const ParameterSpace<SkRect>& dstRect,
                                      const SkSamplingOptions& sampling);

    // Bilinear is used as the default because it can be downgraded to nearest-neighbor when the
    // final transform is pixel-aligned, and chaining multiple bilinear samples and transforms is
    // assumed to be visually close enough to sampling once at highest quality and final transform.
    static constexpr SkSamplingOptions kDefaultSampling{SkFilterMode::kLinear};

    explicit operator bool() const { return SkToBool(fImage); }

    // TODO(michaelludwig): Given the planned expansion of FilterResult state, it might be nice to
    // pull this back and not expose anything other than its bounding box. This will be possible if
    // all rendering can be handled by functions defined on FilterResult.
    const SkSpecialImage* image() const { return fImage.get(); }
    sk_sp<SkSpecialImage> refImage() const { return fImage; }

    // Get the layer-space bounds of the result. This will incorporate any layer-space transform.
    LayerSpace<SkIRect> layerBounds() const { return fLayerBounds; }
    SkTileMode tileMode() const { return fTileMode; }
    SkSamplingOptions sampling() const { return fSamplingOptions; }

    const SkColorFilter* colorFilter() const { return fColorFilter.get(); }

    // Produce a new FilterResult that has been cropped to 'crop', taking into account the context's
    // desired output. When possible, the returned FilterResult will reuse the underlying image and
    // adjust its metadata. This will depend on the current transform and tile mode as well as how
    // the crop rect intersects this result's layer bounds.
    FilterResult applyCrop(const Context& ctx,
                           const LayerSpace<SkIRect>& crop,
                           SkTileMode tileMode=SkTileMode::kDecal) const;

    // Produce a new FilterResult that is the transformation of this FilterResult. When this
    // result's sampling and transform are compatible with the new transformation, the returned
    // FilterResult can reuse the same image data and adjust just the metadata.
    FilterResult applyTransform(const Context& ctx,
                                const LayerSpace<SkMatrix>& transform,
                                const SkSamplingOptions& sampling) const;

    // Produce a new FilterResult that is visually equivalent to the output of the SkColorFilter
    // evaluating this FilterResult. If the color filter affects transparent black, the returned
    // FilterResult can become non-empty even if the input were empty.
    FilterResult applyColorFilter(const Context& ctx,
                                  sk_sp<SkColorFilter> colorFilter) const;

    // Extract image and origin, safely when the image is null. If there are deferred operations
    // on FilterResult (such as tiling or transforms) not representable as an image+origin pair,
    // the returned image will be the resolution resulting from that metadata and not necessarily
    // equal to the original 'image()'.
    // TODO (michaelludwig) - This is intended for convenience until all call sites of
    // SkImageFilter_Base::filterImage() have been updated to work in the new type system
    // (which comes later as SkDevice, SkCanvas, etc. need to be modified, and coordinate space
    // tagging needs to be added).
    sk_sp<SkSpecialImage> imageAndOffset(const Context& ctx, SkIPoint* offset) const;
    // TODO (michaelludwig) - This is a more type-safe version of the above imageAndOffset() and
    // may need to remain to support SkBlurImageFilter calling out to the SkBlurEngine. An alternate
    // option would be for FilterResult::Builder to have a blur() function that internally can
    // resolve the input and pass to the skif::Context's blur engine. Then imageAndOffset() can go
    // away entirely.
    std::pair<sk_sp<SkSpecialImage>, LayerSpace<SkIPoint>> imageAndOffset(const Context& ctx) const;

     // Draw this FilterResult into 'target' by applying the remaining layer-to-device transform of
     // 'mapping', using the provided 'blender' to composite the effective image on top of 'target'.
     // If 'blender' is null, it's equivalent to kSrcOver blending.
    void draw(const Context& ctx, SkDevice* target, const SkBlender* blender) const;

    class Builder;

    enum class ShaderFlags : int {
        kNone = 0,
        // A hint that the input FilterResult will be sampled repeatedly per pixel. If there's
        // colorspace conversions or deferred color filtering, it's worth resolving to a temporary
        // image so that those calculations are performed once per pixel instead of N times.
        kSampledRepeatedly = 1 << 0,
        // Specifies that the shader performs non-trivial operations on its coordinates to determine
        // how to sample any input FilterResults, so their sampling options should not be converted
        // to nearest-neighbor even if they appeared pixel-aligned with the output surface.
        kNonTrivialSampling = 1 << 1,
        // TODO: Add option to convey that the output can carry input tiling forward to make a
        // smaller backing surface somehow. May not be a flag and just args passed to eval().
    };
    SK_DECL_BITMASK_OPS_FRIENDS(ShaderFlags)

private:
    friend class ::FilterResultTestAccess; // For testing draw() and asShader()

    // Renders this FilterResult into a new, but visually equivalent, image that fills 'dstBounds',
    // has default sampling, no color filter, and a transform that translates by only 'dstBounds's
    // top-left corner. 'dstBounds' is intersected with 'fLayerBounds' unless 'preserveTransparency'
    // is true.
    std::pair<sk_sp<SkSpecialImage>, LayerSpace<SkIPoint>>
    resolve(const Context& ctx, LayerSpace<SkIRect> dstBounds,
            bool preserveTransparency=false) const;

    enum class BoundsAnalysis : int {
        // The image can be drawn directly, without needing to apply tiling, or handling how any
        // color filter might affect transparent black.
        kSimple = 0,
        // The image's tiling or color filter modify pixels beyond the image and those regions are
        // visible when rendering to the 'dstBounds'.
        kEffectsVisible = 1 << 0,
        // The crop boundary induced by `fLayerBounds` is visible when rendering to the 'dstBounds',
        // although this could be either because it intersects the image's content or because the
        // effects modify transparent black and fill out to the layer bounds.
        kLayerCropVisible = 1 << 1
    };
    SK_DECL_BITMASK_OPS_FRIENDS(BoundsAnalysis)

    // Determine what effects are visible based on the target 'dstBounds' and extra transform that
    // will be applied when this FilterResult is drawn. These are not LayerSpace because the
    // 'xtraTransform' may be either a within-layer transform, or a layer-to-device space transform.
    // The 'dstBounds' should be in the same coordinate space that 'xtraTransform' maps to. When
    // that is the identity matrix, 'dstBounds' is in layer space.
    //
    // Set 'blendAffectsTransparentBlack' to true when drawing a FilterResult with the non-default
    // src-over blend and the blend modifies transparent black.
    SkEnumBitMask<BoundsAnalysis> analyzeBounds(const SkMatrix& xtraTransform,
                                                const SkIRect& dstBounds,
                                                bool blendAffectsTransparentBlack) const;
    SkEnumBitMask<BoundsAnalysis> analyzeBounds(const LayerSpace<SkIRect>& dstBounds) const {
        return this->analyzeBounds(SkMatrix::I(), SkIRect(dstBounds),
                                   /*blendAffectsTransparentBlack=*/false);
    }

    // Draw directly to the device, which draws the same image as produced by resolve() but can be
    // useful if multiple operations need to be performed on the canvas.
    //
    // This assumes that the device's transform is set to match the current layer space coordinate
    // system. This will concat any internal extra transform and apply clipping as necessary. If
    // 'preserveDeviceState' is true it will undo any modifications. This can be set to false if the
    // device is a one-off that will be snapped to an image after this returns.
    //
    // If 'blender' is null, the filter result is drawn with src-over blending. If it's not, it will
    // be drawn using the given 'blender', filling the device's current clip when the blend
    // modifies transparent black.
    void draw(const Context& ctx,
              SkDevice* device,
              bool preserveDeviceState,
              const SkBlender* blender=nullptr) const;

    // Returns the FilterResult as a shader, ideally without resolving to an axis-aligned image.
    // 'xtraSampling' is the sampling that any parent shader applies to the FilterResult.
    // 'sampleBounds' is the bounding box of coords the shader will be evaluated at by any parent.
    sk_sp<SkShader> asShader(const Context& ctx,
                             const SkSamplingOptions& xtraSampling,
                             SkEnumBitMask<ShaderFlags> flags,
                             const LayerSpace<SkIRect>& sampleBounds) const;

    // Safely updates fTileMode, doing nothing if the FilterResult is empty. Updates the layer
    // bounds to the context's desired output if the tilemode is not decal.
    void updateTileMode(const Context& ctx, SkTileMode tileMode);

    // The effective image of a FilterResult is 'fImage' sampled by 'fSamplingOptions' and
    // respecting 'fTileMode' (on the SkSpecialImage's subset), transformed by 'fTransform',
    // filtered by 'fColorFilter', and then clipped to 'fLayerBounds'.
    sk_sp<SkSpecialImage> fImage;
    SkSamplingOptions     fSamplingOptions;
    SkTileMode            fTileMode;
    // Typically this will be an integer translation that encodes the origin of the top left corner,
    // but can become more complex when combined with applyTransform().
    LayerSpace<SkMatrix>  fTransform;

    // A null color filter is the identity function. Since the output is clipped to fLayerBounds
    // after color filtering, SkColorFilters that affect transparent black are not unbounded.
    sk_sp<SkColorFilter>  fColorFilter;

    // The layer bounds are initially fImage's dimensions mapped by fTransform. As the filter result
    // is processed by the image filter DAG, it can be further restricted by crop rects or the
    // implicit desired output at each node.
    LayerSpace<SkIRect>   fLayerBounds;
};
SK_MAKE_BITMASK_OPS(FilterResult::ShaderFlags)
SK_MAKE_BITMASK_OPS(FilterResult::BoundsAnalysis)

// A FilterResult::Builder is used to render one or more FilterResults or other sources into
// a new FilterResult. It automatically aggregates the incoming bounds to minimize the output's
// layer bounds.
class FilterResult::Builder {
public:
    Builder(const Context& context);
    ~Builder();

    // If 'sampleBounds' is not provided, it defaults to the output bounds calculated for eval()
    // (generally the Context's desired output but could be restricted based on the ShaderFlags).
    //
    // If it is provided, it represents the bounding box of possible coords 'input' will be sampled
    // at by the shader created from eval(). This can be useful to provide when the shader does non
    // trivial sampling since it may avoid having to resolve a FilterResult to an image.
    //
    // The 'inputFlags' are per-input flags that are OR'ed with the ShaderFlag mask passed to
    // eval() to control how 'input' is converted to an SkShader. 'inputSampling' specifies the
    // sampling options to use on the input's image when sampled by the final shader created in eval
    //
    // 'sampleBounds', 'inputFlags' and 'inputSampling' must not be used with merge() or blur().
    Builder& add(const FilterResult& input,
                 std::optional<LayerSpace<SkIRect>> sampleBounds = {},
                 SkEnumBitMask<ShaderFlags> inputFlags = ShaderFlags::kNone,
                 const SkSamplingOptions& inputSampling = kDefaultSampling) {
        fInputs.push_back({input, sampleBounds, inputFlags, inputSampling});
        return *this;
    }

    // Combine all added inputs by merging them with src-over blending into a single output.
    FilterResult merge();

    // Blur the single input with a Gaussian blur. The exact blur implementation is chosen based on
    // the skif::Context's backend. The sample bounds of the input and the final output bounds are
    // automatically derived from the sigma, input layer bounds, and desired output bounds of the
    // Builder's Context.
    FilterResult blur(const LayerSpace<SkSize>& sigma);

    // Combine all added inputs by transforming them into equivalent SkShaders and invoking the
    // shader factory that binds them together into a single shader that fills the output surface.
    //
    // 'ShaderFn' should be an invokable type with the signature
    //     (SkSpan<sk_sp<SkShader>>)->sk_sp<SkShader>
    // The length of the span will equal the number of FilterResults added to the builder. If an
    // input FilterResult was fully transparent, its corresponding shader will be null. 'ShaderFn'
    // should return a null shader its output would be fully transparent.
    //
    // By default, the returned FilterResult will fill the Context's desired image. If
    // 'explicitOutput' has a value, it is intersected with the Context's desired output bounds to
    // produce a possibly restricted output surface that the evaluated shader is rendered into.
    //
    // The shader created by `ShaderFn` will by default be invoked with coordinates in the layer
    // space of the Context. If `evaluateInParameterSpace` is true, the drawing matrix will be
    // adjusted so that the shader processes coordinates mapped back into parameter space (the
    // underlying output is still in layer space). In this case, it's assumed that the shaders for
    // the added FilterResult inputs will be evaluated with coordinates also in parameter space,
    // so they will be adjusted to map back to layer space before sampling their underlying images.
    template <typename ShaderFn>
    FilterResult eval(ShaderFn shaderFn,
                      std::optional<LayerSpace<SkIRect>> explicitOutput = {},
                      bool evaluateInParameterSpace=false) {
        auto outputBounds = this->outputBounds(explicitOutput);
        if (outputBounds.isEmpty()) {
            return {};
        }

        auto inputShaders = this->createInputShaders(outputBounds, evaluateInParameterSpace);
        return this->drawShader(shaderFn(inputShaders), outputBounds, evaluateInParameterSpace);
    }

private:
    struct SampledFilterResult {
        FilterResult fImage;
        std::optional<LayerSpace<SkIRect>> fSampleBounds;
        SkEnumBitMask<ShaderFlags> fFlags;
        SkSamplingOptions fSampling;
    };

    SkSpan<sk_sp<SkShader>> createInputShaders(const LayerSpace<SkIRect>& outputBounds,
                                               bool evaluateInParameterSpace);

    LayerSpace<SkIRect> outputBounds(std::optional<LayerSpace<SkIRect>> explicitOutput) const;

    FilterResult drawShader(sk_sp<SkShader> shader,
                            const LayerSpace<SkIRect>& outputBounds,
                            bool evaluateInParameterSpace) const;

    const Context& fContext; // Must outlive the builder
    skia_private::STArray<1, SampledFilterResult> fInputs;
    // Lazily created once all inputs are collected, but parallels fInputs.
    skia_private::STArray<1, sk_sp<SkShader>> fInputShaders;
};

// The backend provides key functionality to the image filtering pipeline that must be implemented
// by the Skia backend (e.g. raster or GPU). While a Context's state may change as the image filter
// DAG is evaluated, a given filter invocation will always use one Backend.
class Backend : public SkRefCnt {
public:
    ~Backend() override;

    // For creating offscreen intermediate renderable images
    virtual sk_sp<SkDevice> makeDevice(SkISize size,
                                       sk_sp<SkColorSpace>,
                                       const SkSurfaceProps* props=nullptr) const = 0;

    // For input images to be processed by image filters
    virtual sk_sp<SkSpecialImage> makeImage(const SkIRect& subset, sk_sp<SkImage> image) const = 0;

    // For internal data to be accessed by filter implementations
    virtual sk_sp<SkImage> getCachedBitmap(const SkBitmap& data) const = 0;

    // For backend-optimized blurring implementations (TODO: Possibly replaced by a SkBlurEngine).
    // The srcRect and dstRect are relative to (0,0) of 'input's logical image (which may have its
    // own offset to backing data). The returned image should have a width and height equal to the
    // dstRect's dimensions and its (0,0) pixel is assumed to be located at dstRect.topLeft().
    virtual sk_sp<SkSpecialImage> blur(SkSize sigma,
                                       sk_sp<SkSpecialImage> input,
                                       SkIRect srcRect,
                                       SkIRect dstRect,
                                       sk_sp<SkColorSpace>) const = 0;

    // Temporary, until SkBlurImageFilter always delegates to FilterResult::blur()
    virtual bool isBlurSupported() const = 0;

    // Properties controlling the pixel data for offscreen surfaces rendered to during filtering.
    const SkSurfaceProps& surfaceProps() const { return fSurfaceProps; }
    SkColorType colorType() const { return fColorType; }

    SkImageFilterCache* cache() const { return fCache.get(); }

protected:
    Backend(sk_sp<SkImageFilterCache> cache,
            const SkSurfaceProps& surfaceProps,
            const SkColorType colorType);

private:
    sk_sp<SkImageFilterCache> fCache;
    SkSurfaceProps fSurfaceProps;
    SkColorType fColorType;
};

sk_sp<Backend> MakeRasterBackend(const SkSurfaceProps& surfaceProps, SkColorType colorType);

// The context contains all necessary information to describe how the image filter should be
// computed (i.e. the current layer matrix and clip), and the color information of the output of a
// filter DAG. For now, this is just the color space (of the original requesting device). This is
// used when constructing intermediate rendering surfaces, so that we ensure we land in a surface
// that's similar/compatible to the final consumer of the DAG's output.
class Context {
public:
    Context(sk_sp<Backend> backend,
            const Mapping& mapping,
            const LayerSpace<SkIRect>& desiredOutput,
            const FilterResult& source,
            const SkColorSpace* colorSpace)
        : fBackend(std::move(backend))
        , fMapping(mapping)
        , fDesiredOutput(desiredOutput)
        , fSource(source)
        , fColorSpace(sk_ref_sp(colorSpace)) {}

    const Backend* backend() const { return fBackend.get(); }

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

    // The bounds, in the layer space, that the filtered image will be clipped to. The output
    // from filterImage() must cover these clip bounds, except in areas where it will just be
    // transparent black, in which case a smaller output image can be returned.
    const LayerSpace<SkIRect>& desiredOutput() const { return fDesiredOutput; }

    // The output device's color space, so intermediate images can match, and so filtering can
    // be performed in the destination color space.
    SkColorSpace* colorSpace() const { return fColorSpace.get(); }
    sk_sp<SkColorSpace> refColorSpace() const { return fColorSpace; }

    // This is the image to use whenever an expected input filter has been set to null. In the
    // majority of cases, this is the original source image for the image filter DAG so it comes
    // from the SkDevice that holds either the saveLayer or the temporary rendered result. The
    // exception is composing two image filters (via SkImageFilters::Compose), which must use
    // the output of the inner DAG as the "source" for the outer DAG.
    const FilterResult& source() const { return fSource; }


    // Create a new context that matches this context, but with an overridden layer space.
    Context withNewMapping(const Mapping& mapping) const {
        Context c = *this;
        c.fMapping = mapping;
        return c;
    }
    // Create a new context that matches this context, but with an overridden desired output rect.
    Context withNewDesiredOutput(const LayerSpace<SkIRect>& desiredOutput) const {
        Context c = *this;
        c.fDesiredOutput = desiredOutput;
        return c;
    }
    // Create a new context that matches this context, but with an overridden color space.
    Context withNewColorSpace(SkColorSpace* cs) const {
        Context c = *this;
        c.fColorSpace = sk_ref_sp(cs);
        return c;
    }

    // Create a new context that matches this context, but with an overridden source.
    Context withNewSource(const FilterResult& source) const {
        Context c = *this;
        c.fSource = source;
        return c;
    }

private:
    sk_sp<Backend> fBackend;

    // Properties controlling the size and coordinate space of image filtering
    Mapping             fMapping;
    LayerSpace<SkIRect> fDesiredOutput;
    // Can contain a null image if the image filter DAG has no late-bound null inputs.
    FilterResult        fSource;
    // The color space the filters are evaluated in
    sk_sp<SkColorSpace> fColorSpace;
};

} // end namespace skif

#endif // SkImageFilterTypes_DEFINED
