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

// Image filter implementations need to track geometry across a number of coordinate systems. While
// the base Sk[I]Rect and Sk[I]Point types are perfectly suited to represent this data,
// implementations would quickly become difficult to read when there are multiple variables of the
// same type that are defined relative to distinct systems. In particular, when operating on those
// coordinates, are the actions part of transforming from one space to another, or part of the
// effects of the image filter. This template tag enum, in conjunction with the SkFilterBounds,
// SkFilterPoint templates help mitigate these issues and ensure correctness when converting from
// one space to another since it becomes encoded in the type system.
enum class SkFilterCoordinateSystem {
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

// FilterUsage is a template tag to improve the readability of filter implementations. It is can
// be attached to images and geometry to group a collection of related variables and ensure that
// moving from one use case to another is explicit.
// NOTE: This may be aliased as 'As' when using the fluent type names.
enum class SkFilterUsage {
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
    // purpose (e.g. kDevice or kParameter).
    kInherit
};

template<typename T,
         SkFilterCoordinateSystem kCS,
         SkFilterUsage            kU = SkFilterUsage::kInherit>
class SkFilterGeometry {
public:
    SkFilterGeometry() = default;

    explicit SkFilterGeometry(const T& geometry)
            : fData(geometry) {}
    explicit SkFilterGeometry(T&& geometry)
            : fData(std::move(geometry)) {}

    explicit operator const T&() const { return fData; }
    explicit operator T() const { return fData; }

    SkFilterGeometry<T, kCS, kU>& operator=(T&& geometry) {
        fData = std::move(geometry);
        return *this;
    }

    SkFilterGeometry<T, kCS, kU>& operator=(const T& geometry) {
        fData = geometry;
        return *this;
    }

private:
    T fData;
};

template<typename T>
class SkFilterMath {
public:
    template<SkFilterUsage kU>
    static SkFilterGeometry<T, SkFilterCoordinateSystem::kLayer, kU> ParameterToLayer(
            const SkFilterGeometry<T, SkFilterCoordinateSystem::kParameter>& in, const SkMatrix& matrix) {
        return SkFilterGeometry<T, SkFilterCoordinateSystem::kLayer, kU>(ApplyMatrix(T(in), matrix));
    }

    // LayerToDevice();
    // LayerToImage();
    // ImageToLayer();
    // ImageToProxy();
private:
    static T ApplyMatrix(const T& geom, const SkMatrix& matrix);
    static T ApplyOffset(const T& geom, const SkIPoint& offset);
};

template<>
SkRect SkFilterMath<SkRect>::ApplyMatrix(const SkRect& rect, const SkMatrix& matrix) {
    return matrix.mapRect(rect);
}

template<>
SkIRect SkFilterMath<SkIRect>::ApplyMatrix(const SkIRect& rect, const SkMatrix& matrix) {
    return matrix.mapRect(SkRect::Make(rect)).roundOut();
}

template<SkFilterCoordinateSystem kCS, SkFilterUsage kU>
using Bounds = SkFilterGeometry<SkRect, kCS, kU>;
template<SkFilterCoordinateSystem kCS, SkFilterUsage kU>
using IBounds = SkFilterGeometry<SkIRect, kCS, kU>;

template<typename T, SkFilterUsage kU>
using SkParameterToLayer = SkFilterMath<T>::ParameterToLayer<kU>;

/*
template<SkFilterCoordinateSystem kCS, SkFilterUsage kU>
SkPoint SkFilterGeometry<SkPoint, kCS, kU>::applyMatrix(const SkMatrix& matrix) {
    SkPoint p;
    matrix.mapPoints(&p, &fData, 1);
    return p;
}

template<SkFilterCoordinateSystem kCS, SkFilterUsage kU>
SkIPoint SkFilterGeometry<SkIPoint, kCS, kU>::applyMatrix(const SkMatrix& matrix) {
    SkPoint p;
    p.iset(fData);
    matrix.mapPoints(&p, 1);
    return {SkScalarCeilToInt(p.fX), SkScalarCeilToInt(p.fY)};
}
*/

/*
template<SkFilterCoordinateSystem kCS,
         SkFilterUsage            kU = SkFilterUsage::kInherit>
struct SkFilterVector;

// A coordinate system-tagged wrapper of SkIPoint. Static asserts enforce when the optional usage
// parameter must be specified, so that the valid set of Coord templates are:
//   Coord<kParameter>           - a 2D point/vector stored by an SkImageFilter, e.g. an offset
//   Coord<kLayer>               - for layer-space calculations that aren’t associated with an
//                                 input or output
//   Coord<kLayer, kOutput>      - the layer-space origin of the output image
//   Coord<kImage, kOutput>      - a pixel coordinate of the output image
//   Coord<kProxy, kOutput>      - a pixel coordinate of the output image, relative to its actual
//                                 backing proxy (primarily used for gpu implementations)
//   Coord<kLayer, kInput[|0|1]> - the layer-space origin of the possibly slot-specific input image
//   Coord<kImage, kInput[|0|1]> - a pixel coordinate of the input image
//   Coord<kProxy, kInput[|0|1]> - a pixel coordinate of the input image, relative to its actual
//                                 backing proxy (primarily used for gpu implementations)
//
// Unlike SkIPoint which can freely be interpreted as a vector or a point, Coord only refers to
// a point. SkFilterVector provides an equivalent wrapper for a 2D vector.,
template<SkFilterCoordinateSystem kCS,
         SkFilterUsage            kU = SkFilterUsage::kInherit>
struct SkFilterCoord {
public:
    // ... include same static asserts as Bounds
    SkFilterCoord() = default;
    explicit SkFilterCoord(const SkIPoint& point)
            : fPoint(point) {}

    explicit operator SkIPoint() const { return fPoint; };

    // Mirrors SkIPoint API but maintains coordinate system safety.
    void operator+=(const SkFilterVector<kCS, kU>& c) {
        fPoint += c.fVector;
    }

    void operator-=(const SkFilterVector<kCS, kU>& c) {
        fPoint -= c.fVector;
    }

    friend bool operator==(SkFilterCoord<kCS, kU>& a, SkFilterCoord<kCS, kU>& b) {
        return a.fPoint == b.fPoint;
    }

    friend SkFilterCoord<kCS, kU> operator+(SkFilterCoord<kCS, kU>& a, SkFilterVector<kCS, kU>& b) {
        return SkFilterCoord(a.fPoint + b.fVector);
    }

    friend SkFilterVector<kCS, kU> operator-(SkFilterCoord<kCS, kU>& a, SkFilterCoord<kCS, kU>& b) {
        return SkFilterCoord(a.fPoint - b.fPoint);
    }

private:
    SkIPoint fPoint;
};

template<SkFilterCoordinateSystem kCS,
         SkFilterUsage            kU = SkFilterUsage::kInherit>
struct SkFilterVector {
public:
    // ... include same static asserts as Bounds
    SkFilterVector() = default;
    explicit SkFilterVector(const SkIVector& vector)
            : fVector(vector) {}

    explicit operator SkIVector() const { return fVector; };

    // Mirrors SkIPoint API but maintains coordinate system safety.
    SkFilterVector<kCS, kU> operator-() const { return SkFilterVector(-fVector); }

    void operator+=(const SkFilterVector<kCS, kU>& c) {
        fVector += c.fVector;
    }

    void operator-=(const SkFilterVector<kCS, kU>& c) {
        fVector -= c.fVector;
    }

    friend bool operator==(SkFilterVector<kCS, kU>& a, SkFilterVector<kCS, kU>& b) {
        return a.fVector == b.fVector;
    }

    friend SkFilterVector<kCS, kU> operator+(SkFilterVector<kCS, kU>& a,
                                             SkFilterVector<kCS, kU>& b) {
        return SkFilterVector(a.fVector + b.fVector);
    }

    friend SkFilterVector<kCS, kU> operator-(SkFilterVector<kCS, kU>& a,
                                             SkFilterVector<kCS, kU>& b) {
        return SkFilterVector(a.fVector - b.fVector);
    }

private:
    SkIVector applyMatrix(const SkMatrix& matrix) {
        // Use mapVector to skip applying the translation component of the matrix

        return matrix.mapRect(SkRect::Make(fRect)).roundOut();
    }

    SkIVector applyOffset(const SkIPoint& offset) {
        // Offsets are not applied to directions, so this is a no-op
        return fVector;
    }

    SkIVector fVector;
};

// A coordinate system-tagged wrapper of SkIRect. Static asserts enforce when the optional usage
// parameter must be specified, so that the valid set of Bounds templates are:
//   Bounds<kParameter>           - geometry stored by an SkImageFilter, e.g. a crop rect
//   Bounds<kLayer>               - for layer-space calculations that aren’t associated with an
//                                  input or output
//   Bounds<kLayer, kOutput>      - the layer-space bounds of the output image
//   Bounds<kImage, kOutput>      - a subset of pixels of the output image
//   Bounds<kProxy, kOutput>      - a subset of pixels of the output image, relative to its actual
//                                  backing proxy (primarily used for gpu implementations)
//   Bounds<kLayer, kInput[|0|1]> - the layer-space bounds of the possibly slot-specific input image
//   Bounds<kImage, kInput[|0|1]> - a subset of pixels of the input image
//   Bounds<kProxy, kInput[|0|1]> - a subset of pixels of the input image, relative to its actual
//                                  backing proxy (primarily used for gpu implementations)
template<SkFilterCoordinateSystem kCS,
         SkFilterUsage            kU = SkFilterUsage::kInherit>
class SkFilterBounds {
public:
    static_assert(!(kCS == kParameter || kCS == kDevice) || kU == kInherit,
                  "kParameter and kDevice space must use kInherit as their usage");
    static_assert(!(kCS == kImage || kCS == kProxy) || kU != kInherit,
                  "kImage and kProxy space cannot use kInherit for their usage");

    SkFilterBounds() = default;
    explicit SkFilterBounds(const SkIRect& rect)
            : fRect(rect) {}

    // The coordinates of the four corners of the bounding box, specified in kCS.
    explicit operator SkIRect() const { return fRect; };

    // ... add accessors and mutators as necessary
    SkFilterCoord<kCS, kU> topleft() const { return SkFilterCoord<>({fRect.fLeft, fRect.fTop}); }

    // Mirrors SkIRect API but maintains coordinate system safety.
    void join(const SkFilterBounds<kCS, kU>& bounds) {
        fRect.join(bounds.fRect);
    }

    bool intersect(const SkFilterBounds<kCS, kU>& bounds) {
        return fRect.intersect(bounds.fRect);
    }

    void outset(int dx, int dy) {
        fRect.outset(dx, dy);
    }

    void offset(const SkFilterVector<kCS, kU>& offset) {
        fRect.offset(SkIPoint(offset));
    }

private:
    SkIRect applyMatrix(const SkMatrix& matrix) {
        return matrix.mapRect(SkRect::Make(fRect)).roundOut();
    }

    SkIRect applyOffset(const SkIPoint& offset) {
        SkIRect rect;
        rect.offset(offset);
        return rect;
    }

    SkIRect fRect;
};

// Convenience macros to add 'using' declarations that rename the above enums/types to provide a
// more fluent and readable API. This should only be used in a private or local scope to prevent
// leakage of the names. Use the IN_CLASS variant at the start of a class declaration in those
// scenarios. These macros enable the following simpler type names:
//   SkFilteredImage<SkFilterUsage::kInput> -> SkFilteredImage<As::kInput>
//
#define SK_USE_FLUENT_IMAGE_FILTER_TYPES \
    using As = SkFilterUsage; \
    using In = SkFilterCoordinateSystem; \
    template<SkFilterCoordinateSystem kCS, \
             SkFilterUsage            kU = SkFilterUsage::kInherit> \
    using Bounds = SkFilterBounds<kCS, kU>;
    template<SkFilterCoordinateSystem kCS, \
             SkFilterUsage            kU = SkFilterUsage::kInherit> \
    using Coord = SkFilterCoord<kCS, kU>;
    template<SkFilterCoordinateSystem kCS, \
             SkFilterUsage            kU = SkFilterUsage::kInherit> \
    using Vector = SkFilterVector<kCS, kU>;

#define SK_USE_FLUENT_IMAGE_FILTER_TYPES_IN_CLASS \
    protected: SK_USE_FLUENT_IMAGE_FILTER_TYPES public:

// Utility math functions to convert from one coordinate system to another
    // FIXME I don't love this name or having it as a prefix, but I want to define the functions
    // using the fluent API, which requires putting it into a class.
    //
    // Is it possible that we could make them defined on the actual types? so everything would have
    // a toLayer(), toDevice(), toImage(), and toProxy() function?

    // Then the only special functions we need are the usage changing ones for MapImage() and
    // layer casting. I do like that.  And since there are only so many coordinate systems, we
    // could have all of the template specializations defined in the C++ file anyways.
template<typename T, /* SkFilterBounds, SkFilterCoord, or SkFilterVector */
    /*     SkFilterUsage kU>
T<In::kLayer, kU> SkParameterToLayer(const T<In::kParameter>& geom, const SkMatrix& layerCTM) {
    return T<In::kLayer, kU>(geom.applyMatrix(layerCTM));
}

template<typename T,
         SkFilterUsage kU>
T<In::kDevice> SkLayerToDevice(const T<In::kLayer, kU>& geom, const SkMatrix& remainder) {
    return T<In::kDevice>(geom.applyMatrix(remainder));
}

template<typename T,
         SkFilterUsage kU>
T<In::kImage, kU> SkLayerToImage(const T<In::kLayer, kU>& geom, const Coord<In::kLayer, kU>& origin) {
    return T<In::kImage, kU>(geom.applyOffset(-SkIPoint(origin)));
}

template<typename T,
         SkFilterUsage kU>
T<In::kLayer, kU> SkImageToLayer(const T<In::kImage, kU>& geom, const Coord<In::kLayer, kU>& origin) {
    return T<In::kLayer, kU>(geom.applyOffset(SkIPoint(origin)));
}

template<typename T,
         SkFilterUsage kU>
T<In::kProxy, kU> SkImageToProxy(const T<In::kImage, kU>& geom, const Coord<In::kProxy, kU>& origin) {
    return T<In::kProxy, kU>(geom.applyOffset(SkIPoint(origin)));
}

template<typename T,
         SkFilterUsage kUIn, SkFilterUsage kUOut>
T<In::kLayer, kUOut> SkLayerCast(const T<In::kLayer, kUIn>& geom) {
    //. TODO
}

template<typename T,
         SkFilterUsage kUIn, SkFilterUsage kUOut>
T<In::kImage, kUOut> SkMapImageSpace(const T<In::kImage, kUIn>& geom, const Coord<In::kLayer, kUIn>& inOrigin,
                    const Coord<In::kLayer, kUOut>& outOrigin) {
    return SkLayerToImage(SkLayerCast<kUOut>(SkImageToLayer(geom, inOrigin)), outOrigin);
}

class SkCoords {
public:
    SK_USE_FLUENT_IMAGE_FILTER_TYPES_IN_CLASS

    template<SkFilterUsage kU>
    Bounds<In::kImage, kU> LayerToImage(const Bounds<In::kLayer, kU>&, const Coord<In::kLayer, kU>& origin) {
        return in.offset(-origin);
    }

    template<SkFilterUsage kU>
    Coord<In::kImage, kU> LayerToImage(const Coord<In::kLayer, kU>&, const Coord<In::kLayer, kU>& origin) {
        return in - origin;
    }

    template<SkFilterUsage kU>
    Bounds<In::kLayer, kU> ImageToLayer(const Bounds<In::kImage, kU>&, const Coord<In::kLayer, kU>& origin) {
        return in.offset(origin);
    }

    template<SkFilterUsage kU>
    Coord<In::kLayer, kU> ImageToLayer(const Coord<In::kImage, kU>&, const Coord<In::kLayer, kU>& origin) {
        return in + origin;
    }
    template<SkFilterUsage kU>
    Bounds<In::kProxy, kU> ImageToProxy(const Bounds<In::kImage, kU>&, const Coord<In::kProxy, kU>& origin) {
        return in.offset(origin);
    }

    template<SkFilterUsage kU>
    Coord<In::kProxy, kU> ImageToProxy(const Coord<In::kImage, kU>&, const Coord<In::kProxy, kU>& origin) {
        return in + origin;
    }

    // Converting between different usages in the layer coordinate system is free, but should be an
    // explicit choice in the code.
    template<SkFilterUsage kU1, SkFilterUsage kU2>
    Bounds<kLayer, kU2> LayerCast(const Bounds<kLayer, kU1>&);
    template<SkFilterUsage kU1, SkFilterUsage kU2>
    Coord<kLayer, kU2> LayerCast(const Coord<kLayer, kU1>&);
    template<SkFilterUsage kU1, SkFilterUsage kU2>
    Vector<kLayer, kU2> LayerCast(const Vector<kLayer, kU1>&);
    // FIXME make && versions too for the layer space

    // Converting between different image spaces is not and depends on the layer-space origins for
    // both images.
    template<SkFilterUsage kU1, SkFilterUsage kU2>
    Bounds<In::kImage, kU2> MapImage(const Bounds<In::kImage, kU1>&, const Coord<In::kLayer, kU1>& inOrigin,
        const Coord<In::kLayer, kU2>& outOrigin) {
        return LayerToImage<kU2>(LayerCast<kU2>(ImageToLayer<kU1>(in, inOrigin)), outOrigin);
    }

    template<SkFilterUsage kU1, SkFilterUsage ku2>
    Coord<In::kImage, kU2> MapImage(const Coord<In::kImage, kU1>&, const Coord<In::kLayer, kU1>& inOrigin,
        const Coord<In::kLayer, kU2>& outOrigin) {
        return LayerToImage<kU2>(LayerCast<kU2>(ImageToLayer<kU1>(in, inOrigin)), outOrigin);
    }

private:
    SkFilterMath() = delete;
}
*/
// Wraps an SkSpecialImage and tags it with a corresponding usage, either as generic input (e.g. the
// source image), or a specific input image from a filter's connected inputs. It also includes the
// origin of the image in the layer space. This origin is used to draw the image in the correct
// location. The 'layerBounds' rectangle of the filtered image is the layer-space bounding box of
// the image. This has its top left corner at the 'origin' and has the same dimensions as the
// underlying special image subset. Transforming 'layerBounds' by the SkFilterContext's layerCTM and
// painting it with the subset image will display the filtered image in the appropriate device-space
// region.
//
// When filter implementations are processing intermediate SkFilteredImage results, it can be
// assumed that all SkFilteredImages' layerBounds are in the same layer coordinate space defined by
// the shared SkFilterContext.
template<SkFilterUsage kU>
class SkFilteredImage {
public:
    static_assert(kU != SkFilterUsage::kInherit, "Images cannot use the kInherit use case");

    SkFilteredImage()
            : fImage(nullptr)
            , fOrigin({0, 0}) {}

    SkFilteredImage(sk_sp<SkSpecialImage> image, const Coord<In::kLayer, kU>& origin)
            : fImage(std::move(image))
            , fOrigin(origin) {}

    // Allow explicit moves/copies in order to cast from one use type to another, except kInput0
    // and kInput1 can only be cast to kOutput (e.g. as part of a noop image filter).
    template<SkFilterUsage kI>
    explicit SkFilteredImage(SkFilteredImage<kI>&& image)
            : fImage(std::move(image.fImage))
            , fOrigin(std::move(image.fOrigin)) {
        static_assert((kU != SkFilterUsage::kInput) ||
                      (kI != SkFilterUsage::kInput0 && kI != SkFilterUsage::kInput1),
                      "kInput0 and kInput1 cannot be moved to more generic kInput usage.");
        static_assert((kU != SkFilterUsage::kInput0 && kU != SkFilterUsage::kInput1) ||
                      (kI == kU || kI == SkFilterUsage::kInput || kI == SkFilterUsage::kOutput),
                      "Can only move to specific input from the generic kInput or kOutput usage.");
    }

    template<SkFilterUsage kI>
    explicit SkFilteredImage(const SkFilteredImage<kI>& image)
            : fImage(image.fImage)
            , fOrigin(image.fOrigin) {
        static_assert((kU != SkFilterUsage::kInput) ||
                      (kI != SkFilterUsage::kInput0 && kI != SkFilterUsage::kInput1),
                      "kInput0 and kInput1 cannot be copied to more generic kInput usage.");
        static_assert((kU != SkFilterUsage::kInput0 && kU != SkFilterUsage::kInput1) ||
                      (kI == kU || kI == SkFilterUsage::kInput),
                      "Can only copy to specific input from the generic kInput usage.");
    }

    const SkSpecialImage* image() const { return fImage.get(); }
    sk_sp<SkSpecialImage> refImage() const { return fImage; }

    // TODO (michaelludwig) - the geometry types will be updated to tag the coordinate system
    // associated with each one.

    // Get the subset bounds of this image within its backing proxy. This will have the same
    // dimensions as the image.
    const Bounds<In::kProxy, kU>& subset() const { return fImage->subset(); }

    // Get the layer-space bounds of this image. This will have the same dimensions as the
    // image and its top left corner will be 'origin()'.
    const Bounds<In::kLayer, kU>& layerBounds() const {
        return SkIRect::MakeXYWH(fOrigin.x(), fOrigin.y(), fImage->width(), fImage->height());
    }

    // Get the layer-space coordinate of this image's top left pixel.
    const Coord<In::kLayer, kU>& origin() const { return fOrigin; }

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
    // Allow all SkFilteredImage templates access to each others members
    template<SkFilterUsage kO>
    friend class SkFilteredImage;

    sk_sp<SkSpecialImage> fImage;
    Coord<In::kLayer, kU> fOrigin;
};


// The context contains all necessary information to describe how the image filter should be
// computed (i.e. the current layer matrix and clip), and the color information of the output of
// a filter DAG. For now, this is just the color space (of the original requesting device). This
// is used when constructing intermediate rendering surfaces, so that we ensure we land in a
// surface that's similar/compatible to the final consumer of the DAG's output.
class SkFilterContext {
public:
    SK_USE_FLUENT_IMAGE_FILTER_TYPES_IN_CLASS

    // Creates a context with the given layer matrix and destination clip, reading from 'source'
    // with an origin of (0,0).
    SkFilterContext(const SkMatrix& layerCTM, const SkIRect& clipBounds, SkImageFilterCache* cache,
                    SkColorType colorType, SkColorSpace* colorSpace,
                    sk_sp<SkSpecialImage> source)
        : fLayerCTM(layerCTM)
        , fClipBounds(clipBounds)
        , fCache(cache)
        , fColorType(colorType)
        , fColorSpace(colorSpace)
        , fSource(std::move(source), {0, 0}) {}

    SkFilterContext(const SkMatrix& layerCTM, const SkIRect& clipBounds, SkImageFilterCache* cache,
                    SkColorType colorType, SkColorSpace* colorSpace,
                    const SkFilteredImage<As::kInput>& source)
        : fLayerCTM(layerCTM)
        , fClipBounds(clipBounds)
        , fCache(cache)
        , fColorType(colorType)
        , fColorSpace(colorSpace)
        , fSource(source) {}

    /**
     *  Since a context can be built directly, its constructor has no chance to
     *  "return null" if it's given invalid or unsupported inputs. Call this to
     *  know of the the context can be used.
     *
     *  The SkImageFilterCache Key, for example, requires a finite ctm (no infinities
     *  or NaN), so that test is part of isValid.
     */
    bool isValid() const { return fSource.image() != nullptr && fLayerCTM.isFinite(); }

    // Create a new context that matches this context, but with an overridden layer CTM matrix.
    SkFilterContext withLayerCTM(const SkMatrix& ctm) const {
        return SkFilterContext(ctm, fClipBounds, fCache, fColorType, fColorSpace, fSource);
    }
    // Create a new context that matches this context, but with an overridden clip bounds rect.
    SkFilterContext withClipBounds(const SkIRect& clipBounds) const {
        return SkFilterContext(fLayerCTM, clipBounds, fCache, fColorType, fColorSpace, fSource);
    }

    // Create a surface of the given size, that matches the context's color type and color space as
    // closely as possible, and uses the same backend of the device that produced the context's
    // source image.
    sk_sp<SkSpecialSurface> makeSurface(const SkISize& size,
                                        const SkSurfaceProps* props = nullptr) const {
        return fSource.image()->makeSurface(fColorType, fColorSpace, size,
                                            kPremul_SkAlphaType, props);
    }

    // The transformation from the local parameter space of the filters to the layer space where
    // filtering is computed. This may or may not be the total canvas CTM, depending on the
    // matrix type of the total CTM and whether or not the filter DAG supports complex CTMs. If
    // a node returns false from canHandleComplexCTM(), layerCTM() will be at most a scale +
    // translate matrix and any remaining matrix will be handled by the canvas after filtering
    // is finished.
    const SkMatrix& layerCTM() const { return fLayerCTM; }
    // The bounds, in the layer space, that the filtered image will be clipped to. The output
    // from filterImage() must cover these clip bounds, except in areas where it just be
    // transparent black, in which case a smaller output image can be returned.
    const Bounds<In::kLayer, As::kOutput>& clipBounds() const { return fClipBounds; }
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
    const SkFilteredImage<As::kInput>& source() const { return fSource; }

    // True if image filtering should occur on the GPU if possible.
    bool useGPU() const { return fSource.image()->isTextureBacked(); }
    // The recording context to use for when computing the filter with the GPU.
    GrRecordingContext* getGrContext() const { return fSource.image()->getContext(); }

private:
    SkMatrix                     fLayerCTM;
    Bound<In::kLayer, As::kOutput> fClipBounds;
    SkImageFilterCache*          fCache;
    SkColorType                  fColorType;
    // This will be a pointer that is owned by the device controlling the filter process, and our
    // lifetime is bounded by the device, so it can be a bare pointer.
    SkColorSpace*                fColorSpace;
    SkFilteredImage<As::kInput>  fSource;
};

#endif // SkImageFilterTypes_DEFINED
