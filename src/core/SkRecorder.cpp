/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRecorder.h"

#include "SkBigPicture.h"
#include "SkCanvasPriv.h"
#include "SkImage.h"
#include "SkPatchUtils.h"
#include "SkPicture.h"
#include "SkSurface.h"
#include "SkTo.h"

#include <new>

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
    : SkCanvasVirtualEnforcer<SkNoDrawCanvas>(width, height)
    , fDrawPictureMode(Record_DrawPictureMode)
    , fApproxBytesUsedBySubPictures(0)
    , fRecord(record)
    , fMiniRecorder(mr) {}

SkRecorder::SkRecorder(SkRecord* record, const SkRect& bounds, SkMiniRecorder* mr)
    : SkCanvasVirtualEnforcer<SkNoDrawCanvas>(bounds.roundOut())
    , fDrawPictureMode(Record_DrawPictureMode)
    , fApproxBytesUsedBySubPictures(0)
    , fRecord(record)
    , fMiniRecorder(mr) {}

void SkRecorder::reset(SkRecord* record, const SkRect& bounds,
                       DrawPictureMode dpm, SkMiniRecorder* mr) {
    this->forgetRecord();
    fDrawPictureMode = dpm;
    fRecord = record;
    SkIRect rounded = bounds.roundOut();
    this->resetCanvas(rounded.right(), rounded.bottom());
    fMiniRecorder = mr;
}

void SkRecorder::forgetRecord() {
    fDrawableList.reset(nullptr);
    fApproxBytesUsedBySubPictures = 0;
    fRecord = nullptr;
}

// To make appending to fRecord a little less verbose.
template<typename T, typename... Args>
void SkRecorder::append(Args&&... args) {
    if (fMiniRecorder) {
        this->flushMiniRecorder();
    }
    new (fRecord->append<T>()) T{std::forward<Args>(args)...};
}

#define TRY_MINIRECORDER(method, ...) \
    if (fMiniRecorder && fMiniRecorder->method(__VA_ARGS__)) return

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
    this->append<SkRecords::DrawPaint>(paint);
}

void SkRecorder::onDrawPoints(PointMode mode,
                              size_t count,
                              const SkPoint pts[],
                              const SkPaint& paint) {
    this->append<SkRecords::DrawPoints>(paint, mode, SkToUInt(count), this->copy(pts, count));
}

void SkRecorder::onDrawRect(const SkRect& rect, const SkPaint& paint) {
    TRY_MINIRECORDER(drawRect, rect, paint);
    this->append<SkRecords::DrawRect>(paint, rect);
}

void SkRecorder::onDrawEdgeAARect(const SkRect& rect, SkCanvas::QuadAAFlags aa, SkColor color,
                                  SkBlendMode mode) {
    this->append<SkRecords::DrawEdgeAARect>(rect, aa, color, mode);
}

void SkRecorder::onDrawRegion(const SkRegion& region, const SkPaint& paint) {
    this->append<SkRecords::DrawRegion>(paint, region);
}

void SkRecorder::onDrawOval(const SkRect& oval, const SkPaint& paint) {
    this->append<SkRecords::DrawOval>(paint, oval);
}

void SkRecorder::onDrawArc(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle,
                           bool useCenter, const SkPaint& paint) {
    this->append<SkRecords::DrawArc>(paint, oval, startAngle, sweepAngle, useCenter);
}

void SkRecorder::onDrawRRect(const SkRRect& rrect, const SkPaint& paint) {
    this->append<SkRecords::DrawRRect>(paint, rrect);
}

void SkRecorder::onDrawDRRect(const SkRRect& outer, const SkRRect& inner, const SkPaint& paint) {
    this->append<SkRecords::DrawDRRect>(paint, outer, inner);
}

void SkRecorder::onDrawDrawable(SkDrawable* drawable, const SkMatrix* matrix) {
    if (fDrawPictureMode == Record_DrawPictureMode) {
        if (!fDrawableList) {
            fDrawableList.reset(new SkDrawableList);
        }
        fDrawableList->append(drawable);
        this->append<SkRecords::DrawDrawable>(this->copy(matrix), drawable->getBounds(), fDrawableList->count() - 1);
    } else {
        SkASSERT(fDrawPictureMode == Playback_DrawPictureMode);
        drawable->draw(this, matrix);
    }
}

void SkRecorder::onDrawPath(const SkPath& path, const SkPaint& paint) {
    TRY_MINIRECORDER(drawPath, path, paint);
    this->append<SkRecords::DrawPath>(paint, path);
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
    this->append<SkRecords::DrawImage>(this->copy(paint), sk_ref_sp(image), left, top);
}

void SkRecorder::onDrawImageRect(const SkImage* image, const SkRect* src, const SkRect& dst,
                                 const SkPaint* paint, SrcRectConstraint constraint) {
    this->append<SkRecords::DrawImageRect>(this->copy(paint), sk_ref_sp(image), this->copy(src), dst, constraint);
}

void SkRecorder::onDrawImageNine(const SkImage* image, const SkIRect& center,
                                 const SkRect& dst, const SkPaint* paint) {
    this->append<SkRecords::DrawImageNine>(this->copy(paint), sk_ref_sp(image), center, dst);
}

void SkRecorder::onDrawImageLattice(const SkImage* image, const Lattice& lattice, const SkRect& dst,
                                    const SkPaint* paint) {
    int flagCount = lattice.fRectTypes ? (lattice.fXCount + 1) * (lattice.fYCount + 1) : 0;
    SkASSERT(lattice.fBounds);
    this->append<SkRecords::DrawImageLattice>(this->copy(paint), sk_ref_sp(image),
           lattice.fXCount, this->copy(lattice.fXDivs, lattice.fXCount),
           lattice.fYCount, this->copy(lattice.fYDivs, lattice.fYCount),
           flagCount, this->copy(lattice.fRectTypes, flagCount),
           this->copy(lattice.fColors, flagCount), *lattice.fBounds, dst);
}

void SkRecorder::onDrawImageSet(const ImageSetEntry set[], int count, SkFilterQuality filterQuality,
                                SkBlendMode mode) {
    SkAutoTArray<ImageSetEntry> setCopy(count);
    for (int i = 0; i < count; ++i) {
        setCopy[i] = set[i];
    }
    this->append<SkRecords::DrawImageSet>(std::move(setCopy), count, filterQuality, mode);
}

void SkRecorder::onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                                const SkPaint& paint) {
    TRY_MINIRECORDER(drawTextBlob, blob, x, y, paint);
    this->append<SkRecords::DrawTextBlob>(paint, sk_ref_sp(blob), x, y);
}

void SkRecorder::onDrawPicture(const SkPicture* pic, const SkMatrix* matrix, const SkPaint* paint) {
    if (fDrawPictureMode == Record_DrawPictureMode) {
        fApproxBytesUsedBySubPictures += pic->approximateBytesUsed();
        this->append<SkRecords::DrawPicture>(this->copy(paint), sk_ref_sp(pic), matrix ? *matrix : SkMatrix::I());
    } else {
        SkASSERT(fDrawPictureMode == Playback_DrawPictureMode);
        SkAutoCanvasMatrixPaint acmp(this, matrix, paint, pic->cullRect());
        pic->playback(this);
    }
}

void SkRecorder::onDrawVerticesObject(const SkVertices* vertices, const SkVertices::Bone bones[],
                                      int boneCount, SkBlendMode bmode, const SkPaint& paint) {
    this->append<SkRecords::DrawVertices>(paint,
                                          sk_ref_sp(const_cast<SkVertices*>(vertices)),
                                          this->copy(bones, boneCount),
                                          boneCount,
                                          bmode);
}

void SkRecorder::onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                             const SkPoint texCoords[4], SkBlendMode bmode,
                             const SkPaint& paint) {
    this->append<SkRecords::DrawPatch>(paint,
           cubics ? this->copy(cubics, SkPatchUtils::kNumCtrlPts) : nullptr,
           colors ? this->copy(colors, SkPatchUtils::kNumCorners) : nullptr,
           texCoords ? this->copy(texCoords, SkPatchUtils::kNumCorners) : nullptr,
           bmode);
}

void SkRecorder::onDrawAtlas(const SkImage* atlas, const SkRSXform xform[], const SkRect tex[],
                             const SkColor colors[], int count, SkBlendMode mode,
                             const SkRect* cull, const SkPaint* paint) {
    this->append<SkRecords::DrawAtlas>(this->copy(paint),
           sk_ref_sp(atlas),
           this->copy(xform, count),
           this->copy(tex, count),
           this->copy(colors, count),
           count,
           mode,
           this->copy(cull));
}

void SkRecorder::onDrawShadowRec(const SkPath& path, const SkDrawShadowRec& rec) {
    this->append<SkRecords::DrawShadowRec>(path, rec);
}

void SkRecorder::onDrawAnnotation(const SkRect& rect, const char key[], SkData* value) {
    this->append<SkRecords::DrawAnnotation>(rect, SkString(key), sk_ref_sp(value));
}

void SkRecorder::onFlush() {
    this->append<SkRecords::Flush>();
}

void SkRecorder::willSave() {
    this->append<SkRecords::Save>();
}

SkCanvas::SaveLayerStrategy SkRecorder::getSaveLayerStrategy(const SaveLayerRec& rec) {
    this->append<SkRecords::SaveLayer>(this->copy(rec.fBounds)
                    , this->copy(rec.fPaint)
                    , sk_ref_sp(rec.fBackdrop)
                    , sk_ref_sp(rec.fClipMask)
                    , this->copy(rec.fClipMatrix)
                    , rec.fSaveLayerFlags);
    return SkCanvas::kNoLayer_SaveLayerStrategy;
}

bool SkRecorder::onDoSaveBehind(const SkRect* subset) {
    this->append<SkRecords::SaveBehind>(this->copy(subset));
    return false;
}

void SkRecorder::didRestore() {
    this->append<SkRecords::Restore>(this->getTotalMatrix());
}

void SkRecorder::didConcat(const SkMatrix& matrix) {
    this->append<SkRecords::Concat>(matrix);
}

void SkRecorder::didSetMatrix(const SkMatrix& matrix) {
    this->append<SkRecords::SetMatrix>(matrix);
}

void SkRecorder::didTranslate(SkScalar dx, SkScalar dy) {
    this->append<SkRecords::Translate>(dx, dy);
}

void SkRecorder::onClipRect(const SkRect& rect, SkClipOp op, ClipEdgeStyle edgeStyle) {
    INHERITED(onClipRect, rect, op, edgeStyle);
    SkRecords::ClipOpAndAA opAA(op, kSoft_ClipEdgeStyle == edgeStyle);
    this->append<SkRecords::ClipRect>(rect, opAA);
}

void SkRecorder::onClipRRect(const SkRRect& rrect, SkClipOp op, ClipEdgeStyle edgeStyle) {
    INHERITED(onClipRRect, rrect, op, edgeStyle);
    SkRecords::ClipOpAndAA opAA(op, kSoft_ClipEdgeStyle == edgeStyle);
    this->append<SkRecords::ClipRRect>(rrect, opAA);
}

void SkRecorder::onClipPath(const SkPath& path, SkClipOp op, ClipEdgeStyle edgeStyle) {
    INHERITED(onClipPath, path, op, edgeStyle);
    SkRecords::ClipOpAndAA opAA(op, kSoft_ClipEdgeStyle == edgeStyle);
    this->append<SkRecords::ClipPath>(path, opAA);
}

void SkRecorder::onClipRegion(const SkRegion& deviceRgn, SkClipOp op) {
    INHERITED(onClipRegion, deviceRgn, op);
    this->append<SkRecords::ClipRegion>(deviceRgn, op);
}

sk_sp<SkSurface> SkRecorder::onNewSurface(const SkImageInfo&, const SkSurfaceProps&) {
    return nullptr;
}
