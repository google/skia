/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRecorder.h"
#include "SkPatchUtils.h"
#include "SkPicture.h"

// SkCanvas will fail in mysterious ways if it doesn't know the real width and height.
SkRecorder::SkRecorder(SkRecord* record, int width, int height)
    : SkCanvas(width, height, SkCanvas::kConservativeRasterClip_InitFlag)
    , fRecord(record)
    , fSaveLayerCount(0) {}

void SkRecorder::forgetRecord() {
    fRecord = NULL;
}

// To make appending to fRecord a little less verbose.
#define APPEND(T, ...) \
        SkNEW_PLACEMENT_ARGS(fRecord->append<SkRecords::T>(), SkRecords::T, (__VA_ARGS__))

// For methods which must call back into SkCanvas.
#define INHERITED(method, ...) this->SkCanvas::method(__VA_ARGS__)

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
T* SkRecorder::copy(const T src[], size_t count) {
    if (NULL == src) {
        return NULL;
    }
    T* dst = fRecord->alloc<T>(count);
    for (size_t i = 0; i < count; i++) {
        SkNEW_PLACEMENT_ARGS(dst + i, T, (src[i]));
    }
    return dst;
}

// Specialization for copying strings, using memcpy.
// This measured around 2x faster for copying code points,
// but I found no corresponding speedup for other arrays.
template <>
char* SkRecorder::copy(const char src[], size_t count) {
    if (NULL == src) {
        return NULL;
    }
    char* dst = fRecord->alloc<char>(count);
    memcpy(dst, src, count);
    return dst;
}

// As above, assuming and copying a terminating \0.
template <>
char* SkRecorder::copy(const char* src) {
    return this->copy(src, strlen(src)+1);
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
    APPEND(DrawPoints, delay_copy(paint), mode, count, this->copy(pts, count));
}

void SkRecorder::drawRect(const SkRect& rect, const SkPaint& paint) {
    APPEND(DrawRect, delay_copy(paint), rect);
}

void SkRecorder::drawOval(const SkRect& oval, const SkPaint& paint) {
    APPEND(DrawOval, delay_copy(paint), oval);
}

void SkRecorder::drawRRect(const SkRRect& rrect, const SkPaint& paint) {
    APPEND(DrawRRect, delay_copy(paint), rrect);
}

void SkRecorder::onDrawDRRect(const SkRRect& outer, const SkRRect& inner, const SkPaint& paint) {
    APPEND(DrawDRRect, delay_copy(paint), outer, inner);
}

void SkRecorder::drawPath(const SkPath& path, const SkPaint& paint) {
    APPEND(DrawPath, delay_copy(paint), delay_copy(path));
}

void SkRecorder::drawBitmap(const SkBitmap& bitmap,
                            SkScalar left,
                            SkScalar top,
                            const SkPaint* paint) {
    APPEND(DrawBitmap, this->copy(paint), delay_copy(bitmap), left, top);
}

void SkRecorder::drawBitmapRectToRect(const SkBitmap& bitmap,
                                      const SkRect* src,
                                      const SkRect& dst,
                                      const SkPaint* paint,
                                      DrawBitmapRectFlags flags) {
    APPEND(DrawBitmapRectToRect,
           this->copy(paint), delay_copy(bitmap), this->copy(src), dst, flags);
}

void SkRecorder::drawBitmapMatrix(const SkBitmap& bitmap,
                                  const SkMatrix& matrix,
                                  const SkPaint* paint) {
    APPEND(DrawBitmapMatrix, this->copy(paint), delay_copy(bitmap), matrix);
}

void SkRecorder::drawBitmapNine(const SkBitmap& bitmap,
                                const SkIRect& center,
                                const SkRect& dst,
                                const SkPaint* paint) {
    APPEND(DrawBitmapNine, this->copy(paint), delay_copy(bitmap), center, dst);
}

void SkRecorder::drawSprite(const SkBitmap& bitmap, int left, int top, const SkPaint* paint) {
    APPEND(DrawSprite, this->copy(paint), delay_copy(bitmap), left, top);
}

void SkRecorder::onDrawText(const void* text, size_t byteLength,
                            SkScalar x, SkScalar y, const SkPaint& paint) {
    APPEND(DrawText,
           delay_copy(paint), this->copy((const char*)text, byteLength), byteLength, x, y);
}

void SkRecorder::onDrawPosText(const void* text, size_t byteLength,
                               const SkPoint pos[], const SkPaint& paint) {
    const unsigned points = paint.countText(text, byteLength);
    APPEND(DrawPosText,
           delay_copy(paint),
           this->copy((const char*)text, byteLength),
           byteLength,
           this->copy(pos, points));
}

void SkRecorder::onDrawPosTextH(const void* text, size_t byteLength,
                                const SkScalar xpos[], SkScalar constY, const SkPaint& paint) {
    const unsigned points = paint.countText(text, byteLength);
    APPEND(DrawPosTextH,
           delay_copy(paint),
           this->copy((const char*)text, byteLength),
           byteLength,
           this->copy(xpos, points),
           constY);
}

void SkRecorder::onDrawTextOnPath(const void* text, size_t byteLength, const SkPath& path,
                                  const SkMatrix* matrix, const SkPaint& paint) {
    APPEND(DrawTextOnPath,
           delay_copy(paint),
           this->copy((const char*)text, byteLength),
           byteLength,
           delay_copy(path),
           this->copy(matrix));
}

void SkRecorder::onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                                const SkPaint& paint) {
    APPEND(DrawTextBlob, delay_copy(paint), blob, x, y);
}

void SkRecorder::onDrawPicture(const SkPicture* pic, const SkMatrix* matrix, const SkPaint* paint) {
    APPEND(DrawPicture, this->copy(paint), pic, this->copy(matrix));
}

void SkRecorder::drawVertices(VertexMode vmode,
                              int vertexCount, const SkPoint vertices[],
                              const SkPoint texs[], const SkColor colors[],
                              SkXfermode* xmode,
                              const uint16_t indices[], int indexCount, const SkPaint& paint) {
    APPEND(DrawVertices, delay_copy(paint),
                         vmode,
                         vertexCount,
                         this->copy(vertices, vertexCount),
                         texs ? this->copy(texs, vertexCount) : NULL,
                         colors ? this->copy(colors, vertexCount) : NULL,
                         xmode,
                         this->copy(indices, indexCount),
                         indexCount);
}

void SkRecorder::onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                             const SkPoint texCoords[4], SkXfermode* xmode, const SkPaint& paint) {
    APPEND(DrawPatch, delay_copy(paint),
           cubics ? this->copy(cubics, SkPatchUtils::kNumCtrlPts) : NULL,
           colors ? this->copy(colors, SkPatchUtils::kNumCorners) : NULL,
           texCoords ? this->copy(texCoords, SkPatchUtils::kNumCorners) : NULL,
           xmode);
}

void SkRecorder::willSave() {
    fSaveIsSaveLayer.push(false);
    APPEND(Save);
}

SkCanvas::SaveLayerStrategy SkRecorder::willSaveLayer(const SkRect* bounds,
                                                      const SkPaint* paint,
                                                      SkCanvas::SaveFlags flags) {
    fSaveLayerCount++;
    fSaveIsSaveLayer.push(true);
    APPEND(SaveLayer, this->copy(bounds), this->copy(paint), flags);
    return SkCanvas::kNoLayer_SaveLayerStrategy;
}

void SkRecorder::didRestore() {
    SkBool8 saveLayer;
    fSaveIsSaveLayer.pop(&saveLayer);
    if (saveLayer) {
        fSaveLayerCount--;
    }
    APPEND(Restore, this->devBounds(), this->getTotalMatrix());
}

void SkRecorder::onPushCull(const SkRect& rect) {
    APPEND(PushCull, rect);
}

void SkRecorder::onPopCull() {
    APPEND(PopCull);
}

void SkRecorder::didConcat(const SkMatrix& matrix) {
    this->didSetMatrix(this->getTotalMatrix());
}

void SkRecorder::didSetMatrix(const SkMatrix& matrix) {
    SkDEVCODE(if (matrix != this->getTotalMatrix()) {
        matrix.dump();
        this->getTotalMatrix().dump();
        SkASSERT(matrix == this->getTotalMatrix());
    })
    APPEND(SetMatrix, matrix);
}

void SkRecorder::onClipRect(const SkRect& rect, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
    INHERITED(onClipRect, rect, op, edgeStyle);
    APPEND(ClipRect, this->devBounds(), rect, op, edgeStyle == kSoft_ClipEdgeStyle);
}

void SkRecorder::onClipRRect(const SkRRect& rrect, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
    INHERITED(onClipRRect, rrect, op, edgeStyle);
    APPEND(ClipRRect, this->devBounds(), rrect, op, edgeStyle == kSoft_ClipEdgeStyle);
}

void SkRecorder::onClipPath(const SkPath& path, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
    INHERITED(onClipPath, path, op, edgeStyle);
    APPEND(ClipPath, this->devBounds(), delay_copy(path), op, edgeStyle == kSoft_ClipEdgeStyle);
}

void SkRecorder::onClipRegion(const SkRegion& deviceRgn, SkRegion::Op op) {
    INHERITED(onClipRegion, deviceRgn, op);
    APPEND(ClipRegion, this->devBounds(), delay_copy(deviceRgn), op);
}

void SkRecorder::beginCommentGroup(const char* description) {
    APPEND(BeginCommentGroup, this->copy(description));
}

void SkRecorder::addComment(const char* key, const char* value) {
    APPEND(AddComment, this->copy(key), this->copy(value));
}

void SkRecorder::endCommentGroup() {
    APPEND(EndCommentGroup);
}

bool SkRecorder::isDrawingToLayer() const {
    return fSaveLayerCount > 0;
}

void SkRecorder::drawData(const void* data, size_t length) {
    APPEND(DrawData, copy((const char*)data), length);
}
