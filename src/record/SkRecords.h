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
#define SK_RECORD_TYPES(M)  \
    M(Restore)              \
    M(Save)                 \
    M(SaveLayer)            \
    M(Concat)               \
    M(SetMatrix)            \
    M(ClipPath)             \
    M(ClipRRect)            \
    M(ClipRect)             \
    M(ClipRegion)           \
    M(Clear)                \
    M(DrawBitmap)           \
    M(DrawBitmapMatrix)     \
    M(DrawBitmapNine)       \
    M(DrawBitmapRectToRect) \
    M(DrawDRRect)           \
    M(DrawOval)             \
    M(DrawPaint)            \
    M(DrawPath)             \
    M(DrawPoints)           \
    M(DrawPosText)          \
    M(DrawPosTextH)         \
    M(DrawRRect)            \
    M(DrawRect)             \
    M(DrawSprite)           \
    M(DrawText)             \
    M(DrawTextOnPath)       \
    M(DrawVertices)         \
    M(PushCull)             \
    M(PopCull)

// Defines SkRecords::Type, an enum of all record types.
#define ENUM(T) T##_Type,
enum Type { SK_RECORD_TYPES(ENUM) };
#undef ENUM

// Macros to make it easier to define a record for a draw call with 0 args, 1 args, 2 args, etc.
// These should be clearer when you look at their use below.
#define RECORD0(T)                      \
struct T {                              \
    static const Type kType = T##_Type; \
    T() {}                              \
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

// Pointers here represent either an optional value or an array if accompanied by a count.
// None of these records manages the lifetimes of pointers, except for DrawVertices handling its
// Xfermode specially.

RECORD0(Restore);
RECORD1(Save, SkCanvas::SaveFlags, flags);
RECORD3(SaveLayer, SkRect*, bounds, SkPaint*, paint, SkCanvas::SaveFlags, flags);

RECORD1(PushCull, SkRect, rect);
RECORD0(PopCull);

RECORD1(Concat, SkMatrix, matrix);
RECORD1(SetMatrix, SkMatrix, matrix);

RECORD3(ClipPath, SkPath, path, SkRegion::Op, op, bool, doAA);
RECORD3(ClipRRect, SkRRect, rrect, SkRegion::Op, op, bool, doAA);
RECORD3(ClipRect, SkRect, rect, SkRegion::Op, op, bool, doAA);
RECORD2(ClipRegion, SkRegion, region, SkRegion::Op, op);

RECORD1(Clear, SkColor, color);
RECORD4(DrawBitmap, ImmutableBitmap, bitmap, SkScalar, left, SkScalar, top, SkPaint*, paint);
RECORD3(DrawBitmapMatrix, ImmutableBitmap, bitmap, SkMatrix, matrix, SkPaint*, paint);
RECORD4(DrawBitmapNine, ImmutableBitmap, bitmap, SkIRect, center, SkRect, dst, SkPaint*, paint);
RECORD5(DrawBitmapRectToRect, ImmutableBitmap, bitmap,
                              SkRect*, src,
                              SkRect, dst,
                              SkPaint*, paint,
                              SkCanvas::DrawBitmapRectFlags, flags);
RECORD3(DrawDRRect, SkRRect, outer, SkRRect, inner, SkPaint, paint);
RECORD2(DrawOval, SkRect, oval, SkPaint, paint);
RECORD1(DrawPaint, SkPaint, paint);
RECORD2(DrawPath, SkPath, path, SkPaint, paint);
RECORD4(DrawPoints, SkCanvas::PointMode, mode, size_t, count, SkPoint*, pts, SkPaint, paint);
RECORD4(DrawPosText, char*, text, size_t, byteLength, SkPoint*, pos, SkPaint, paint);
RECORD5(DrawPosTextH, char*, text,
                      size_t, byteLength,
                      SkScalar*, xpos,
                      SkScalar, y,
                      SkPaint, paint);
RECORD2(DrawRRect, SkRRect, rrect, SkPaint, paint);
RECORD2(DrawRect, SkRect, rect, SkPaint, paint);
RECORD4(DrawSprite, ImmutableBitmap, bitmap, int, left, int, top, SkPaint*, paint);
RECORD5(DrawText, char*, text, size_t, byteLength, SkScalar, x, SkScalar, y, SkPaint, paint);
RECORD5(DrawTextOnPath, char*, text,
                        size_t, byteLength,
                        SkPath, path,
                        SkMatrix*, matrix,
                        SkPaint, paint);

// This guy is so ugly we just write it manually.
struct DrawVertices {
    static const Type kType = DrawVertices_Type;

    DrawVertices(SkCanvas::VertexMode vmode,
                 int vertexCount,
                 SkPoint* vertices,
                 SkPoint* texs,
                 SkColor* colors,
                 SkXfermode* xmode,
                 uint16_t* indices,
                 int indexCount,
                 const SkPaint& paint)
        : vmode(vmode)
        , vertexCount(vertexCount)
        , vertices(vertices)
        , texs(texs)
        , colors(colors)
        , xmode(SkSafeRef(xmode))
        , indices(indices)
        , indexCount(indexCount)
        , paint(paint) {}

    SkCanvas::VertexMode vmode;
    int vertexCount;
    SkPoint* vertices;
    SkPoint* texs;
    SkColor* colors;
    SkAutoTUnref<SkXfermode> xmode;
    uint16_t* indices;
    int indexCount;
    SkPaint paint;
};

#undef RECORD0
#undef RECORD1
#undef RECORD2
#undef RECORD3
#undef RECORD4
#undef RECORD5

}  // namespace SkRecords

#endif//SkRecords_DEFINED
