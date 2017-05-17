/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRecords_DEFINED
#define SkRecords_DEFINED

#include "SkData.h"
#include "SkCanvas.h"
#include "SkDrawable.h"
#include "SkDrawShadowRec.h"
#include "SkImage.h"
#include "SkImageFilter.h"
#include "SkMatrix.h"
#include "SkPath.h"
#include "SkPicture.h"
#include "SkRect.h"
#include "SkRegion.h"
#include "SkRRect.h"
#include "SkRSXform.h"
#include "SkString.h"
#include "SkTextBlob.h"
#include "SkVertices.h"

// Windows.h, will pull in all of the GDI defines.  GDI #defines
// DrawText to DrawTextA or DrawTextW, but SkRecord has a struct
// called DrawText. Since this file does not use GDI, undefing
// DrawText makes things less confusing.
#ifdef DrawText
#undef DrawText
#endif

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
    M(Restore)                                                      \
    M(Save)                                                         \
    M(SaveLayer)                                                    \
    M(SetMatrix)                                                    \
    M(Translate)                                                    \
    M(Concat)                                                       \
    M(ClipPath)                                                     \
    M(ClipRRect)                                                    \
    M(ClipRect)                                                     \
    M(ClipRegion)                                                   \
    M(DrawArc)                                                      \
    M(DrawDrawable)                                                 \
    M(DrawImage)                                                    \
    M(DrawImageLattice)                                             \
    M(DrawImageRect)                                                \
    M(DrawImageNine)                                                \
    M(DrawDRRect)                                                   \
    M(DrawOval)                                                     \
    M(DrawPaint)                                                    \
    M(DrawPath)                                                     \
    M(DrawPatch)                                                    \
    M(DrawPicture)                                                  \
    M(DrawPoints)                                                   \
    M(DrawPosText)                                                  \
    M(DrawPosTextH)                                                 \
    M(DrawText)                                                     \
    M(DrawTextOnPath)                                               \
    M(DrawTextRSXform)                                              \
    M(DrawRRect)                                                    \
    M(DrawRect)                                                     \
    M(DrawRegion)                                                   \
    M(DrawTextBlob)                                                 \
    M(DrawAtlas)                                                    \
    M(DrawVertices)                                                 \
    M(DrawShadowRec)                                                \
    M(DrawAnnotation)

// Defines SkRecords::Type, an enum of all record types.
#define ENUM(T) T##_Type,
enum Type { SK_RECORD_TYPES(ENUM) };
#undef ENUM

#define ACT_AS_PTR(ptr)                 \
    operator T*() const { return ptr; } \
    T* operator->() const { return ptr; }

// An Optional doesn't own the pointer's memory, but may need to destroy non-POD data.
template <typename T>
class Optional : SkNoncopyable {
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
};

// Like Optional, but ptr must not be NULL.
template <typename T>
class Adopted : SkNoncopyable {
public:
    Adopted(T* ptr) : fPtr(ptr) { SkASSERT(fPtr); }
    Adopted(Adopted* source) {
        // Transfer ownership from source to this.
        fPtr = source->fPtr;
        source->fPtr = NULL;
    }
    ~Adopted() { if (fPtr) fPtr->~T(); }

    ACT_AS_PTR(fPtr)
private:
    T* fPtr;
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
RECORD(Restore, 0,
        SkIRect devBounds;
        TypedMatrix matrix);
RECORD(Save, 0);

RECORD(SaveLayer, kHasPaint_Tag,
       Optional<SkRect> bounds;
       Optional<SkPaint> paint;
       sk_sp<const SkImageFilter> backdrop;
       sk_sp<const SkImage> clipMask;
       Optional<SkMatrix> clipMatrix;
       SkCanvas::SaveLayerFlags saveLayerFlags);

RECORD(SetMatrix, 0,
        TypedMatrix matrix);
RECORD(Concat, 0,
        TypedMatrix matrix);

RECORD(Translate, 0,
        SkScalar dx;
        SkScalar dy);

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
        SkIRect devBounds;
        PreCachedPath path;
        ClipOpAndAA opAA);
RECORD(ClipRRect, 0,
        SkIRect devBounds;
        SkRRect rrect;
        ClipOpAndAA opAA);
RECORD(ClipRect, 0,
        SkIRect devBounds;
        SkRect rect;
        ClipOpAndAA opAA);
RECORD(ClipRegion, 0,
        SkIRect devBounds;
        SkRegion region;
        SkClipOp op);

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
        SkScalar top);
RECORD(DrawImageLattice, kDraw_Tag|kHasImage_Tag|kHasPaint_Tag,
        Optional<SkPaint> paint;
        sk_sp<const SkImage> image;
        int xCount;
        PODArray<int> xDivs;
        int yCount;
        PODArray<int> yDivs;
        int flagCount;
        PODArray<SkCanvas::Lattice::Flags> flags;
        SkIRect src;
        SkRect dst);
RECORD(DrawImageRect, kDraw_Tag|kHasImage_Tag|kHasPaint_Tag,
        Optional<SkPaint> paint;
        sk_sp<const SkImage> image;
        Optional<SkRect> src;
        SkRect dst;
        SkCanvas::SrcRectConstraint constraint);
RECORD(DrawImageNine, kDraw_Tag|kHasImage_Tag|kHasPaint_Tag,
        Optional<SkPaint> paint;
        sk_sp<const SkImage> image;
        SkIRect center;
        SkRect dst);
RECORD(DrawOval, kDraw_Tag|kHasPaint_Tag,
        SkPaint paint;
        SkRect oval);
RECORD(DrawPaint, kDraw_Tag|kHasPaint_Tag,
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
        SkPoint* pts);
RECORD(DrawPosText, kDraw_Tag|kHasText_Tag|kHasPaint_Tag,
        SkPaint paint;
        PODArray<char> text;
        size_t byteLength;
        PODArray<SkPoint> pos);
RECORD(DrawPosTextH, kDraw_Tag|kHasText_Tag|kHasPaint_Tag,
        SkPaint paint;
        PODArray<char> text;
        unsigned byteLength;
        SkScalar y;
        PODArray<SkScalar> xpos);
RECORD(DrawRRect, kDraw_Tag|kHasPaint_Tag,
        SkPaint paint;
        SkRRect rrect);
RECORD(DrawRect, kDraw_Tag|kHasPaint_Tag,
        SkPaint paint;
        SkRect rect);
RECORD(DrawRegion, kDraw_Tag|kHasPaint_Tag,
        SkPaint paint;
        SkRegion region);
RECORD(DrawText, kDraw_Tag|kHasText_Tag|kHasPaint_Tag,
        SkPaint paint;
        PODArray<char> text;
        size_t byteLength;
        SkScalar x;
        SkScalar y);
RECORD(DrawTextBlob, kDraw_Tag|kHasText_Tag|kHasPaint_Tag,
        SkPaint paint;
        sk_sp<const SkTextBlob> blob;
        SkScalar x;
        SkScalar y);
RECORD(DrawTextOnPath, kDraw_Tag|kHasText_Tag|kHasPaint_Tag,
        SkPaint paint;
        PODArray<char> text;
        size_t byteLength;
        PreCachedPath path;
        TypedMatrix matrix);
RECORD(DrawTextRSXform, kDraw_Tag|kHasText_Tag|kHasPaint_Tag,
        SkPaint paint;
        PODArray<char> text;
        size_t byteLength;
        PODArray<SkRSXform> xforms;
        Optional<SkRect> cull);
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
#undef RECORD

}  // namespace SkRecords

#endif//SkRecords_DEFINED
