#include "SkRecorder.h"
#include "SkPicture.h"

// SkCanvas will fail in mysterious ways if it doesn't know the real width and height.
SkRecorder::SkRecorder(SkRecord* record, int width, int height)
    : SkCanvas(width, height), fRecord(record) {}

// To make appending to fRecord a little less verbose.
#define APPEND(T, ...) \
        SkNEW_PLACEMENT_ARGS(fRecord->append<SkRecords::T>(), SkRecords::T, (__VA_ARGS__))

// The structs we're creating all copy their constructor arguments.  Given the way the SkRecords
// framework works, sometimes they happen to technically be copied twice, which is fine and elided
// into a single copy unless the class has a non-trivial copy constructor.  For classes with
// non-trivial copy constructors, we skip the first copy (and its destruction) by wrapping the value
// with delay_copy(), forcing the argument to be passed by const&.
//
// This is used below for SkBitmap, SkPaint, SkPath, and SkRegion, which all have non-trivial copy
// constructors and destructors.  You'll know you've got a good candidate T if you see ~T() show up
// unexpectedly on a profile of record time.  Otherwise don't bother.
template <typename T>
class Reference {
public:
    Reference(const T& x) : fX(x) {}
    operator const T&() const { return fX; }
private:
    const T& fX;
};

template <typename T>
static Reference<T> delay_copy(const T& x) { return Reference<T>(x); }

// Use copy() only for optional arguments, to be copied if present or skipped if not.
// (For most types we just pass by value and let copy constructors do their thing.)
template <typename T>
T* SkRecorder::copy(const T* src) {
    if (NULL == src) {
        return NULL;
    }
    return SkNEW_PLACEMENT_ARGS(fRecord->alloc<T>(), T, (*src));
}

// This copy() is for arrays.
// It will work with POD or non-POD, though currently we only use it for POD.
template <typename T>
T* SkRecorder::copy(const T src[], unsigned count) {
    if (NULL == src) {
        return NULL;
    }
    T* dst = fRecord->alloc<T>(count);
    for (unsigned i = 0; i < count; i++) {
        SkNEW_PLACEMENT_ARGS(dst + i, T, (src[i]));
    }
    return dst;
}

// Specialization for copying strings, using memcpy.
// This measured around 2x faster for copying code points,
// but I found no corresponding speedup for other arrays.
template <>
char* SkRecorder::copy(const char src[], unsigned count) {
    if (NULL == src) {
        return NULL;
    }
    char* dst = fRecord->alloc<char>(count);
    memcpy(dst, src, count);
    return dst;
}

void SkRecorder::clear(SkColor color) {
    APPEND(Clear, color);
}

void SkRecorder::drawPaint(const SkPaint& paint) {
    APPEND(DrawPaint, delay_copy(paint));
}

void SkRecorder::drawPoints(PointMode mode,
                            size_t count,
                            const SkPoint pts[],
                            const SkPaint& paint) {
    APPEND(DrawPoints, mode, count, this->copy(pts, count), delay_copy(paint));
}

void SkRecorder::drawRect(const SkRect& rect, const SkPaint& paint) {
    APPEND(DrawRect, rect, delay_copy(paint));
}

void SkRecorder::drawOval(const SkRect& oval, const SkPaint& paint) {
    APPEND(DrawOval, oval, delay_copy(paint));
}

void SkRecorder::drawRRect(const SkRRect& rrect, const SkPaint& paint) {
    APPEND(DrawRRect, rrect, delay_copy(paint));
}

void SkRecorder::drawPath(const SkPath& path, const SkPaint& paint) {
    APPEND(DrawPath, delay_copy(path), delay_copy(paint));
}

void SkRecorder::drawBitmap(const SkBitmap& bitmap,
                            SkScalar left,
                            SkScalar top,
                            const SkPaint* paint) {
    APPEND(DrawBitmap, delay_copy(bitmap), left, top, this->copy(paint));
}

void SkRecorder::drawBitmapRectToRect(const SkBitmap& bitmap,
                                      const SkRect* src,
                                      const SkRect& dst,
                                      const SkPaint* paint,
                                      DrawBitmapRectFlags flags) {
    APPEND(DrawBitmapRectToRect,
           delay_copy(bitmap), this->copy(src), dst, this->copy(paint), flags);
}

void SkRecorder::drawBitmapMatrix(const SkBitmap& bitmap,
                                  const SkMatrix& matrix,
                                  const SkPaint* paint) {
    APPEND(DrawBitmapMatrix, delay_copy(bitmap), matrix, this->copy(paint));
}

void SkRecorder::drawBitmapNine(const SkBitmap& bitmap,
                                const SkIRect& center,
                                const SkRect& dst,
                                const SkPaint* paint) {
    APPEND(DrawBitmapNine, delay_copy(bitmap), center, dst, this->copy(paint));
}

void SkRecorder::drawSprite(const SkBitmap& bitmap, int left, int top, const SkPaint* paint) {
    APPEND(DrawSprite, delay_copy(bitmap), left, top, this->copy(paint));
}

void SkRecorder::drawText(const void* text, size_t byteLength,
                          SkScalar x, SkScalar y, const SkPaint& paint) {
    APPEND(DrawText,
           this->copy((const char*)text, byteLength), byteLength, x, y, delay_copy(paint));
}

void SkRecorder::drawPosText(const void* text, size_t byteLength,
                             const SkPoint pos[], const SkPaint& paint) {
    const unsigned points = paint.countText(text, byteLength);
    APPEND(DrawPosText,
           this->copy((const char*)text, byteLength), byteLength,
           this->copy(pos, points), delay_copy(paint));
}

void SkRecorder::drawPosTextH(const void* text, size_t byteLength,
                              const SkScalar xpos[], SkScalar constY, const SkPaint& paint) {
    const unsigned points = paint.countText(text, byteLength);
    APPEND(DrawPosTextH,
           this->copy((const char*)text, byteLength), byteLength,
           this->copy(xpos, points), constY, delay_copy(paint));
}

void SkRecorder::drawTextOnPath(const void* text, size_t byteLength,
                                const SkPath& path, const SkMatrix* matrix, const SkPaint& paint) {
    APPEND(DrawTextOnPath,
           this->copy((const char*)text, byteLength), byteLength,
           delay_copy(path), this->copy(matrix), delay_copy(paint));
}

void SkRecorder::drawPicture(SkPicture& picture) {
    picture.draw(this);
}

void SkRecorder::drawVertices(VertexMode vmode,
                              int vertexCount, const SkPoint vertices[],
                              const SkPoint texs[], const SkColor colors[],
                              SkXfermode* xmode,
                              const uint16_t indices[], int indexCount, const SkPaint& paint) {
    APPEND(DrawVertices, vmode,
                         vertexCount,
                         this->copy(vertices, vertexCount),
                         texs ? this->copy(texs, vertexCount) : NULL,
                         colors ? this->copy(colors, vertexCount) : NULL,
                         xmode,
                         this->copy(indices, indexCount),
                         indexCount,
                         delay_copy(paint));
}

void SkRecorder::willSave(SkCanvas::SaveFlags flags) {
    APPEND(Save, flags);
}

SkCanvas::SaveLayerStrategy SkRecorder::willSaveLayer(const SkRect* bounds,
                                                      const SkPaint* paint,
                                                      SkCanvas::SaveFlags flags) {
    APPEND(SaveLayer, this->copy(bounds), this->copy(paint), flags);
    return SkCanvas::kNoLayer_SaveLayerStrategy;
}

void SkRecorder::willRestore() {
    APPEND(Restore);
}

void SkRecorder::didConcat(const SkMatrix& matrix) {
    APPEND(Concat, matrix);
}

void SkRecorder::didSetMatrix(const SkMatrix& matrix) {
    APPEND(SetMatrix, matrix);
}

void SkRecorder::onDrawDRRect(const SkRRect& outer, const SkRRect& inner, const SkPaint& paint) {
    APPEND(DrawDRRect, outer, inner, delay_copy(paint));
}

void SkRecorder::onClipRect(const SkRect& rect, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
    APPEND(ClipRect, rect, op, edgeStyle == kSoft_ClipEdgeStyle);
}

void SkRecorder::onClipRRect(const SkRRect& rrect, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
    APPEND(ClipRRect, rrect, op, edgeStyle == kSoft_ClipEdgeStyle);
}

void SkRecorder::onClipPath(const SkPath& path, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
    APPEND(ClipPath, delay_copy(path), op, edgeStyle == kSoft_ClipEdgeStyle);
}

void SkRecorder::onClipRegion(const SkRegion& deviceRgn, SkRegion::Op op) {
    APPEND(ClipRegion, delay_copy(deviceRgn), op);
}
