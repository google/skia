/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRecords_DEFINED
#define SkRecords_DEFINED

#include "SkCanvas.h"

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
    M(PushCull)                                                     \
    M(PopCull)                                                      \
    M(PairedPushCull)         /*From SkRecordAnnotateCullingPairs*/ \
    M(Concat)                                                       \
    M(SetMatrix)                                                    \
    M(ClipPath)                                                     \
    M(ClipRRect)                                                    \
    M(ClipRect)                                                     \
    M(ClipRegion)                                                   \
    M(Clear)                                                        \
    M(DrawBitmap)                                                   \
    M(DrawBitmapMatrix)                                             \
    M(DrawBitmapNine)                                               \
    M(DrawBitmapRectToRect)                                         \
    M(DrawDRRect)                                                   \
    M(DrawOval)                                                     \
    M(DrawPaint)                                                    \
    M(DrawPath)                                                     \
    M(DrawPoints)                                                   \
    M(DrawPosText)                                                  \
    M(DrawPosTextH)                                                 \
    M(DrawRRect)                                                    \
    M(DrawRect)                                                     \
    M(DrawSprite)                                                   \
    M(DrawText)                                                     \
    M(DrawTextOnPath)                                               \
    M(DrawVertices)                                                 \
    M(BoundedDrawPosTextH)    /*From SkRecordBoundDrawPosTextH*/

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

#define ACT_AS_PTR(ptr)                       \
    operator T*() { return ptr; }             \
    operator const T*() const { return ptr; } \
    T* operator->() { return ptr; }           \
    const T* operator->() const { return ptr; }

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
class ImmutableBitmap {
public:
    explicit ImmutableBitmap(const SkBitmap& bitmap) {
        if (bitmap.isImmutable()) {
            fBitmap = bitmap;
        } else {
            bitmap.copyTo(&fBitmap);
        }
        fBitmap.setImmutable();
    }

    operator const SkBitmap& () const { return fBitmap; }

private:
    SkBitmap fBitmap;
};

RECORD0(NoOp);

RECORD0(Restore);
RECORD0(Save);
RECORD3(SaveLayer, Optional<SkRect>, bounds, Optional<SkPaint>, paint, SkCanvas::SaveFlags, flags);

RECORD1(PushCull, SkRect, rect);
RECORD0(PopCull);

RECORD1(Concat, SkMatrix, matrix);
RECORD1(SetMatrix, SkMatrix, matrix);

RECORD3(ClipPath, SkPath, path, SkRegion::Op, op, bool, doAA);
RECORD3(ClipRRect, SkRRect, rrect, SkRegion::Op, op, bool, doAA);
RECORD3(ClipRect, SkRect, rect, SkRegion::Op, op, bool, doAA);
RECORD2(ClipRegion, SkRegion, region, SkRegion::Op, op);

RECORD1(Clear, SkColor, color);
// While not strictly required, if you have an SkPaint, it's fastest to put it first.
RECORD4(DrawBitmap, Optional<SkPaint>, paint,
                    ImmutableBitmap, bitmap,
                    SkScalar, left,
                    SkScalar, top);
RECORD3(DrawBitmapMatrix, Optional<SkPaint>, paint, ImmutableBitmap, bitmap, SkMatrix, matrix);
RECORD4(DrawBitmapNine, Optional<SkPaint>, paint,
                        ImmutableBitmap, bitmap,
                        SkIRect, center,
                        SkRect, dst);
RECORD5(DrawBitmapRectToRect, Optional<SkPaint>, paint,
                              ImmutableBitmap, bitmap,
                              Optional<SkRect>, src,
                              SkRect, dst,
                              SkCanvas::DrawBitmapRectFlags, flags);
RECORD3(DrawDRRect, SkPaint, paint, SkRRect, outer, SkRRect, inner);
RECORD2(DrawOval, SkPaint, paint, SkRect, oval);
RECORD1(DrawPaint, SkPaint, paint);
RECORD2(DrawPath, SkPaint, paint, SkPath, path);
RECORD4(DrawPoints, SkPaint, paint, SkCanvas::PointMode, mode, size_t, count, SkPoint*, pts);
RECORD4(DrawPosText, SkPaint, paint,
                     PODArray<char>, text,
                     size_t, byteLength,
                     PODArray<SkPoint>, pos);
RECORD5(DrawPosTextH, SkPaint, paint,
                      PODArray<char>, text,
                      size_t, byteLength,
                      PODArray<SkScalar>, xpos,
                      SkScalar, y);
RECORD2(DrawRRect, SkPaint, paint, SkRRect, rrect);
RECORD2(DrawRect, SkPaint, paint, SkRect, rect);
RECORD4(DrawSprite, Optional<SkPaint>, paint, ImmutableBitmap, bitmap, int, left, int, top);
RECORD5(DrawText, SkPaint, paint,
                  PODArray<char>, text,
                  size_t, byteLength,
                  SkScalar, x,
                  SkScalar, y);
RECORD5(DrawTextOnPath, SkPaint, paint,
                        PODArray<char>, text,
                        size_t, byteLength,
                        SkPath, path,
                        Optional<SkMatrix>, matrix);

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

// Records added by optimizations.
RECORD2(PairedPushCull, Adopted<PushCull>, base, unsigned, skip);
RECORD3(BoundedDrawPosTextH, Adopted<DrawPosTextH>, base, SkScalar, minY, SkScalar, maxY);

#undef RECORD0
#undef RECORD1
#undef RECORD2
#undef RECORD3
#undef RECORD4
#undef RECORD5

}  // namespace SkRecords

#endif//SkRecords_DEFINED
