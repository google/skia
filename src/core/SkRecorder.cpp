/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBigPicture.h"
#include "SkCanvasPriv.h"
#include "SkImage.h"
#include "SkPatchUtils.h"
#include "SkPicture.h"
#include "SkRecorder.h"
#include "SkSurface.h"

SkDrawableList::~SkDrawableList() {
    fArray.unrefAll();
}

SkBigPicture::SnapshotArray* SkDrawableList::newDrawableSnapshot() {
    const int count = fArray.count();
    if (0 == count) {
        return nullptr;
    }
    SkAutoTMalloc<const SkPicture*> pics(count);
    for (int i = 0; i < count; ++i) {
        pics[i] = fArray[i]->newPictureSnapshot();
    }
    return new SkBigPicture::SnapshotArray(pics.release(), count);
}

void SkDrawableList::append(SkDrawable* drawable) {
    *fArray.append() = SkRef(drawable);
}

///////////////////////////////////////////////////////////////////////////////////////////////

SkRecorder::SkRecorder(SkRecord* record, int width, int height, SkMiniRecorder* mr)
    : SkNoDrawCanvas(width, height)
    , fDrawPictureMode(Record_DrawPictureMode)
    , fApproxBytesUsedBySubPictures(0)
    , fRecord(record)
    , fMiniRecorder(mr) {}

SkRecorder::SkRecorder(SkRecord* record, const SkRect& bounds, SkMiniRecorder* mr)
    : SkNoDrawCanvas(bounds.roundOut())
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
    fDrawableList.reset(nullptr);
    fApproxBytesUsedBySubPictures = 0;
    fRecord = nullptr;
}

// To make appending to fRecord a little less verbose.
#define APPEND(T, ...)             \
    if (fMiniRecorder) {           \
        this->flushMiniRecorder(); \
    }                              \
    new (fRecord->append<SkRecords::T>()) SkRecords::T{__VA_ARGS__}

#define TRY_MINIRECORDER(method, ...)                       \
    if (fMiniRecorder && fMiniRecorder->method(__VA_ARGS__)) { return; }

// For methods which must call back into SkNoDrawCanvas.
#define INHERITED(method, ...) this->SkNoDrawCanvas::method(__VA_ARGS__)

// Use copy() only for optional arguments, to be copied if present or skipped if not.
// (For most types we just pass by value and let copy constructors do their thing.)
template <typename T>
T* SkRecorder::copy(const T* src) {
    if (nullptr == src) {
        return nullptr;
    }
    return new (fRecord->alloc<T>()) T(*src);
}

// This copy() is for arrays.
// It will work with POD or non-POD, though currently we only use it for POD.
template <typename T>
T* SkRecorder::copy(const T src[], size_t count) {
    if (nullptr == src) {
        return nullptr;
    }
    T* dst = fRecord->alloc<T>(count);
    for (size_t i = 0; i < count; i++) {
        new (dst + i) T(src[i]);
    }
    return dst;
}

// Specialization for copying strings, using memcpy.
// This measured around 2x faster for copying code points,
// but I found no corresponding speedup for other arrays.
template <>
char* SkRecorder::copy(const char src[], size_t count) {
    if (nullptr == src) {
        return nullptr;
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

void SkRecorder::onDrawRegion(const SkRegion& region, const SkPaint& paint) {
    APPEND(DrawRegion, paint, region);
}

void SkRecorder::onDrawOval(const SkRect& oval, const SkPaint& paint) {
    APPEND(DrawOval, paint, oval);
}

void SkRecorder::onDrawArc(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle,
                           bool useCenter, const SkPaint& paint) {
    APPEND(DrawArc, paint, oval, startAngle, sweepAngle, useCenter);
}

void SkRecorder::onDrawRRect(const SkRRect& rrect, const SkPaint& paint) {
    APPEND(DrawRRect, paint, rrect);
}

void SkRecorder::onDrawDRRect(const SkRRect& outer, const SkRRect& inner, const SkPaint& paint) {
    APPEND(DrawDRRect, paint, outer, inner);
}

void SkRecorder::onDrawDrawable(SkDrawable* drawable, const SkMatrix* matrix) {
    if (fDrawPictureMode == Record_DrawPictureMode) {
        if (!fDrawableList) {
            fDrawableList.reset(new SkDrawableList);
        }
        fDrawableList->append(drawable);
        APPEND(DrawDrawable, this->copy(matrix), drawable->getBounds(), fDrawableList->count() - 1);
    } else {
        SkASSERT(fDrawPictureMode == Playback_DrawPictureMode);
        drawable->draw(this, matrix);
    }
}

void SkRecorder::onDrawPath(const SkPath& path, const SkPaint& paint) {
    TRY_MINIRECORDER(drawPath, path, paint);
    APPEND(DrawPath, paint, path);
}

void SkRecorder::onDrawBitmap(const SkBitmap& bitmap,
                              SkScalar left,
                              SkScalar top,
                              const SkPaint* paint) {
    sk_sp<SkImage> image = SkImage::MakeFromBitmap(bitmap);
    if (image) {
        this->onDrawImage(image.get(), left, top, paint);
    }
}

void SkRecorder::onDrawBitmapRect(const SkBitmap& bitmap,
                                  const SkRect* src,
                                  const SkRect& dst,
                                  const SkPaint* paint,
                                  SrcRectConstraint constraint) {
    sk_sp<SkImage> image = SkImage::MakeFromBitmap(bitmap);
    if (image) {
        this->onDrawImageRect(image.get(), src, dst, paint, constraint);
    }
}

void SkRecorder::onDrawBitmapNine(const SkBitmap& bitmap,
                                  const SkIRect& center,
                                  const SkRect& dst,
                                  const SkPaint* paint) {
    sk_sp<SkImage> image = SkImage::MakeFromBitmap(bitmap);
    if (image) {
        this->onDrawImageNine(image.get(), center, dst, paint);
    }
}

void SkRecorder::onDrawBitmapLattice(const SkBitmap& bitmap, const Lattice& lattice,
                                     const SkRect& dst, const SkPaint* paint) {
    sk_sp<SkImage> image = SkImage::MakeFromBitmap(bitmap);
    this->onDrawImageLattice(image.get(), lattice, dst, paint);
}

void SkRecorder::onDrawImage(const SkImage* image, SkScalar left, SkScalar top,
                             const SkPaint* paint) {
    APPEND(DrawImage, this->copy(paint), sk_ref_sp(image), left, top);
}

void SkRecorder::onDrawImageRect(const SkImage* image, const SkRect* src, const SkRect& dst,
                                 const SkPaint* paint, SrcRectConstraint constraint) {
    APPEND(DrawImageRect, this->copy(paint), sk_ref_sp(image), this->copy(src), dst, constraint);
}

void SkRecorder::onDrawImageNine(const SkImage* image, const SkIRect& center,
                                 const SkRect& dst, const SkPaint* paint) {
    APPEND(DrawImageNine, this->copy(paint), sk_ref_sp(image), center, dst);
}

void SkRecorder::onDrawImageLattice(const SkImage* image, const Lattice& lattice, const SkRect& dst,
                                    const SkPaint* paint) {
    int flagCount = lattice.fFlags ? (lattice.fXCount + 1) * (lattice.fYCount + 1) : 0;
    SkASSERT(lattice.fBounds);
    APPEND(DrawImageLattice, this->copy(paint), sk_ref_sp(image),
           lattice.fXCount, this->copy(lattice.fXDivs, lattice.fXCount),
           lattice.fYCount, this->copy(lattice.fYDivs, lattice.fYCount),
           flagCount, this->copy(lattice.fFlags, flagCount), *lattice.fBounds, dst);
}

void SkRecorder::onDrawText(const void* text, size_t byteLength,
                            SkScalar x, SkScalar y, const SkPaint& paint) {
    APPEND(DrawText,
           paint, this->copy((const char*)text, byteLength), byteLength, x, y);
}

void SkRecorder::onDrawPosText(const void* text, size_t byteLength,
                               const SkPoint pos[], const SkPaint& paint) {
    const int points = paint.countText(text, byteLength);
    APPEND(DrawPosText,
           paint,
           this->copy((const char*)text, byteLength),
           byteLength,
           this->copy(pos, points));
}

void SkRecorder::onDrawPosTextH(const void* text, size_t byteLength,
                                const SkScalar xpos[], SkScalar constY, const SkPaint& paint) {
    const int points = paint.countText(text, byteLength);
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

void SkRecorder::onDrawTextRSXform(const void* text, size_t byteLength, const SkRSXform xform[],
                                   const SkRect* cull, const SkPaint& paint) {
    APPEND(DrawTextRSXform,
           paint,
           this->copy((const char*)text, byteLength),
           byteLength,
           this->copy(xform, paint.countText(text, byteLength)),
           this->copy(cull));
}

void SkRecorder::onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                                const SkPaint& paint) {
    TRY_MINIRECORDER(drawTextBlob, blob, x, y, paint);
    APPEND(DrawTextBlob, paint, sk_ref_sp(blob), x, y);
}

void SkRecorder::onDrawPicture(const SkPicture* pic, const SkMatrix* matrix, const SkPaint* paint) {
    if (fDrawPictureMode == Record_DrawPictureMode) {
        fApproxBytesUsedBySubPictures += pic->approximateBytesUsed();
        APPEND(DrawPicture, this->copy(paint), sk_ref_sp(pic), matrix ? *matrix : SkMatrix::I());
    } else {
        SkASSERT(fDrawPictureMode == Playback_DrawPictureMode);
        SkAutoCanvasMatrixPaint acmp(this, matrix, paint, pic->cullRect());
        pic->playback(this);
    }
}

void SkRecorder::onDrawShadowedPicture(const SkPicture* pic, const SkMatrix* matrix,
                                       const SkPaint* paint, const SkShadowParams& params) {
    if (fDrawPictureMode == Record_DrawPictureMode) {
        fApproxBytesUsedBySubPictures += pic->approximateBytesUsed();
        APPEND(DrawShadowedPicture, this->copy(paint),
                                    sk_ref_sp(pic),
                                    matrix ? *matrix : SkMatrix::I(),
                                    params);
    } else {
        // TODO update pic->playback(this) to draw the shadowed pic
        SkASSERT(fDrawPictureMode == Playback_DrawPictureMode);
        SkAutoCanvasMatrixPaint acmp(this,  matrix, paint, pic->cullRect());
        pic->playback(this);
    }
}


void SkRecorder::onDrawVerticesObject(const SkVertices* vertices, SkBlendMode bmode,
                                      const SkPaint& paint) {
    APPEND(DrawVertices, paint, sk_ref_sp(const_cast<SkVertices*>(vertices)), bmode);
}

void SkRecorder::onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                             const SkPoint texCoords[4], SkBlendMode bmode,
                             const SkPaint& paint) {
    APPEND(DrawPatch, paint,
           cubics ? this->copy(cubics, SkPatchUtils::kNumCtrlPts) : nullptr,
           colors ? this->copy(colors, SkPatchUtils::kNumCorners) : nullptr,
           texCoords ? this->copy(texCoords, SkPatchUtils::kNumCorners) : nullptr,
           bmode);
}

void SkRecorder::onDrawAtlas(const SkImage* atlas, const SkRSXform xform[], const SkRect tex[],
                             const SkColor colors[], int count, SkBlendMode mode,
                             const SkRect* cull, const SkPaint* paint) {
    APPEND(DrawAtlas, this->copy(paint),
           sk_ref_sp(atlas),
           this->copy(xform, count),
           this->copy(tex, count),
           this->copy(colors, count),
           count,
           mode,
           this->copy(cull));
}

void SkRecorder::onDrawAnnotation(const SkRect& rect, const char key[], SkData* value) {
    APPEND(DrawAnnotation, rect, SkString(key), sk_ref_sp(value));
}

void SkRecorder::willSave() {
    APPEND(Save);
}

SkCanvas::SaveLayerStrategy SkRecorder::getSaveLayerStrategy(const SaveLayerRec& rec) {
    APPEND(SaveLayer, this->copy(rec.fBounds)
                    , this->copy(rec.fPaint)
                    , sk_ref_sp(rec.fBackdrop)
                    , rec.fSaveLayerFlags);
    return SkCanvas::kNoLayer_SaveLayerStrategy;
}

void SkRecorder::didRestore() {
    APPEND(Restore, this->getDeviceClipBounds(), this->getTotalMatrix());
}

void SkRecorder::didConcat(const SkMatrix& matrix) {
    APPEND(Concat, matrix);
}

void SkRecorder::didSetMatrix(const SkMatrix& matrix) {
    APPEND(SetMatrix, matrix);
}

void SkRecorder::didTranslate(SkScalar dx, SkScalar dy) {
    APPEND(Translate, dx, dy);
}

void SkRecorder::didTranslateZ(SkScalar z) {
#ifdef SK_EXPERIMENTAL_SHADOWING
    APPEND(TranslateZ, z);
#endif
}

void SkRecorder::onClipRect(const SkRect& rect, SkClipOp op, ClipEdgeStyle edgeStyle) {
    INHERITED(onClipRect, rect, op, edgeStyle);
    SkRecords::ClipOpAndAA opAA(op, kSoft_ClipEdgeStyle == edgeStyle);
    APPEND(ClipRect, this->getDeviceClipBounds(), rect, opAA);
}

void SkRecorder::onClipRRect(const SkRRect& rrect, SkClipOp op, ClipEdgeStyle edgeStyle) {
    INHERITED(onClipRRect, rrect, op, edgeStyle);
    SkRecords::ClipOpAndAA opAA(op, kSoft_ClipEdgeStyle == edgeStyle);
    APPEND(ClipRRect, this->getDeviceClipBounds(), rrect, opAA);
}

void SkRecorder::onClipPath(const SkPath& path, SkClipOp op, ClipEdgeStyle edgeStyle) {
    INHERITED(onClipPath, path, op, edgeStyle);
    SkRecords::ClipOpAndAA opAA(op, kSoft_ClipEdgeStyle == edgeStyle);
    APPEND(ClipPath, this->getDeviceClipBounds(), path, opAA);
}

void SkRecorder::onClipRegion(const SkRegion& deviceRgn, SkClipOp op) {
    INHERITED(onClipRegion, deviceRgn, op);
    APPEND(ClipRegion, this->getDeviceClipBounds(), deviceRgn, op);
}

sk_sp<SkSurface> SkRecorder::onNewSurface(const SkImageInfo&, const SkSurfaceProps&) {
    return nullptr;
}
