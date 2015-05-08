/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRecords_DEFINED
#define SkRecords_DEFINED

#include "SkCanvas.h"
#include "SkDrawable.h"
#include "SkPicture.h"
#include "SkTextBlob.h"

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
    M(ClipPath)                                                     \
    M(ClipRRect)                                                    \
    M(ClipRect)                                                     \
    M(ClipRegion)                                                   \
    M(BeginCommentGroup)                                            \
    M(AddComment)                                                   \
    M(EndCommentGroup)                                              \
    M(DrawBitmap)                                                   \
    M(DrawBitmapNine)                                               \
    M(DrawBitmapRectToRect)                                         \
    M(DrawBitmapRectToRectBleed)                                    \
    M(DrawDrawable)                                                 \
    M(DrawImage)                                                    \
    M(DrawImageRect)                                                \
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
    M(DrawRRect)                                                    \
    M(DrawRect)                                                     \
    M(DrawSprite)                                                   \
    M(DrawTextBlob)                                                 \
    M(DrawVertices)

// Defines SkRecords::Type, an enum of all record types.
#define ENUM(T) T##_Type,
enum Type { SK_RECORD_TYPES(ENUM) };
#undef ENUM

// Macros to make it easier to define a record for a draw call with 0 args, 1 args, 2 args, etc.
// These should be clearer when you look at their use below.
#define RECORD0(T)                      \
struct T {                              \
    static const Type kType = T##_Type; \
};

// We try to be flexible about the types the constructors take.  Instead of requring the exact type
// A here, we take any type Z which implicitly casts to A.  This allows the delay_copy() trick to
// work, allowing the caller to decide whether to pass by value or by const&.

#define RECORD1(T, A, a)                \
struct T {                              \
    static const Type kType = T##_Type; \
    template <typename Z>               \
    T(Z a) : a(a) {}                    \
    A a;                                \
};

#define RECORD2(T, A, a, B, b)          \
struct T {                              \
    static const Type kType = T##_Type; \
    template <typename Z, typename Y>   \
    T(Z a, Y b) : a(a), b(b) {}         \
    A a; B b;                           \
};

#define RECORD3(T, A, a, B, b, C, c)              \
struct T {                                        \
    static const Type kType = T##_Type;           \
    template <typename Z, typename Y, typename X> \
    T(Z a, Y b, X c) : a(a), b(b), c(c) {}        \
    A a; B b; C c;                                \
};

#define RECORD4(T, A, a, B, b, C, c, D, d)                    \
struct T {                                                    \
    static const Type kType = T##_Type;                       \
    template <typename Z, typename Y, typename X, typename W> \
    T(Z a, Y b, X c, W d) : a(a), b(b), c(c), d(d) {}         \
    A a; B b; C c; D d;                                       \
};

#define RECORD5(T, A, a, B, b, C, c, D, d, E, e)                          \
struct T {                                                                \
    static const Type kType = T##_Type;                                   \
    template <typename Z, typename Y, typename X, typename W, typename V> \
    T(Z a, Y b, X c, W d, V e) : a(a), b(b), c(c), d(d), e(e) {}          \
    A a; B b; C c; D d; E e;                                              \
};

#define ACT_AS_PTR(ptr)                 \
    operator T*() const { return ptr; } \
    T* operator->() const { return ptr; }

template <typename T>
class RefBox : SkNoncopyable {
public:
    RefBox(T* obj) : fObj(SkSafeRef(obj)) {}
    ~RefBox() { SkSafeUnref(fObj); }

    ACT_AS_PTR(fObj);

private:
    T* fObj;
};

// An Optional doesn't own the pointer's memory, but may need to destroy non-POD data.
template <typename T>
class Optional : SkNoncopyable {
public:
    Optional(T* ptr) : fPtr(ptr) {}
    ~Optional() { if (fPtr) fPtr->~T(); }

    ACT_AS_PTR(fPtr);
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

    ACT_AS_PTR(fPtr);
private:
    T* fPtr;
};

// PODArray doesn't own the pointer's memory, and we assume the data is POD.
template <typename T>
class PODArray {
public:
    PODArray(T* ptr) : fPtr(ptr) {}
    // Default copy and assign.

    ACT_AS_PTR(fPtr);
private:
    T* fPtr;
};

#undef ACT_AS_PTR

// Like SkBitmap, but deep copies pixels if they're not immutable.
// Using this, we guarantee the immutability of all bitmaps we record.
class ImmutableBitmap : SkNoncopyable {
public:
    explicit ImmutableBitmap(const SkBitmap& bitmap) {
        if (bitmap.isImmutable()) {
            fBitmap = bitmap;
        } else {
            bitmap.copyTo(&fBitmap);
        }
        fBitmap.setImmutable();
    }

    int width()  const { return fBitmap.width();  }
    int height() const { return fBitmap.height(); }

    // While the pixels are immutable, SkBitmap itself is not thread-safe, so return a copy.
    SkBitmap shallowCopy() const { return fBitmap; }
private:
    SkBitmap fBitmap;
};

// SkPath::getBounds() isn't thread safe unless we precache the bounds in a singlethreaded context.
// SkPath::cheapComputeDirection() is similar.
// Recording is a convenient time to cache these, or we can delay it to between record and playback.
struct PreCachedPath : public SkPath {
    explicit PreCachedPath(const SkPath& path) : SkPath(path) {
        this->updateBoundsCache();
        SkPath::Direction junk;
        (void)this->cheapComputeDirection(&junk);
    }
};

// Like SkPath::getBounds(), SkMatrix::getType() isn't thread safe unless we precache it.
// This may not cover all SkMatrices used by the picture (e.g. some could be hiding in a shader).
struct TypedMatrix : public SkMatrix {
    explicit TypedMatrix(const SkMatrix& matrix) : SkMatrix(matrix) {
        (void)this->getType();
    }
};

RECORD0(NoOp);

RECORD2(Restore, SkIRect, devBounds, TypedMatrix, matrix);
RECORD0(Save);
RECORD3(SaveLayer, Optional<SkRect>, bounds, Optional<SkPaint>, paint, SkCanvas::SaveFlags, flags);

RECORD1(SetMatrix, TypedMatrix, matrix);

struct RegionOpAndAA {
    RegionOpAndAA(SkRegion::Op op, bool aa) : op(op), aa(aa) {}
    SkRegion::Op op : 31;  // This really only needs to be 3, but there's no win today to do so.
    unsigned     aa :  1;  // MSVC won't pack an enum with an bool, so we call this an unsigned.
};
SK_COMPILE_ASSERT(sizeof(RegionOpAndAA) == 4, RegionOpAndAASize);

RECORD3(ClipPath,   SkIRect, devBounds, PreCachedPath,  path, RegionOpAndAA, opAA);
RECORD3(ClipRRect,  SkIRect, devBounds, SkRRect,       rrect, RegionOpAndAA, opAA);
RECORD3(ClipRect,   SkIRect, devBounds, SkRect,         rect, RegionOpAndAA, opAA);
RECORD3(ClipRegion, SkIRect, devBounds, SkRegion,     region, SkRegion::Op,    op);

RECORD1(BeginCommentGroup, PODArray<char>, description);
RECORD2(AddComment, PODArray<char>, key, PODArray<char>, value);
RECORD0(EndCommentGroup);

// While not strictly required, if you have an SkPaint, it's fastest to put it first.
RECORD4(DrawBitmap, Optional<SkPaint>, paint,
                    ImmutableBitmap, bitmap,
                    SkScalar, left,
                    SkScalar, top);
RECORD4(DrawBitmapNine, Optional<SkPaint>, paint,
                        ImmutableBitmap, bitmap,
                        SkIRect, center,
                        SkRect, dst);
RECORD4(DrawBitmapRectToRect, Optional<SkPaint>, paint,
                              ImmutableBitmap, bitmap,
                              Optional<SkRect>, src,
                              SkRect, dst);
RECORD4(DrawBitmapRectToRectBleed, Optional<SkPaint>, paint,
                                   ImmutableBitmap, bitmap,
                                   Optional<SkRect>, src,
                                   SkRect, dst);
RECORD3(DrawDRRect, SkPaint, paint, SkRRect, outer, SkRRect, inner);
RECORD2(DrawDrawable, SkRect, worstCaseBounds, int32_t, index);
RECORD4(DrawImage, Optional<SkPaint>, paint,
                   RefBox<const SkImage>, image,
                   SkScalar, left,
                   SkScalar, top);
RECORD4(DrawImageRect, Optional<SkPaint>, paint,
                       RefBox<const SkImage>, image,
                       Optional<SkRect>, src,
                       SkRect, dst);
RECORD2(DrawOval, SkPaint, paint, SkRect, oval);
RECORD1(DrawPaint, SkPaint, paint);
RECORD2(DrawPath, SkPaint, paint, PreCachedPath, path);
RECORD3(DrawPicture, Optional<SkPaint>, paint,
                     RefBox<const SkPicture>, picture,
                     TypedMatrix, matrix);
RECORD4(DrawPoints, SkPaint, paint, SkCanvas::PointMode, mode, unsigned, count, SkPoint*, pts);
RECORD4(DrawPosText, SkPaint, paint,
                     PODArray<char>, text,
                     size_t, byteLength,
                     PODArray<SkPoint>, pos);
RECORD5(DrawPosTextH, SkPaint, paint,
                      PODArray<char>, text,
                      unsigned, byteLength,
                      SkScalar, y,
                      PODArray<SkScalar>, xpos);
RECORD2(DrawRRect, SkPaint, paint, SkRRect, rrect);
RECORD2(DrawRect, SkPaint, paint, SkRect, rect);
RECORD4(DrawSprite, Optional<SkPaint>, paint, ImmutableBitmap, bitmap, int, left, int, top);
RECORD5(DrawText, SkPaint, paint,
                  PODArray<char>, text,
                  size_t, byteLength,
                  SkScalar, x,
                  SkScalar, y);
RECORD4(DrawTextBlob, SkPaint, paint,
                      RefBox<const SkTextBlob>, blob,
                      SkScalar, x,
                      SkScalar, y);
RECORD5(DrawTextOnPath, SkPaint, paint,
                        PODArray<char>, text,
                        size_t, byteLength,
                        PreCachedPath, path,
                        TypedMatrix, matrix);

RECORD5(DrawPatch, SkPaint, paint,
                   PODArray<SkPoint>, cubics,
                   PODArray<SkColor>, colors,
                   PODArray<SkPoint>, texCoords,
                   RefBox<SkXfermode>, xmode);

// This guy is so ugly we just write it manually.
struct DrawVertices {
    static const Type kType = DrawVertices_Type;

    DrawVertices(const SkPaint& paint,
                 SkCanvas::VertexMode vmode,
                 int vertexCount,
                 SkPoint* vertices,
                 SkPoint* texs,
                 SkColor* colors,
                 SkXfermode* xmode,
                 uint16_t* indices,
                 int indexCount)
        : paint(paint)
        , vmode(vmode)
        , vertexCount(vertexCount)
        , vertices(vertices)
        , texs(texs)
        , colors(colors)
        , xmode(SkSafeRef(xmode))
        , indices(indices)
        , indexCount(indexCount) {}

    SkPaint paint;
    SkCanvas::VertexMode vmode;
    int vertexCount;
    PODArray<SkPoint> vertices;
    PODArray<SkPoint> texs;
    PODArray<SkColor> colors;
    SkAutoTUnref<SkXfermode> xmode;
    PODArray<uint16_t> indices;
    int indexCount;
};

#undef RECORD0
#undef RECORD1
#undef RECORD2
#undef RECORD3
#undef RECORD4
#undef RECORD5

}  // namespace SkRecords

#endif//SkRecords_DEFINED
