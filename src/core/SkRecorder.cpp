/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBigPicture.h"
#include "SkCanvasPriv.h"
#include "SkPatchUtils.h"
#include "SkPicture.h"
#include "SkPictureUtils.h"
#include "SkRecorder.h"

//#define WRAP_BITMAP_AS_IMAGE

SkDrawableList::~SkDrawableList() {
    fArray.unrefAll();
}

SkBigPicture::SnapshotArray* SkDrawableList::newDrawableSnapshot() {
    const int count = fArray.count();
    if (0 == count) {
        return NULL;
    }
    SkAutoTMalloc<const SkPicture*> pics(count);
    for (int i = 0; i < count; ++i) {
        pics[i] = fArray[i]->newPictureSnapshot();
    }
    return SkNEW_ARGS(SkBigPicture::SnapshotArray, (pics.detach(), count));
}

void SkDrawableList::append(SkDrawable* drawable) {
    *fArray.append() = SkRef(drawable);
}

///////////////////////////////////////////////////////////////////////////////////////////////

SkRecorder::SkRecorder(SkRecord* record, int width, int height, SkMiniRecorder* mr)
    : SkCanvas(SkIRect::MakeWH(width, height), SkCanvas::kConservativeRasterClip_InitFlag)
    , fDrawPictureMode(Record_DrawPictureMode)
    , fApproxBytesUsedBySubPictures(0)
    , fRecord(record)
    , fMiniRecorder(mr) {}

SkRecorder::SkRecorder(SkRecord* record, const SkRect& bounds, SkMiniRecorder* mr)
    : SkCanvas(bounds.roundOut(), SkCanvas::kConservativeRasterClip_InitFlag)
    , fDrawPictureMode(Record_DrawPictureMode)
    , fApproxBytesUsedBySubPictures(0)
    , fRecord(record)
    , fMiniRecorder(mr) {}

void SkRecorder::reset(SkRecord* record, const SkRect& bounds,
                       DrawPictureMode dpm, SkMiniRecorder* mr) {
    this->forgetRecord();
    fDrawPictureMode = dpm;
    fRecord = record;
    this->resetForNextPicture(bounds.roundOut());
    fMiniRecorder = mr;
}

void SkRecorder::forgetRecord() {
    fDrawableList.reset(NULL);
    fApproxBytesUsedBySubPictures = 0;
    fRecord = NULL;
}

// To make appending to fRecord a little less verbose.
#define APPEND(T, ...)                                    \
        if (fMiniRecorder) { this->flushMiniRecorder(); } \
        SkNEW_PLACEMENT_ARGS(fRecord->append<SkRecords::T>(), SkRecords::T, (__VA_ARGS__))

#define TRY_MINIRECORDER(method, ...)                       \
    if (fMiniRecorder && fMiniRecorder->method(__VA_ARGS__)) { return; }

// For methods which must call back into SkCanvas.
#define INHERITED(method, ...) this->SkCanvas::method(__VA_ARGS__)

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

void SkRecorder::flushMiniRecorder() {
    if (fMiniRecorder) {
        SkMiniRecorder* mr = fMiniRecorder;
        fMiniRecorder = nullptr;  // Needs to happen before flushAndReset() or we recurse forever.
        mr->flushAndReset(this);
    }
}

void SkRecorder::onDrawPaint(const SkPaint& paint) {
    APPEND(DrawPaint, paint);
}

void SkRecorder::onDrawPoints(PointMode mode,
                              size_t count,
                              const SkPoint pts[],
                              const SkPaint& paint) {
    APPEND(DrawPoints, paint, mode, SkToUInt(count), this->copy(pts, count));
}

void SkRecorder::onDrawRect(const SkRect& rect, const SkPaint& paint) {
    TRY_MINIRECORDER(drawRect, rect, paint);
    APPEND(DrawRect, paint, rect);
}

void SkRecorder::onDrawOval(const SkRect& oval, const SkPaint& paint) {
    APPEND(DrawOval, paint, oval);
}

void SkRecorder::onDrawRRect(const SkRRect& rrect, const SkPaint& paint) {
    APPEND(DrawRRect, paint, rrect);
}

void SkRecorder::onDrawDRRect(const SkRRect& outer, const SkRRect& inner, const SkPaint& paint) {
    APPEND(DrawDRRect, paint, outer, inner);
}

void SkRecorder::onDrawDrawable(SkDrawable* drawable, const SkMatrix* matrix) {
    if (!fDrawableList) {
        fDrawableList.reset(SkNEW(SkDrawableList));
    }
    fDrawableList->append(drawable);
    APPEND(DrawDrawable, this->copy(matrix), drawable->getBounds(), fDrawableList->count() - 1);
}

void SkRecorder::onDrawPath(const SkPath& path, const SkPaint& paint) {
    TRY_MINIRECORDER(drawPath, path, paint);
    APPEND(DrawPath, paint, path);
}

void SkRecorder::onDrawBitmap(const SkBitmap& bitmap,
                              SkScalar left,
                              SkScalar top,
                              const SkPaint* paint) {
#ifdef WRAP_BITMAP_AS_IMAGE
    SkAutoTUnref<SkImage> image(SkImage::NewFromBitmap(bitmap));
    if (image) {
        this->onDrawImage(image, left, top, paint);
    }
#else
    APPEND(DrawBitmap, this->copy(paint), bitmap, left, top);
#endif
}

void SkRecorder::onDrawBitmapRect(const SkBitmap& bitmap,
                                  const SkRect* src,
                                  const SkRect& dst,
                                  const SkPaint* paint,
                                  SrcRectConstraint constraint) {
#ifdef WRAP_BITMAP_AS_IMAGE
    // TODO: need a way to support the flags for images...
    SkAutoTUnref<SkImage> image(SkImage::NewFromBitmap(bitmap));
    if (image) {
        this->onDrawImageRect(image, src, dst, paint);
    }
#else
    TRY_MINIRECORDER(drawBitmapRect, bitmap, src, dst, paint, constraint);
    if (kFast_SrcRectConstraint == constraint) {
        APPEND(DrawBitmapRectFast, this->copy(paint), bitmap, this->copy(src), dst);
        return;
    }
    SkASSERT(kStrict_SrcRectConstraint == constraint);
    APPEND(DrawBitmapRect, this->copy(paint), bitmap, this->copy(src), dst);
#endif
}

void SkRecorder::onDrawBitmapNine(const SkBitmap& bitmap,
                                  const SkIRect& center,
                                  const SkRect& dst,
                                  const SkPaint* paint) {
#ifdef WRAP_BITMAP_AS_IMAGE
    SkAutoTUnref<SkImage> image(SkImage::NewFromBitmap(bitmap));
    if (image) {
        this->onDrawImageNine(image, center, dst, paint);
    }
#else
    APPEND(DrawBitmapNine, this->copy(paint), bitmap, center, dst);
#endif
}

void SkRecorder::onDrawImage(const SkImage* image, SkScalar left, SkScalar top,
                             const SkPaint* paint) {
    APPEND(DrawImage, this->copy(paint), image, left, top);
}

void SkRecorder::onDrawImageRect(const SkImage* image, const SkRect* src, const SkRect& dst,
                                 const SkPaint* paint, SrcRectConstraint constraint) {
    APPEND(DrawImageRect, this->copy(paint), image, this->copy(src), dst, constraint);
}

void SkRecorder::onDrawImageNine(const SkImage* image, const SkIRect& center,
                                 const SkRect& dst, const SkPaint* paint) {
    APPEND(DrawImageNine, this->copy(paint), image, center, dst);
}

void SkRecorder::onDrawSprite(const SkBitmap& bitmap, int left, int top, const SkPaint* paint) {
    APPEND(DrawSprite, this->copy(paint), bitmap, left, top);
}

void SkRecorder::onDrawText(const void* text, size_t byteLength,
                            SkScalar x, SkScalar y, const SkPaint& paint) {
    APPEND(DrawText,
           paint, this->copy((const char*)text, byteLength), byteLength, x, y);
}

void SkRecorder::onDrawPosText(const void* text, size_t byteLength,
                               const SkPoint pos[], const SkPaint& paint) {
    const unsigned points = paint.countText(text, byteLength);
    APPEND(DrawPosText,
           paint,
           this->copy((const char*)text, byteLength),
           byteLength,
           this->copy(pos, points));
}

void SkRecorder::onDrawPosTextH(const void* text, size_t byteLength,
                                const SkScalar xpos[], SkScalar constY, const SkPaint& paint) {
    const unsigned points = paint.countText(text, byteLength);
    APPEND(DrawPosTextH,
           paint,
           this->copy((const char*)text, byteLength),
           SkToUInt(byteLength),
           constY,
           this->copy(xpos, points));
}

void SkRecorder::onDrawTextOnPath(const void* text, size_t byteLength, const SkPath& path,
                                  const SkMatrix* matrix, const SkPaint& paint) {
    APPEND(DrawTextOnPath,
           paint,
           this->copy((const char*)text, byteLength),
           byteLength,
           path,
           matrix ? *matrix : SkMatrix::I());
}

void SkRecorder::onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                                const SkPaint& paint) {
    TRY_MINIRECORDER(drawTextBlob, blob, x, y, paint);
    APPEND(DrawTextBlob, paint, blob, x, y);
}

void SkRecorder::onDrawPicture(const SkPicture* pic, const SkMatrix* matrix, const SkPaint* paint) {
    if (fDrawPictureMode == Record_DrawPictureMode) {
        fApproxBytesUsedBySubPictures += SkPictureUtils::ApproximateBytesUsed(pic);
        APPEND(DrawPicture, this->copy(paint), pic, matrix ? *matrix : SkMatrix::I());
    } else {
        SkASSERT(fDrawPictureMode == Playback_DrawPictureMode);
        SkAutoCanvasMatrixPaint acmp(this, matrix, paint, pic->cullRect());
        pic->playback(this);
    }
}

void SkRecorder::onDrawVertices(VertexMode vmode,
                                int vertexCount, const SkPoint vertices[],
                                const SkPoint texs[], const SkColor colors[],
                                SkXfermode* xmode,
                                const uint16_t indices[], int indexCount, const SkPaint& paint) {
    APPEND(DrawVertices, paint,
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
    APPEND(DrawPatch, paint,
           cubics ? this->copy(cubics, SkPatchUtils::kNumCtrlPts) : NULL,
           colors ? this->copy(colors, SkPatchUtils::kNumCorners) : NULL,
           texCoords ? this->copy(texCoords, SkPatchUtils::kNumCorners) : NULL,
           xmode);
}

void SkRecorder::onDrawAtlas(const SkImage* atlas, const SkRSXform xform[], const SkRect tex[],
                             const SkColor colors[], int count, SkXfermode::Mode mode,
                             const SkRect* cull, const SkPaint* paint) {
    APPEND(DrawAtlas, this->copy(paint),
           atlas,
           this->copy(xform, count),
           this->copy(tex, count),
           this->copy(colors, count),
           count,
           mode,
           this->copy(cull));
}

void SkRecorder::willSave() {
    APPEND(Save);
}

SkCanvas::SaveLayerStrategy SkRecorder::willSaveLayer(const SkRect* bounds,
                                                      const SkPaint* paint,
                                                      SkCanvas::SaveFlags flags) {
    APPEND(SaveLayer, this->copy(bounds), this->copy(paint), flags);
    return SkCanvas::kNoLayer_SaveLayerStrategy;
}

void SkRecorder::didRestore() {
    APPEND(Restore, this->devBounds(), this->getTotalMatrix());
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
    SkRecords::RegionOpAndAA opAA(op, kSoft_ClipEdgeStyle == edgeStyle);
    APPEND(ClipRect, this->devBounds(), rect, opAA);
}

void SkRecorder::onClipRRect(const SkRRect& rrect, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
    INHERITED(onClipRRect, rrect, op, edgeStyle);
    SkRecords::RegionOpAndAA opAA(op, kSoft_ClipEdgeStyle == edgeStyle);
    APPEND(ClipRRect, this->devBounds(), rrect, opAA);
}

void SkRecorder::onClipPath(const SkPath& path, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
    INHERITED(onClipPath, path, op, edgeStyle);
    SkRecords::RegionOpAndAA opAA(op, kSoft_ClipEdgeStyle == edgeStyle);
    APPEND(ClipPath, this->devBounds(), path, opAA);
}

void SkRecorder::onClipRegion(const SkRegion& deviceRgn, SkRegion::Op op) {
    INHERITED(onClipRegion, deviceRgn, op);
    APPEND(ClipRegion, this->devBounds(), deviceRgn, op);
}

