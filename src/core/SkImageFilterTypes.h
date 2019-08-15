/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageFilterTypes_DEFINED
#define SkImageFilterTypes_DEFINED

#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"

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

// Image filter implementations need to track geometry across a number of coordinate systems. While
// the base Sk[I]Rect and Sk[I]Point types are perfectly suited to represent this data,
// implementations would quickly become difficult to read when there are multiple variables of the
// same type that are defined relative to distinct systems. In particular, when operating on those
// coordinates, are the actions part of transforming from one space to another, or part of the
// effects of the image filter. This template tag enum, in conjunction with the geometric wrappers
// provided below help mitigate these issues and ensure correctness when converting from one space
// to another.
enum class CoordSpace {
    // The coordinate system specific to an image filter and its spatial parameters, such as offset
    // vector or blur radius. Unless specified otherwise (e.g., the ‘x’ parameter to the
    // ‘yImageFilter’)parameters are mapped from this space into kLayerSpace using the portion of
    // the CTM in filter context (i.e., the layer matrix). If a blur had a sigma of 2 for both x and
    // y axes, with a scaling matrix (Sx=2,Sy=3), the layer-space blur would have sigmas = (4,6).
    kParameter,
    // The space where filtering occurs aligned to the x & y axes. The layer matrix of the filtering
    // context transforms from a filter’s parameter space to the layer space. As a final step, when
    // drawing the filtered result to its destination, layer space can be mapped to device space via
    // the remainder matrix held by the SkCanvas.
    kLayer,
    // The final destination space for the filtered image, represented by the total matrix of the
    // SkCanvas. This should only be relevant in the final draw of the filtered result.
    kDevice,
    // The pixel space for an image as input or output in part of the filtering process. An offset
    // vector maps its top left corner into layer space and is returned with the output of each
    // filter node in the DAG. If we blurred a 10x10 image we would have a 16x16 image space and a
    // -3,-3..13,13 layer space (with the stupid offset being -3,-3).
    kImage,
    // The real pixel space that holds a logical image. An offset maps the top left logical pixel to
    // a real pixel in the backing proxy. The proxy’s size will be at least large enough to contain
    // the offset logical image. This space captures both the loose-fitted-ness of special images
    // and subsetted special images. If a filter needed to output a 12x12 image and there was a
    // 16x16 scratch texture available, it could be used as an approx. fit proxy. If this image then
    // needed to be cropped, its subset coordinates could be modified without copying to a new
    // buffer.
    kProxy
};

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
    // Designates that the semantic use of the bounds is inherited from the coordinate system of the
    // geometry. This is intended for use with coordinate systems that have a single inherent
    // purpose (e.g. kDevice or kParameter, or kLayer when the geometry is not attached to a
    // specific image).
    kInherit
};

// Convenience macros to add 'using' declarations that rename the above enums to provide a more
// fluent and readable API. This should only be used in a private or local scope to prevent leakage
// of the names. Use the IN_CLASS variant at the start of a class declaration in those scenarios.
// These macros enable the following simpler type names:
//   skif::Image<skif::Usage::kInput> -> skif::Image<For::kInput> or
//   skif::IRect<skif::CoordSpace::kLayer, skif::Usage::kInput> ->
//       skif::IRect<In::kLayer, For::kInput>
#define SK_USE_FLUENT_IMAGE_FILTER_TYPES \
    using In  = skif::CoordSpace;        \
    using For = skif::Usage;

#define SK_USE_FLUENT_IMAGE_FILTER_TYPES_IN_CLASS \
    protected: SK_USE_FLUENT_IMAGE_FILTER_TYPES public:

///////////////////////////////////////////////////////////////////////////////////////////////////
// Internal helpers to build up the geometry tagging system
template<CoordSpace, Usage> class IPoint;

// Impose limits on the combination of coordinate space and usage that is common to
// all geometries.
#define STATIC_COORD_SPACE_CONSTRAINTS \
    static_assert(!(kCS == CoordSpace::kParameter || kCS == CoordSpace::kDevice) || \
                  kU == Usage::kInherit,                                            \
                  "kParameter and kDevice space must use kInherit as their usage"); \
    static_assert(!(kCS == CoordSpace::kImage || kCS == CoordSpace::kProxy) ||      \
                  kU != Usage::kInherit,                                            \
                  "kImage and kProxy space cannot use kInherit for their usage");
// Friend declarations for the top-level coordinate system functions, which can be used to grant
// them access to the underlying data and basis/origin mapping functions (which shouldn't be
// be public because they are only meant to be used under specific combinations of space/usage).
#define FRIENDS_COORD_SPACE_FUNCTIONS \
    template<Usage kF, template<CoordSpace, Usage = Usage::kInherit> class Geometry>        \
    friend Geometry<CoordSpace::kLayer, kF> ParameterToLayer(                               \
            const Geometry<CoordSpace::kParameter>& geom, const SkMatrix& layerMatrix);     \
    template<Usage kF, template<CoordSpace, Usage = Usage::kInherit> class Geometry>        \
    friend Geometry<CoordSpace::kDevice> LayerToDevice(                                     \
            const Geometry<CoordSpace::kLayer, kF>& geom, const SkMatrix& remainderMatrix); \
    template<Usage kF, template<CoordSpace, Usage = Usage::kInherit> class Geometry>        \
    friend Geometry<CoordSpace::kLayer, kF> DeviceToLayer(                                  \
            const Geometry<CoordSpace::kDevice>& geom, const SkMatrix& remainderMatrix);    \
    template<Usage kF, template<CoordSpace, Usage> class Geometry>                          \
    friend Geometry<CoordSpace::kImage, kF> LayerToImage(                                   \
            const Geometry<CoordSpace::kLayer, kF>& geom,                                   \
            const IPoint<CoordSpace::kLayer, kF>& origin);                                  \
    template<Usage kF, template<CoordSpace, Usage> class Geometry>                          \
    friend Geometry<CoordSpace::kLayer, kF> ImageToLayer(                                   \
            const Geometry<CoordSpace::kImage, kF>& geom,                                   \
            const IPoint<CoordSpace::kLayer, kF>& origin);                                  \
    template<Usage kF, template<CoordSpace, Usage> class Geometry>                          \
    friend Geometry<CoordSpace::kProxy, kF> ImageToProxy(                                   \
            const Geometry<CoordSpace::kImage, kF>& geom,                                   \
            const IPoint<CoordSpace::kProxy, kF>& origin);

///////////////////////////////////////////////////////////////////////////////////////////////////

// Coordinate system-tagged wrappers around the  key primitive Skia geometric types. A class is
// defined to wrap each of SkIVector, SkVector, SkIPoint, SkPoint, SkIRect, and SkRect:
//   SkIRect   -> skif::IRect<kCS, kU>
//   SkRect    -> skif::Rect<kCS, kU>
//   SkIPoint  -> skif::IPoint<kCS, kU>
//   SkPoint   -> skif::Point<kCS, kU>
//   SkIVector -> skif::IVector<kCS, kU> (same base type, but acts automatically as a direction)
//   SkVector  -> skif::Vector<kCS, kU> (same base type, but acts automatically as a direction)
//
// Static asserts enforce when the optional usage parameter must be specified, so that the valid set
// of coordinate system and usage combinations is::
//   Geom<In::kParameter>                - geometry stored by an SkImageFilter, e.g. a crop rect
//   Geom<In::kLayer>                    - for layer-space calculations that aren’t associated with
//                                         an input or output
//   Geom<In::kLayer, For::kOutput>      - e.g., the layer-space bounds of the output image
//   Geom<In::kImage, For::kOutput>      - e.g., a subset of pixels of the output image
//   Geom<In::kProxy, For::kOutput>      - a subset of pixels of the output image, relative to its
//                                         actual backing proxy (primarily used for gpu filtering)
//   Geom<In::kLayer, For::kInput[|0|1]> - e.g., the layer-space bounds of the possibly slot
//                                         specific input image
//   Geom<In::kImage, For::kInput[|0|1]> - a subset of pixels of the input image
//   Geom<In::kProxy, For::kInput[|0|1]> - a subset of pixels of the input image, relative to its
//                                         actual backing proxy (primarily used for gpu filtering)
// Every Geom type follows common pattern:
//     template<CoordSpace kCS, Usage kU = Usage::kInherit>
//     class Geom {
//     public:
//         using SkType = <foo>;
//         Geom() = default;
//         explicit Geom(const <foo>& geom);
//         explicit Geom(<foo>&& geom);
//
//         explicit operator const <foo>&() const;
//
//         // ... then CS-tagged APIs that match the useful bits in <foo>
//     private:
//         <foo> fData;
//
//         template<CoordSpace kCSOut>
//         Geom<kCSOut, kU> mapBasis(const SkMatrix& matrix) const;
//         template<CoordSpace kCSOut>
//         Geom<kCSOut, kU> mapOrigin(const SkIPoint& origin) const;
//    };
//
// Each type has the private mapBasis and mapOrigin functions defined so that the top-level
// coordinate system transformation functions can be written generically. Six separate types are
// used instead of a common template because each wrapper has a unique API for operating on the
// geometry in a typesafe manner.

/*
 * Tagged wrapper around SkIVector. Technically SkIVector and SkIPoint are the same. This wrapping
 * type enforces it is used as a direction-only vector.
 */
template<CoordSpace kCS, Usage kU = Usage::kInherit>
class IVector {
public:
    STATIC_COORD_SPACE_CONSTRAINTS

    // The underlying Skia math type
    using SkType = SkIVector;

    IVector() = default;
    explicit IVector(const SkIVector& geometry)
            : fData(geometry) {}
    explicit IVector(SkIVector&& geometry)
            : fData(std::move(geometry)) {}

    explicit operator const SkIVector&() const { return fData; }

    // Parrot the SkIVector API while preserving coord space and usage
    int32_t x() const { return fData.fX; }
    int32_t y() const { return fData.fY; }

    IVector<kCS, kU> operator-() const { return IVector<kCS, kU>(-fData); }

    IVector<kCS, kU> operator+(const IVector<kCS, kU>& v) {
        return IVector<kCS, kU>(fData + v.fData);
    }
    IVector<kCS, kU> operator-(const IVector<kCS, kU>& v) {
        return IVector<kCS, kU>(fData - v.fData);
    }

    void operator+=(const IVector<kCS, kU>& v) { fData += v.fData; }
    void operator-=(const IVector<kCS, kU>& v) { fData -= v.fData; }

private:
    FRIENDS_COORD_SPACE_FUNCTIONS

    template<CoordSpace kCSOut>
    IVector<kCSOut, kU> mapBasis(const SkMatrix& matrix) const {
        SkVector v = SkVector::Make(SkIntToScalar(fData.fX), SkIntToScalar(fData.fY));
        matrix.mapVectors(&v, 1);
        return IVector<kCSOut, kU>(SkIVector::Make(SkScalarCeilToInt(v.fX),
                                                   SkScalarCeilToInt(v.fY)));
    }

    template<CoordSpace kCSOut>
    IVector<kCSOut, kU> mapOrigin(const SkIPoint& origin) const {
        // If a translation is all that maps from CS to another, then the basis vectors of the space
        // are the same, so direction vectors do not change.
        return IVector<kCSOut, kU>(fData);
    }

    SkIVector fData;
};

/*
 * Tagged wrapper around SkVector. Technically SkVector and SkPoint are the same. This wrapping
 * type enforces it is used as a direction-only vector.
 */
template<CoordSpace kCS, Usage kU = Usage::kInherit>
class Vector {
public:
    STATIC_COORD_SPACE_CONSTRAINTS

    // The underlying Skia math type
    using SkType = SkVector;

    Vector() = default;
    explicit Vector(const SkVector& geometry)
            : fData(geometry) {}
    explicit Vector(SkVector&& geometry)
            : fData(std::move(geometry)) {}

    explicit operator const SkVector&() const { return fData; }

    // Parrot the SkVector API while preserving coord space and usage
    SkScalar x() const { return fData.fX; }
    SkScalar y() const { return fData.fY; }
    SkScalar length() const { return fData.length(); }

    Vector<kCS, kU> operator-() const { return Vector<kCS, kU>(-fData); }

    Vector<kCS, kU> operator*(SkScalar s) const { return Vector<kCS, kU>(fData * s); }
    Vector<kCS, kU> operator+(const Vector<kCS, kU>& v) { return Vector<kCS, kU>(fData + v.fData); }
    Vector<kCS, kU> operator-(const Vector<kCS, kU>& v) { return Vector<kCS, kU>(fData - v.fData); }

    void operator*=(SkScalar s) { fData *= s; }
    void operator+=(const Vector<kCS, kU>& v) { fData += v.fData; }
    void operator-=(const Vector<kCS, kU>& v) { fData -= v.fData; }

    friend Vector<kCS, kU> operator*(SkScalar s, const Vector<kCS, kU>& b) {
        return b * s;
    }

private:
    FRIENDS_COORD_SPACE_FUNCTIONS

    template<CoordSpace kCSOut>
    Vector<kCSOut, kU> mapBasis(const SkMatrix& matrix) const {
        SkVector v;
        matrix.mapVectors(&v, &fData, 1);
        return Vector<kCSOut, kU>(v);
    }

    template<CoordSpace kCSOut>
    Vector<kCSOut, kU> mapOrigin(const SkIPoint& origin) const {
        // If a translation is all that maps from CS to another, then the basis vectors of the space
        // are the same, so direction vectors do not change.
        return Vector<kCSOut, kU>(fData);
    }

    SkVector fData;
};

/*
 * Tagged wrapper around SkIPoint. Technically SkIVector and SkIPoint are the same. This wrapping
 * type enforces it is used as a position in space.
 */
template<CoordSpace kCS, Usage kU = Usage::kInherit>
class IPoint {
public:
    STATIC_COORD_SPACE_CONSTRAINTS

    // The underlying Skia math type
    using SkType = SkIPoint;

    IPoint() = default;
    explicit IPoint(const SkIPoint& geometry)
            : fData(geometry) {}
    explicit IPoint(SkIPoint&& geometry)
            : fData(std::move(geometry)) {}

    explicit operator const SkIPoint&() const { return fData; }

    // Parrot the SkIPoint API while preserving coord space and usage
    int32_t x() const { return fData.fX; }
    int32_t y() const { return fData.fY; }

    // Adding/subtracting vectors makes a new point
    IPoint<kCS, kU> operator+(const IVector<kCS, kU>& v) {
        return IPoint<kCS, kU>(fData + v.fData);
    }
    IPoint<kCS, kU> operator-(const IVector<kCS, kU>& v) {
        return IPoint<kCS, kU>(fData - v.fData);
    }

    void operator+=(const IVector<kCS, kU>& v) { fData += v.fData; }
    void operator-=(const IVector<kCS, kU>& v) { fData -= v.fData; }

    // Subtracting another point makes the vector between them
    IVector<kCS, kU> operator-(const IPoint<kCS, kU>& p) {
        return IVector<kCS, kU>(fData - p.fData);
    }

private:
    FRIENDS_COORD_SPACE_FUNCTIONS

    template<CoordSpace kCSOut>
    IPoint<kCSOut, kU> mapBasis(const SkMatrix& matrix) const {
        SkPoint p = SkPoint::Make(SkIntToScalar(fData.fX), SkIntToScalar(fData.fY));
        matrix.mapPoints(&p, 1);
        return IPoint<kCSOut, kU>(SkIPoint::Make(SkScalarCeilToInt(p.fX), SkScalarCeilToInt(p.fY)));
    }

    template<CoordSpace kCSOut>
    IPoint<kCSOut, kU> mapOrigin(const SkIPoint& origin) const {
        return IPoint<kCSOut, kU>(fData + origin);
    }

    SkIPoint fData;
};

/*
 * Tagged wrapper around SkPoint. Technically SkVector and SkPoint are the same. This wrapping
 * type enforces it is used as a position in space.
 */
template<CoordSpace kCS, Usage kU = Usage::kInherit>
class Point {
public:
    STATIC_COORD_SPACE_CONSTRAINTS

    // The underlying Skia math type
    using SkType = SkPoint;

    Point() = default;
    explicit Point(const SkPoint& geometry)
            : fData(geometry) {}
    explicit Point(SkPoint&& geometry)
            : fData(std::move(geometry)) {}

    explicit operator const SkPoint&() const { return fData; }

    // Parrot the SkPoint API while preserving coord space and usage
    SkScalar x() const { return fData.fX; }
    SkScalar y() const { return fData.fY; }
    SkScalar distanceToOrigin() const { return fData.distanceToOrigin(); }

    // Adding/subtracting vectors makes a new point
    Point<kCS, kU> operator+(const Vector<kCS, kU>& v) {
        return Point<kCS, kU>(fData + v.fData);
    }
    Point<kCS, kU> operator-(const Vector<kCS, kU>& v) {
        return Point<kCS, kU>(fData - v.fData);
    }

    void operator+=(const Vector<kCS, kU>& v) { fData += v.fData; }
    void operator-=(const Vector<kCS, kU>& v) { fData -= v.fData; }

    // Subtracting another point makes the vector between them
    Vector<kCS, kU> operator-(const Point<kCS, kU>& p) {
        return Vector<kCS, kU>(fData - p.fData);
    }

private:
    FRIENDS_COORD_SPACE_FUNCTIONS

    template<CoordSpace kCSOut>
    Point<kCSOut, kU> mapBasis(const SkMatrix& matrix) const {
        SkPoint p;
        matrix.mapPoints(&p, &fData, 1);
        return Point<kCSOut, kU>(p);
    }

    template<CoordSpace kCSOut>
    Point<kCSOut, kU> mapOrigin(const SkIPoint& origin) const {
        return Point<kCSOut, kU>(fData + SkPoint::Make(SkIntToScalar(origin.fX),
                                                       SkIntToScalar(origin.fY)));
    }

    SkPoint fData;
};

/*
 * Tagged wrapper around SkIRect.
 */
template<CoordSpace kCS, Usage kU = Usage::kInherit>
class IRect {
public:
    STATIC_COORD_SPACE_CONSTRAINTS

    // The underlying Skia math type
    using SkType = SkIRect;

    IRect() = default;
    explicit IRect(const SkIRect& geometry)
            : fData(geometry) {}
    explicit IRect(SkIRect&& geometry)
            : fData(std::move(geometry)) {}

    explicit operator const SkIRect&() const { return fData; }

    // Parrot the SkIRect API while preserving coord space and usage
    int32_t left() const { return fData.fLeft; }
    int32_t top() const { return fData.fTop; }
    int32_t right() const { return fData.fRight; }
    int32_t bottom() const { return fData.fBottom; }
    int32_t width() const { return fData.width(); }
    int32_t height() const { return fData.height(); }

    IPoint<kCS, kU> topLeft() const { return IPoint<kCS, kU>(fData.topLeft()); }

    bool intersect(const IRect<kCS, kU>& r) { return fData.intersect(r.fData); }
    void join(const IRect<kCS, kU>& r) { fData.join(r.fData); }
    void offset(const IVector<kCS, kU>& v) { fData.offset(SkIPoint(v)); }
    void outset(int dx, int dy) { fData.outset(dx, dy); }

private:
    FRIENDS_COORD_SPACE_FUNCTIONS

    template<CoordSpace kCSOut>
    IRect<kCSOut, kU> mapBasis(const SkMatrix& matrix) const {
        return IRect<kCSOut, kU>(matrix.mapRect(SkRect::Make(fData)).roundOut());
    }

    template<CoordSpace kCSOut>
    IRect<kCSOut, kU> mapOrigin(const SkIPoint& origin) const {
        return IRect<kCSOut, kU>(fData.makeOffset(origin.fX, origin.fY));
    }

    SkIRect fData;
};

/*
 * Tagged wrapper around SkRect.
 */
template<CoordSpace kCS, Usage kU = Usage::kInherit>
class Rect {
public:
    STATIC_COORD_SPACE_CONSTRAINTS

    // The underlying Skia math type
    using SkType = SkRect;

    Rect() = default;
    explicit Rect(const SkRect& geometry)
            : fData(geometry) {}
    explicit Rect(SkRect&& geometry)
            : fData(std::move(geometry)) {}

    explicit operator const SkRect&() const { return fData; }

    // Parrot the SkRect API while preserving coord space and usage
    SkScalar left() const { return fData.fLeft; }
    SkScalar top() const { return fData.fTop; }
    SkScalar right() const { return fData.fRight; }
    SkScalar bottom() const { return fData.fBottom; }
    SkScalar width() const { return fData.width(); }
    SkScalar height() const { return fData.height(); }

    Point<kCS, kU> topLeft() const {
        return Point<kCS, kU>(SkPoint::Make(fData.fLeft, fData.fTop));
    }

    IRect<kCS, kU> roundOut() const { return IRect<kCS, kU>(fData.roundOut()); }

    bool intersect(const Rect<kCS, kU>& r) { return fData.intersect(r.fData); }
    void join(const Rect<kCS, kU>& r) { fData.join(r.fData); }
    void offset(const Vector<kCS, kU>& v) { fData.offset(SkIPoint(v)); }
    void outset(SkScalar dx, SkScalar dy) { fData.outset(dx, dy); }

private:
    FRIENDS_COORD_SPACE_FUNCTIONS

    template<CoordSpace kCSOut>
    Rect<kCSOut, kU> mapBasis(const SkMatrix& matrix) const {
        return Rect<kCSOut, kU>(matrix.mapRect(fData));
    }

    template<CoordSpace kCSOut>
    Rect<kCSOut, kU> mapOrigin(const SkIPoint& origin) const {
        return Rect<kCSOut, kU>(fData.makeOffset(SkIntToScalar(origin.fX),
                                                 SkIntToScalar(origin.fY)));
    }

    SkRect fData;
};

#undef FRIENDS_COORD_SPACE_FUNCTIONS
#undef STATIC_COORD_SPACE_CONSTRAINTS

///////////////////////////////////////////////////////////////////////////////////////////////////
// Functions to transform from one coordinate system and/or usage to another in a type safe manner.
// - While the templates are verbose here, type inference should make using these very easy.
// - Filters that need to compute new geometry should do so in the kLayer coordinate system, in
//   which case Geometry provides -> and * operators for direct access to the base Skia APIs
// - These signatures must be kept in sync with FRIENDS_COORD_SPACE_FUNCTIONS macro in order to have
//   access to the private mapBasis() and mapOrigin() functions.

// Cast from one usage to another, which is just a semantic conversion since the input and output
// geometries are defined in the shared layer coordinate space.
// NOTE: The out Usage template argument comes first so that calls can be written LayerCast<Out>()
// and then kUIn and Geometry will be inferred from the 'geom' argument. If kUOut was not first the
// compiler isn't able to sort it out.
template<Usage kUOut, Usage kUIn, template<CoordSpace, Usage> class Geometry>
Geometry<CoordSpace::kLayer, kUOut> LayerCast(const Geometry<CoordSpace::kLayer, kUIn>& geom) {
    using T = typename Geometry<CoordSpace::kLayer, kUIn>::SkType;
    return Geometry<CoordSpace::kLayer, kUOut>(T(geom));
}
template<Usage kUOut, Usage kUIn, template<CoordSpace, Usage> class Geometry>
Geometry<CoordSpace::kLayer, kUOut> LayerCast(Geometry<CoordSpace::kLayer, kUIn>&& geom) {
    using T = typename Geometry<CoordSpace::kLayer, kUIn>::SkType;
    return Geometry<CoordSpace::kLayer, kUOut>(std::move(T(geom)));
}

// Transform the parameter-space geometry into layer-space using the provided layer matrix,
// i.e. the value returned by skif::Context.layerMatrix().
template<Usage kU = Usage::kInherit, template<CoordSpace, Usage = Usage::kInherit> class Geometry>
Geometry<CoordSpace::kLayer, kU> ParameterToLayer(const Geometry<CoordSpace::kParameter>& geom,
                                                  const SkMatrix& layerMatrix) {
    // kParameter-space geometry always uses kInherit, so this needs the extra LayerCast to
    // go from <kLayer, kInherit> to <kLayer, kU>
    return LayerCast<kU>(geom.template mapBasis<CoordSpace::kLayer>(layerMatrix));
}

// Transform the layer-space geometry into device-space using the provided remainder matrix. The
// remainder matrix is the additional transformation applied to the results of the image filter
// as it is drawn into device space. The decomposition of the canvas' total CTM into a remainder and
// layer matrix is handled internally by SkCanvas.
template<Usage kU = Usage::kInherit, template<CoordSpace, Usage = Usage::kInherit> class Geometry>
Geometry<CoordSpace::kDevice> LayerToDevice(const Geometry<CoordSpace::kLayer, kU>& geom,
                                            const SkMatrix& remainderMatrix) {
    // kDevice-space geometry can only be kInherit, so have to cast from kU to kInherit before
    // mapping the geometry.
    return LayerCast<Usage::kInherit>(geom).template mapBasis<CoordSpace::kDevice>(remainderMatrix);
}

// Transform the device-space geometry into layer-space using the provided remainder matrix. The
// geometry is mapped using the inverse of the remainder, so 'geom' is returned unmodified if the
// matrix cannot be inverted. The remainder matrix is the additional transformation applied to the
// results of the image filter as it is drawn into device space. The decomposition of the canvas'
// total CTM into a remainder and layer matrix is handled internally by SkCanvas.
template<Usage kU = Usage::kInherit, template<CoordSpace, Usage = Usage::kInherit> class Geometry>
Geometry<CoordSpace::kLayer, kU> DeviceToLayer(const Geometry<CoordSpace::kDevice>& geom,
                                               const SkMatrix& remainderMatrix) {
    // kDevice-space geometry always uses kInherit, so this needs the extra LayerCast to go from
    // <kLayer, kInherit> to <kLayer, kU>
    SkMatrix inv;
    if (!remainderMatrix.invert(&inv)) {
        return Geometry<CoordSpace::kLayer, kU>(Geometry<CoordSpace::kDevice>::SkType(geom));
    } else {
        return LayerCast<kU>(geom.template mapBasis<CoordSpace::kLayer>(inv));
    }
}

// Transform the layer-space geometry to a usage-specific image space. This does not perform any
// clamping to (0, 0) or (width, height), it only translates the layer coordinates such that
// 'origin' maps to (0, 0) in the image space.
template<Usage kU, template<CoordSpace, Usage> class Geometry>
Geometry<CoordSpace::kImage, kU> LayerToImage(const Geometry<CoordSpace::kLayer, kU>& geom,
                                              const IPoint<CoordSpace::kLayer, kU>& origin) {
    return geom.template mapOrigin<CoordSpace::kImage>(-SkIPoint(origin));
}

// Transform a specific image-space geometry to the shared layer space. This does validate that the
// image pixel coordinate was within the defined content of the image, it only translates the layer
// coordinates such that the top left image pixel (0, 0) maps to 'origin'.
template<Usage kU, template<CoordSpace, Usage> class Geometry>
Geometry<CoordSpace::kLayer, kU> ImageToLayer(const Geometry<CoordSpace::kImage, kU>& geom,
                                              const IPoint<CoordSpace::kLayer, kU>& origin) {
    return geom.template mapOrigin<CoordSpace::kLayer>(SkIPoint(origin));
}

// Transform a specific image-space geometry to its underlying proxy space. This does validate that
// the image pixel coordinate was within the defined content of the image, it only translates the
// layer coordinates such that the top left image pixel (0, 0) maps to the 'origin' pixel in the
// backing proxy.
template<Usage kU, template<CoordSpace, Usage> class Geometry>
Geometry<CoordSpace::kProxy, kU> ImageToProxy(const Geometry<CoordSpace::kImage, kU>& geom,
                                              const IPoint<CoordSpace::kProxy, kU>& origin) {
    return geom.template mapOrigin<CoordSpace::kProxy>(SkIPoint(origin));
}

// Map from one specific image space to another. Unlike in layer space, this is not just a semantic
// conversion because this requires accounting for the origins of each image.
template<Usage kUOut, Usage kUIn, template<CoordSpace, Usage> class Geometry>
Geometry<CoordSpace::kImage, kUOut> MapImageSpace(
        const Geometry<CoordSpace::kImage, kUIn>& geom,
        const IPoint<CoordSpace::kLayer, kUIn>& inOrigin,
        const IPoint<CoordSpace::kLayer, kUOut>& outOrigin) {
    return LayerToImage(LayerCast<kUOut>(ImageToLayer(geom, inOrigin)), outOrigin);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

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
// skif::Context (i.e. the enum value kLayer refers to that specific space). The combination of
// an Image's Usage and the kImage or kProxy values defines a specific coordinate space. kImage
// by itself does not represent a specific image space.
template<Usage kU>
class Image {
public:
    static_assert(kU != Usage::kInherit, "Images cannot use the kInherit usage");
    SK_USE_FLUENT_IMAGE_FILTER_TYPES_IN_CLASS

    Image() : fImage(nullptr)
            , fOrigin(SkIPoint::Make(0, 0)) {}

    Image(sk_sp<SkSpecialImage> image, const IPoint<In::kLayer, kU>& origin)
            : fImage(std::move(image))
            , fOrigin(origin) {}

    // Allow explicit moves/copies in order to cast from one use type to another, except kInput0
    // and kInput1 can only be cast to kOutput (e.g. as part of a noop image filter).
    template<Usage kI>
    explicit Image(Image<kI>&& image)
            : fImage(std::move(image.fImage))
            , fOrigin(std::move(SkIPoint(image.fOrigin))) {
        static_assert((kU != Usage::kInput) || (kI != Usage::kInput0 && kI != Usage::kInput1),
                      "kInput0 and kInput1 cannot be moved to more generic kInput usage.");
        static_assert((kU != Usage::kInput0 && kU != Usage::kInput1) ||
                      (kI == kU || kI == Usage::kInput || kI == Usage::kOutput),
                      "Can only move to specific input from the generic kInput or kOutput usage.");
    }

    template<Usage kI>
    explicit Image(const Image<kI>& image)
            : fImage(image.fImage)
            , fOrigin(SkIPoint(image.fOrigin)) {
        static_assert((kU != Usage::kInput) || (kI != Usage::kInput0 && kI != Usage::kInput1),
                      "kInput0 and kInput1 cannot be copied to more generic kInput usage.");
        static_assert((kU != Usage::kInput0 && kU != Usage::kInput1) ||
                      (kI == kU || kI == Usage::kInput || kI == Usage::kOutput),
                      "Can only copy to specific input from the generic kInput usage.");
    }

    const SkSpecialImage* image() const { return fImage.get(); }
    sk_sp<SkSpecialImage> refImage() const { return fImage; }

    // Get the subset bounds of this image within its backing proxy. This will have the same
    // dimensions as the image.
    IRect<In::kProxy, kU> subset() const { return IRect<In::kProxy, kU>(fImage->subset()); }

    // Get the layer-space bounds of this image. This will have the same dimensions as the
    // image and its top left corner will be 'origin()'.
    IRect<In::kLayer, kU> layerBounds() const {
        return IRect<In::kLayer, kU>(SkIRect::MakeXYWH(fOrigin->x(), fOrigin->y(),
                                                       fImage->width(), fImage->height()));
    }

    // Get the layer-space coordinate of this image's top left pixel.
    const IPoint<In::kLayer, kU>& origin() const { return fOrigin; }

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
    // Allow all Image templates access to each others members
    template<Usage kO>
    friend class Image;

    sk_sp<SkSpecialImage>  fImage;
    IPoint<In::kLayer, kU> fOrigin;
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
        , fTargetOutput(clipBounds)
        , fCache(cache)
        , fColorType(colorType)
        , fColorSpace(colorSpace)
        , fSource(sk_ref_sp(source), IPoint<In::kLayer, For::kInput>({0, 0})) {}

    Context(const SkMatrix& layerMatrix, const IRect<In::kLayer, For::kOutput>& clipBounds,
            SkImageFilterCache* cache, SkColorType colorType, SkColorSpace* colorSpace,
            const Image<For::kInput>& source)
        : fLayerMatrix(layerMatrix)
        , fTargetOutput(clipBounds)
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
    // The bounds, in the layer space, that the filtered image produce. The output from
    // filterImage() must cover these clip bounds, except in areas where it will just be transparent
    // black, in which case a smaller output image can be returned.
    const IRect<In::kLayer, For::kOutput>& targetOutput() const { return fTargetOutput; }
    // DEPRECATED: Use targetOutput() instead
    const SkIRect& clipBounds() const { return static_cast<const SkIRect&>(fTargetOutput); }
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
    const Image<For::kInput>& source() const { return fSource; }
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
    Context withNewLayerMatrix(const SkMatrix& layerMatrix) const {
        return Context(layerMatrix, fTargetOutput, fCache, fColorType, fColorSpace, fSource);
    }
    // Create a new context that matches this context, but with an overridden target output rect.
    Context withNewTargetOutput(const IRect<In::kLayer, For::kOutput>& bounds) const {
        return Context(fLayerMatrix, bounds, fCache, fColorType, fColorSpace, fSource);
    }
    // DEPRECATED: use withNewTargetOutput() instead
    Context withNewClipBounds(const SkIRect& clipBounds) const {
        return this->withNewTargetOutput(IRect<In::kLayer, For::kOutput>(clipBounds));
    }

private:
    SkMatrix                        fLayerMatrix;
    IRect<In::kLayer, For::kOutput> fTargetOutput;
    SkImageFilterCache*             fCache;
    SkColorType                     fColorType;
    // This will be a pointer that is owned by the device controlling the filter process, and our
    // lifetime is bounded by the device, so it can be a bare pointer.
    SkColorSpace*                   fColorSpace;
    Image<For::kInput>              fSource;
};

} // end namespace skif

#endif // SkImageFilterTypes_DEFINED
