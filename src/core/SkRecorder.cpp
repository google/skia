/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkRecorder.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRSXform.h"
#include "include/core/SkRect.h"
#include "include/core/SkShader.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkVertices.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"
#include "include/private/chromium/Slug.h"
#include "src/core/SkBigPicture.h"
#include "src/core/SkCanvasPriv.h"
#include "src/core/SkRecord.h"
#include "src/core/SkRecords.h"
#include "src/text/GlyphRun.h"
#include "src/utils/SkPatchUtils.h"

#include <cstdint>
#include <cstring>
#include <memory>
#include <new>

class SkBlender;
class SkMesh;
class SkPath;
class SkRRect;
class SkRegion;
class SkSurfaceProps;
enum class SkBlendMode;
enum class SkClipOp;
struct SkDrawShadowRec;

using namespace skia_private;

SkDrawableList::~SkDrawableList() {
    for(SkDrawable* p : fArray) {
        p->unref();
    }
    fArray.reset();
}

SkBigPicture::SnapshotArray* SkDrawableList::newDrawableSnapshot() {
    const int count = fArray.size();
    if (0 == count) {
        return nullptr;
    }
    AutoTMalloc<const SkPicture*> pics(count);
    for (int i = 0; i < count; ++i) {
        pics[i] = fArray[i]->makePictureSnapshot().release();
    }
    return new SkBigPicture::SnapshotArray(pics.release(), count);
}

void SkDrawableList::append(SkDrawable* drawable) {
    *fArray.append() = SkRef(drawable);
}

///////////////////////////////////////////////////////////////////////////////////////////////

static SkIRect safe_picture_bounds(const SkRect& bounds) {
    SkIRect picBounds = bounds.roundOut();
    // roundOut() saturates the float edges to +/-SK_MaxS32FitsInFloat (~2billion), but this is
    // large enough that width/height calculations will overflow, leading to negative dimensions.
    static constexpr int32_t kSafeEdge = SK_MaxS32FitsInFloat / 2 - 1;
    static constexpr SkIRect kSafeBounds = {-kSafeEdge, -kSafeEdge, kSafeEdge, kSafeEdge};
    static_assert((kSafeBounds.fRight - kSafeBounds.fLeft) >= 0 &&
                  (kSafeBounds.fBottom - kSafeBounds.fTop) >= 0);
    if (!picBounds.intersect(kSafeBounds)) {
        picBounds.setEmpty();
    }
    return picBounds;
}

SkRecorder::SkRecorder(SkRecord* record, int width, int height)
        : SkCanvasVirtualEnforcer<SkNoDrawCanvas>(width, height)
        , fApproxBytesUsedBySubPictures(0)
        , fRecord(record) {
    SkASSERT(this->imageInfo().width() >= 0 && this->imageInfo().height() >= 0);
}

SkRecorder::SkRecorder(SkRecord* record, const SkRect& bounds)
        : SkCanvasVirtualEnforcer<SkNoDrawCanvas>(safe_picture_bounds(bounds))
        , fApproxBytesUsedBySubPictures(0)
        , fRecord(record) {
    SkASSERT(this->imageInfo().width() >= 0 && this->imageInfo().height() >= 0);
}

void SkRecorder::reset(SkRecord* record, const SkRect& bounds) {
    this->forgetRecord();
    fRecord = record;
    this->resetCanvas(safe_picture_bounds(bounds));
    SkASSERT(this->imageInfo().width() >= 0 && this->imageInfo().height() >= 0);
}

void SkRecorder::forgetRecord() {
    fDrawableList.reset(nullptr);
    fApproxBytesUsedBySubPictures = 0;
    fRecord = nullptr;
}

// To make appending to fRecord a little less verbose.
template<typename T, typename... Args>
void SkRecorder::append(Args&&... args) {
    new (fRecord->append<T>()) T{std::forward<Args>(args)...};
}

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

void SkRecorder::onDrawPaint(const SkPaint& paint) {
    this->append<SkRecords::DrawPaint>(paint);
}

void SkRecorder::onDrawBehind(const SkPaint& paint) {
    this->append<SkRecords::DrawBehind>(paint);
}

void SkRecorder::onDrawPoints(PointMode mode,
                              size_t count,
                              const SkPoint pts[],
                              const SkPaint& paint) {
    this->append<SkRecords::DrawPoints>(paint, mode, SkToUInt(count), this->copy(pts, count));
}

void SkRecorder::onDrawRect(const SkRect& rect, const SkPaint& paint) {
    this->append<SkRecords::DrawRect>(paint, rect);
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
    if (!fDrawableList) {
        fDrawableList = std::make_unique<SkDrawableList>();
    }
    fDrawableList->append(drawable);
    this->append<SkRecords::DrawDrawable>(this->copy(matrix), drawable->getBounds(), fDrawableList->count() - 1);
}

void SkRecorder::onDrawPath(const SkPath& path, const SkPaint& paint) {
    this->append<SkRecords::DrawPath>(paint, path);
}

void SkRecorder::onDrawImage2(const SkImage* image, SkScalar x, SkScalar y,
                              const SkSamplingOptions& sampling, const SkPaint* paint) {
    this->append<SkRecords::DrawImage>(this->copy(paint), sk_ref_sp(image), x, y, sampling);
}

void SkRecorder::onDrawImageRect2(const SkImage* image, const SkRect& src, const SkRect& dst,
                                  const SkSamplingOptions& sampling, const SkPaint* paint,
                                  SrcRectConstraint constraint) {
    this->append<SkRecords::DrawImageRect>(this->copy(paint), sk_ref_sp(image), src, dst,
                                           sampling, constraint);
}

void SkRecorder::onDrawImageLattice2(const SkImage* image, const Lattice& lattice, const SkRect& dst,
                                     SkFilterMode filter, const SkPaint* paint) {
    int flagCount = lattice.fRectTypes ? (lattice.fXCount + 1) * (lattice.fYCount + 1) : 0;
    SkASSERT(lattice.fBounds);
    this->append<SkRecords::DrawImageLattice>(this->copy(paint), sk_ref_sp(image),
           lattice.fXCount, this->copy(lattice.fXDivs, lattice.fXCount),
           lattice.fYCount, this->copy(lattice.fYDivs, lattice.fYCount),
           flagCount, this->copy(lattice.fRectTypes, flagCount),
           this->copy(lattice.fColors, flagCount), *lattice.fBounds, dst, filter);
}

void SkRecorder::onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                                const SkPaint& paint) {
    this->append<SkRecords::DrawTextBlob>(paint, sk_ref_sp(blob), x, y);
}

void SkRecorder::onDrawSlug(const sktext::gpu::Slug* slug) {
    this->append<SkRecords::DrawSlug>(sk_ref_sp(slug));
}

void SkRecorder::onDrawGlyphRunList(
        const sktext::GlyphRunList& glyphRunList, const SkPaint& paint) {
    sk_sp<SkTextBlob> blob = sk_ref_sp(glyphRunList.blob());
    if (glyphRunList.blob() == nullptr) {
        blob = glyphRunList.makeBlob();
    }

    this->onDrawTextBlob(blob.get(), glyphRunList.origin().x(), glyphRunList.origin().y(), paint);
}

void SkRecorder::onDrawPicture(const SkPicture* pic, const SkMatrix* matrix, const SkPaint* paint) {
    fApproxBytesUsedBySubPictures += pic->approximateBytesUsed();
    this->append<SkRecords::DrawPicture>(this->copy(paint), sk_ref_sp(pic), matrix ? *matrix : SkMatrix::I());
}

void SkRecorder::onDrawVerticesObject(const SkVertices* vertices, SkBlendMode bmode,
                                      const SkPaint& paint) {
    this->append<SkRecords::DrawVertices>(paint,
                                          sk_ref_sp(const_cast<SkVertices*>(vertices)),
                                          bmode);
}

void SkRecorder::onDrawMesh(const SkMesh& mesh, sk_sp<SkBlender> blender, const SkPaint& paint) {
    this->append<SkRecords::DrawMesh>(paint, mesh, std::move(blender));
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

void SkRecorder::onDrawAtlas2(const SkImage* atlas, const SkRSXform xform[], const SkRect tex[],
                              const SkColor colors[], int count, SkBlendMode mode,
                              const SkSamplingOptions& sampling, const SkRect* cull,
                              const SkPaint* paint) {
    this->append<SkRecords::DrawAtlas>(this->copy(paint),
           sk_ref_sp(atlas),
           this->copy(xform, count),
           this->copy(tex, count),
           this->copy(colors, count),
           count,
           mode,
           sampling,
           this->copy(cull));
}

void SkRecorder::onDrawShadowRec(const SkPath& path, const SkDrawShadowRec& rec) {
    this->append<SkRecords::DrawShadowRec>(path, rec);
}

void SkRecorder::onDrawAnnotation(const SkRect& rect, const char key[], SkData* value) {
    this->append<SkRecords::DrawAnnotation>(rect, SkString(key), sk_ref_sp(value));
}

void SkRecorder::onDrawEdgeAAQuad(const SkRect& rect, const SkPoint clip[4],
                                  QuadAAFlags aa, const SkColor4f& color, SkBlendMode mode) {
    this->append<SkRecords::DrawEdgeAAQuad>(
            rect, this->copy(clip, 4), aa, color, mode);
}

void SkRecorder::onDrawEdgeAAImageSet2(const ImageSetEntry set[], int count,
                                       const SkPoint dstClips[], const SkMatrix preViewMatrices[],
                                       const SkSamplingOptions& sampling, const SkPaint* paint,
                                       SrcRectConstraint constraint) {
    int totalDstClipCount, totalMatrixCount;
    SkCanvasPriv::GetDstClipAndMatrixCounts(set, count, &totalDstClipCount, &totalMatrixCount);

    AutoTArray<ImageSetEntry> setCopy(count);
    for (int i = 0; i < count; ++i) {
        setCopy[i] = set[i];
    }

    this->append<SkRecords::DrawEdgeAAImageSet>(this->copy(paint), std::move(setCopy), count,
            this->copy(dstClips, totalDstClipCount),
            this->copy(preViewMatrices, totalMatrixCount), sampling, constraint);
}

void SkRecorder::willSave() {
    this->append<SkRecords::Save>();
}

SkCanvas::SaveLayerStrategy SkRecorder::getSaveLayerStrategy(const SaveLayerRec& rec) {
    this->append<SkRecords::SaveLayer>(this->copy(rec.fBounds)
                    , this->copy(rec.fPaint)
                    , sk_ref_sp(rec.fBackdrop)
                    , rec.fSaveLayerFlags
                    , SkCanvasPriv::GetBackdropScaleFactor(rec));
    return SkCanvas::kNoLayer_SaveLayerStrategy;
}

bool SkRecorder::onDoSaveBehind(const SkRect* subset) {
    this->append<SkRecords::SaveBehind>(this->copy(subset));
    return false;
}

void SkRecorder::didRestore() {
    this->append<SkRecords::Restore>(this->getTotalMatrix());
}

void SkRecorder::didConcat44(const SkM44& m) {
    this->append<SkRecords::Concat44>(m);
}

void SkRecorder::didSetM44(const SkM44& m) {
    this->append<SkRecords::SetM44>(m);
}

void SkRecorder::didScale(SkScalar sx, SkScalar sy) {
    this->append<SkRecords::Scale>(sx, sy);
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

void SkRecorder::onClipShader(sk_sp<SkShader> cs, SkClipOp op) {
    INHERITED(onClipShader, cs, op);
    this->append<SkRecords::ClipShader>(std::move(cs), op);
}

void SkRecorder::onClipRegion(const SkRegion& deviceRgn, SkClipOp op) {
    INHERITED(onClipRegion, deviceRgn, op);
    this->append<SkRecords::ClipRegion>(deviceRgn, op);
}

void SkRecorder::onResetClip() {
    INHERITED(onResetClip);
    this->append<SkRecords::ResetClip>();
}

sk_sp<SkSurface> SkRecorder::onNewSurface(const SkImageInfo&, const SkSurfaceProps&) {
    return nullptr;
}
