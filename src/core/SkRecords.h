/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRecords_DEFINED
#define SkRecords_DEFINED

#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkM44.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRSXform.h"
#include "include/core/SkRect.h"
#include "include/core/SkRegion.h"
#include "include/core/SkString.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkVertices.h"
#include "src/core/SkDrawShadowInfo.h"

namespace SkRecords {

// A list of all the types of canvas calls we can record.
// Each of these is reified into a struct below.
//
// (We're using the macro-of-macro trick here to do several different things with the same list.)
//
// We leave this SK_RECORD_TYPES macro defined for use by code that wants to operate on SkRecords
// types polymorphically.  (See SkRecord::Record::{visit,mutate} for an example.)
//
// Order doesn't technically matter here, but the compiler can generally generate better code if
// you keep them semantically grouped, especially the Draws.  It's also nice to leave NoOp at 0.
#define SK_RECORD_TYPES(M)                                          \
    M(NoOp)                                                         \
    M(Flush)                                                        \
    M(Restore)                                                      \
    M(Save)                                                         \
    M(SaveLayer)                                                    \
    M(SaveBehind)                                                   \
    M(MarkCTM)                                                      \
    M(SetMatrix)                                                    \
    M(SetM44)                                                       \
    M(Translate)                                                    \
    M(Scale)                                                        \
    M(Concat)                                                       \
    M(Concat44)                                                     \
    M(ClipPath)                                                     \
    M(ClipRRect)                                                    \
    M(ClipRect)                                                     \
    M(ClipRegion)                                                   \
    M(ClipShader)                                                   \
    M(ResetClip)                                                    \
    M(DrawArc)                                                      \
    M(DrawDrawable)                                                 \
    M(DrawImage)                                                    \
    M(DrawImageLattice)                                             \
    M(DrawImageRect)                                                \
    M(DrawDRRect)                                                   \
    M(DrawOval)                                                     \
    M(DrawBehind)                                                   \
    M(DrawPaint)                                                    \
    M(DrawPath)                                                     \
    M(DrawPatch)                                                    \
    M(DrawPicture)                                                  \
    M(DrawPoints)                                                   \
    M(DrawRRect)                                                    \
    M(DrawRect)                                                     \
    M(DrawRegion)                                                   \
    M(DrawTextBlob)                                                 \
    M(DrawAtlas)                                                    \
    M(DrawVertices)                                                 \
    M(DrawShadowRec)                                                \
    M(DrawAnnotation)                                               \
    M(DrawEdgeAAQuad)                                               \
    M(DrawEdgeAAImageSet)


// Defines SkRecords::Type, an enum of all record types.
#define ENUM(T) T##_Type,
enum Type { SK_RECORD_TYPES(ENUM) };
#undef ENUM

#define ACT_AS_PTR(ptr)                 \
    operator T*() const { return ptr; } \
    T* operator->() const { return ptr; }

// An Optional doesn't own the pointer's memory, but may need to destroy non-POD data.
template <typename T>
class Optional {
public:
    Optional() : fPtr(nullptr) {}
    Optional(T* ptr) : fPtr(ptr) {}
    Optional(Optional&& o) : fPtr(o.fPtr) {
        o.fPtr = nullptr;
    }
    ~Optional() { if (fPtr) fPtr->~T(); }

    ACT_AS_PTR(fPtr)
private:
    T* fPtr;
    Optional(const Optional&) = delete;
    Optional& operator=(const Optional&) = delete;
};

// PODArray doesn't own the pointer's memory, and we assume the data is POD.
template <typename T>
class PODArray {
public:
    PODArray() {}
    PODArray(T* ptr) : fPtr(ptr) {}
    // Default copy and assign.

    ACT_AS_PTR(fPtr)
private:
    T* fPtr;
};

#undef ACT_AS_PTR

// SkPath::getBounds() isn't thread safe unless we precache the bounds in a singlethreaded context.
// SkPath::cheapComputeDirection() is similar.
// Recording is a convenient time to cache these, or we can delay it to between record and playback.
struct PreCachedPath : public SkPath {
    PreCachedPath() {}
    PreCachedPath(const SkPath& path);
};

// Like SkPath::getBounds(), SkMatrix::getType() isn't thread safe unless we precache it.
// This may not cover all SkMatrices used by the picture (e.g. some could be hiding in a shader).
struct TypedMatrix : public SkMatrix {
    TypedMatrix() {}
    TypedMatrix(const SkMatrix& matrix);
};

enum Tags {
    kDraw_Tag      = 1,   // May draw something (usually named DrawFoo).
    kHasImage_Tag  = 2,   // Contains an SkImage or SkBitmap.
    kHasText_Tag   = 4,   // Contains text.
    kHasPaint_Tag  = 8,   // May have an SkPaint field, at least optionally.

    kDrawWithPaint_Tag = kDraw_Tag | kHasPaint_Tag,
};

// A macro to make it a little easier to define a struct that can be stored in SkRecord.
#define RECORD(T, tags, ...)            \
struct T {                              \
    static const Type kType = T##_Type; \
    static const int kTags = tags;      \
    __VA_ARGS__;                        \
};

RECORD(NoOp, 0);
RECORD(Flush, 0);
RECORD(Restore, 0,
        TypedMatrix matrix);
RECORD(Save, 0);

RECORD(SaveLayer, kHasPaint_Tag,
       Optional<SkRect> bounds;
       Optional<SkPaint> paint;
       sk_sp<const SkImageFilter> backdrop;
       SkCanvas::SaveLayerFlags saveLayerFlags;
       SkScalar backdropScale);

RECORD(SaveBehind, 0,
       Optional<SkRect> subset);

RECORD(MarkCTM, 0,
       SkString name);
RECORD(SetMatrix, 0,
        TypedMatrix matrix);
RECORD(SetM44, 0,
        SkM44 matrix);
RECORD(Concat, 0,
        TypedMatrix matrix);
RECORD(Concat44, 0,
       SkM44 matrix);

RECORD(Translate, 0,
        SkScalar dx;
        SkScalar dy);

RECORD(Scale, 0,
       SkScalar sx;
       SkScalar sy);

struct ClipOpAndAA {
    ClipOpAndAA() {}
    ClipOpAndAA(SkClipOp op, bool aa) : fOp(static_cast<unsigned>(op)), fAA(aa) {}

    SkClipOp op() const { return static_cast<SkClipOp>(fOp); }
    bool aa() const { return fAA != 0; }

private:
    unsigned fOp : 31;  // This really only needs to be 3, but there's no win today to do so.
    unsigned fAA :  1;  // MSVC won't pack an enum with an bool, so we call this an unsigned.
};
static_assert(sizeof(ClipOpAndAA) == 4, "ClipOpAndAASize");

RECORD(ClipPath, 0,
        PreCachedPath path;
        ClipOpAndAA opAA);
RECORD(ClipRRect, 0,
        SkRRect rrect;
        ClipOpAndAA opAA);
RECORD(ClipRect, 0,
        SkRect rect;
        ClipOpAndAA opAA);
RECORD(ClipRegion, 0,
        SkRegion region;
        SkClipOp op);
RECORD(ClipShader, 0,
        sk_sp<SkShader> shader;
        SkClipOp op);
RECORD(ResetClip, 0);

// While not strictly required, if you have an SkPaint, it's fastest to put it first.
RECORD(DrawArc, kDraw_Tag|kHasPaint_Tag,
       SkPaint paint;
       SkRect oval;
       SkScalar startAngle;
       SkScalar sweepAngle;
       unsigned useCenter);
RECORD(DrawDRRect, kDraw_Tag|kHasPaint_Tag,
        SkPaint paint;
        SkRRect outer;
        SkRRect inner);
RECORD(DrawDrawable, kDraw_Tag,
        Optional<SkMatrix> matrix;
        SkRect worstCaseBounds;
        int32_t index);
RECORD(DrawImage, kDraw_Tag|kHasImage_Tag|kHasPaint_Tag,
        Optional<SkPaint> paint;
        sk_sp<const SkImage> image;
        SkScalar left;
        SkScalar top;
        SkSamplingOptions sampling);
RECORD(DrawImageLattice, kDraw_Tag|kHasImage_Tag|kHasPaint_Tag,
        Optional<SkPaint> paint;
        sk_sp<const SkImage> image;
        int xCount;
        PODArray<int> xDivs;
        int yCount;
        PODArray<int> yDivs;
        int flagCount;
        PODArray<SkCanvas::Lattice::RectType> flags;
        PODArray<SkColor> colors;
        SkIRect src;
        SkRect dst;
        SkFilterMode filter);
RECORD(DrawImageRect, kDraw_Tag|kHasImage_Tag|kHasPaint_Tag,
        Optional<SkPaint> paint;
        sk_sp<const SkImage> image;
        SkRect src;
        SkRect dst;
        SkSamplingOptions sampling;
        SkCanvas::SrcRectConstraint constraint);
RECORD(DrawOval, kDraw_Tag|kHasPaint_Tag,
        SkPaint paint;
        SkRect oval);
RECORD(DrawPaint, kDraw_Tag|kHasPaint_Tag,
        SkPaint paint);
RECORD(DrawBehind, kDraw_Tag|kHasPaint_Tag,
       SkPaint paint);
RECORD(DrawPath, kDraw_Tag|kHasPaint_Tag,
        SkPaint paint;
        PreCachedPath path);
RECORD(DrawPicture, kDraw_Tag|kHasPaint_Tag,
        Optional<SkPaint> paint;
        sk_sp<const SkPicture> picture;
        TypedMatrix matrix);
RECORD(DrawPoints, kDraw_Tag|kHasPaint_Tag,
        SkPaint paint;
        SkCanvas::PointMode mode;
        unsigned count;
        PODArray<SkPoint> pts);
RECORD(DrawRRect, kDraw_Tag|kHasPaint_Tag,
        SkPaint paint;
        SkRRect rrect);
RECORD(DrawRect, kDraw_Tag|kHasPaint_Tag,
        SkPaint paint;
        SkRect rect);
RECORD(DrawRegion, kDraw_Tag|kHasPaint_Tag,
        SkPaint paint;
        SkRegion region);
RECORD(DrawTextBlob, kDraw_Tag|kHasText_Tag|kHasPaint_Tag,
        SkPaint paint;
        sk_sp<const SkTextBlob> blob;
        SkScalar x;
        SkScalar y);
RECORD(DrawPatch, kDraw_Tag|kHasPaint_Tag,
        SkPaint paint;
        PODArray<SkPoint> cubics;
        PODArray<SkColor> colors;
        PODArray<SkPoint> texCoords;
        SkBlendMode bmode);
RECORD(DrawAtlas, kDraw_Tag|kHasImage_Tag|kHasPaint_Tag,
        Optional<SkPaint> paint;
        sk_sp<const SkImage> atlas;
        PODArray<SkRSXform> xforms;
        PODArray<SkRect> texs;
        PODArray<SkColor> colors;
        int count;
        SkBlendMode mode;
        SkSamplingOptions sampling;
        Optional<SkRect> cull);
RECORD(DrawVertices, kDraw_Tag|kHasPaint_Tag,
        SkPaint paint;
        sk_sp<SkVertices> vertices;
        SkBlendMode bmode);
RECORD(DrawShadowRec, kDraw_Tag,
       PreCachedPath path;
       SkDrawShadowRec rec);
RECORD(DrawAnnotation, 0,  // TODO: kDraw_Tag, skia:5548
       SkRect rect;
       SkString key;
       sk_sp<SkData> value);
RECORD(DrawEdgeAAQuad, kDraw_Tag,
       SkRect rect;
       PODArray<SkPoint> clip;
       SkCanvas::QuadAAFlags aa;
       SkColor4f color;
       SkBlendMode mode);
RECORD(DrawEdgeAAImageSet, kDraw_Tag|kHasImage_Tag|kHasPaint_Tag,
       Optional<SkPaint> paint;
       SkAutoTArray<SkCanvas::ImageSetEntry> set;
       int count;
       PODArray<SkPoint> dstClips;
       PODArray<SkMatrix> preViewMatrices;
       SkSamplingOptions sampling;
       SkCanvas::SrcRectConstraint constraint);
#undef RECORD

}  // namespace SkRecords

#endif//SkRecords_DEFINED
