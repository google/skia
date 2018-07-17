/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkThreadedBMPDevice.h"

#include "SkPath.h"
#include "SkSpecialImage.h"
#include "SkTaskGroup.h"
#include "SkVertices.h"

// Calling init(j, k) would initialize the j-th element on k-th thread. It returns false if it's
// already initiailized.
bool SkThreadedBMPDevice::DrawQueue::initColumn(int column, int thread) {
    return fElements[column].tryInitOnce(&fThreadAllocs[thread]);
}

// Calling work(i, j, k) would draw j-th element the i-th tile on k-th thead. If the element still
// needs to be initialized, drawFn will return false without drawing.
bool SkThreadedBMPDevice::DrawQueue::work2D(int row, int column, int thread) {
    return fElements[column].tryDraw(fDevice->fTileBounds[row], &fThreadAllocs[thread]);
}

void SkThreadedBMPDevice::DrawQueue::reset() {
    if (fTasks) {
        fTasks->finish();
    }

    fThreadAllocs.reset(fDevice->fThreadCnt);
    fSize = 0;

    // using TaskGroup2D = SkSpinningTaskGroup2D;
    using TaskGroup2D = SkFlexibleTaskGroup2D;

    fTasks.reset(new TaskGroup2D(this, fDevice->fTileCnt, fDevice->fExecutor,
                                 fDevice->fThreadCnt));
    fTasks->start();
}

SkThreadedBMPDevice::SkThreadedBMPDevice(const SkBitmap& bitmap,
                                         int tiles,
                                         int threads,
                                         SkExecutor* executor)
        : INHERITED(bitmap)
        , fTileCnt(tiles)
        , fThreadCnt(threads <= 0 ? tiles : threads)
        , fQueue(this)
{
    if (executor == nullptr) {
        fInternalExecutor = SkExecutor::MakeFIFOThreadPool(fThreadCnt);
        executor = fInternalExecutor.get();
    }
    fExecutor = executor;

    // Tiling using stripes for now; we'll explore better tiling in the future.
    int h = (bitmap.height() + fTileCnt - 1) / SkTMax(fTileCnt, 1);
    int w = bitmap.width();
    int top = 0;
    for(int tid = 0; tid < fTileCnt; ++tid, top += h) {
        fTileBounds.push_back(SkIRect::MakeLTRB(0, top, w, top + h));
    }
    fQueue.reset();
}

void SkThreadedBMPDevice::flush() {
    fQueue.reset();
    fAlloc.reset();
}

SkThreadedBMPDevice::DrawState::DrawState(SkThreadedBMPDevice* dev) {
    // we need fDst to be set, and if we're actually drawing, to dirty the genID
    if (!dev->accessPixels(&fDst)) {
        // NoDrawDevice uses us (why?) so we have to catch this case w/ no pixels
        fDst.reset(dev->imageInfo(), nullptr, 0);
    }
    fMatrix = dev->ctm();
    fMatrix.getType(); // make it thread safe
    fRC = dev->fRCStack.rc();
}

SkDraw SkThreadedBMPDevice::DrawState::getDraw() const {
    SkDraw draw;
    draw.fDst = fDst;
    draw.fMatrix = &fMatrix;
    draw.fRC = &fRC;
    return draw;
}

SkThreadedBMPDevice::TileDraw::TileDraw(const DrawState& ds, const SkIRect& tileBounds)
        : fTileRC(ds.fRC) {
    fDst = ds.fDst;
    fMatrix = &ds.fMatrix;
    fTileRC.op(tileBounds, SkRegion::kIntersect_Op);
    fRC = &fTileRC;
}

static inline SkRect get_fast_bounds(const SkRect& r, const SkPaint& p) {
    SkRect result;
    if (p.canComputeFastBounds()) {
        result = p.computeFastBounds(r, &result);
    } else {
        result = SkRectPriv::MakeLargest();
    }
    return result;
}

void SkThreadedBMPDevice::drawPaint(const SkPaint& paint) {
    SkRect drawBounds = SkRectPriv::MakeLargest();
    fQueue.push(drawBounds, [=](SkArenaAlloc*, const DrawState& ds, const SkIRect& tileBounds){
        TileDraw(ds, tileBounds).drawPaint(paint);
    });
}

void SkThreadedBMPDevice::drawPoints(SkCanvas::PointMode mode, size_t count,
        const SkPoint pts[], const SkPaint& paint) {
    SkPoint* clonedPts = this->cloneArray(pts, count);
    SkRect drawBounds = SkRectPriv::MakeLargest(); // TODO tighter drawBounds
    fQueue.push(drawBounds, [=](SkArenaAlloc*, const DrawState& ds, const SkIRect& tileBounds){
        TileDraw(ds, tileBounds).drawPoints(mode, count, clonedPts, paint, nullptr);
    });
}

void SkThreadedBMPDevice::drawRect(const SkRect& r, const SkPaint& paint) {
    SkRect drawBounds = get_fast_bounds(r, paint);
    fQueue.push(drawBounds, [=](SkArenaAlloc*, const DrawState& ds, const SkIRect& tileBounds){
        TileDraw(ds, tileBounds).drawRect(r, paint);
    });
}

void SkThreadedBMPDevice::drawRRect(const SkRRect& rrect, const SkPaint& paint) {
#ifdef SK_IGNORE_BLURRED_RRECT_OPT
    SkPath  path;

    path.addRRect(rrect);
    // call the VIRTUAL version, so any subclasses who do handle drawPath aren't
    // required to override drawRRect.
    this->drawPath(path, paint, nullptr, false);
#else
    SkRect drawBounds = get_fast_bounds(rrect.getBounds(), paint);
    fQueue.push(drawBounds, [=](SkArenaAlloc*, const DrawState& ds, const SkIRect& tileBounds){
        TileDraw(ds, tileBounds).drawRRect(rrect, paint);
    });
#endif
}

void SkThreadedBMPDevice::drawPath(const SkPath& path, const SkPaint& paint,
        const SkMatrix* prePathMatrix, bool pathIsMutable) {
    SkRect drawBounds = path.isInverseFillType() ? SkRectPriv::MakeLargest()
                                                 : get_fast_bounds(path.getBounds(), paint);
    // when path is small, init-once has too much overhead; init-once also can't handle mask filter
    if (path.countVerbs() < 4 || paint.getMaskFilter()) {
        fQueue.push(drawBounds, [=](SkArenaAlloc*, const DrawState& ds, const SkIRect& tileBounds) {
            TileDraw(ds, tileBounds).drawPath(path, paint, prePathMatrix, false);
        });
    } else {
        fQueue.push(drawBounds, [=](SkArenaAlloc* alloc, DrawElement* elem) {
            SkInitOnceData data = {alloc, elem};
            elem->getDraw().drawPath(path, paint, prePathMatrix, false, false, nullptr, &data);
        });
    }
}

SkBitmap SkThreadedBMPDevice::snapBitmap(const SkBitmap& bitmap) {
    // We can't use bitmap.isImmutable() because it could be temporarily immutable
    // TODO(liyuqian): use genID to reduce the copy frequency
    SkBitmap snap;
    snap.allocPixels(bitmap.info());
    bitmap.readPixels(snap.pixmap());
    return snap;
}

void SkThreadedBMPDevice::drawBitmap(const SkBitmap& bitmap, const SkMatrix& matrix,
                                     const SkRect* dstOrNull, const SkPaint& paint) {
    SkRect drawBounds;
    SkRect* clonedDstOrNull = nullptr;
    if (dstOrNull == nullptr) {
        drawBounds = SkRect::MakeWH(bitmap.width(), bitmap.height());
        matrix.mapRect(&drawBounds);
    } else {
        drawBounds = *dstOrNull;
        clonedDstOrNull = fAlloc.make<SkRect>(*dstOrNull);
    }

    SkBitmap snap = this->snapBitmap(bitmap);
    fQueue.push(drawBounds, [=](SkArenaAlloc*, const DrawState& ds, const SkIRect& tileBounds){
        SkBitmap local = snap; // bitmap is not thread safe; copy a local one.
        TileDraw(ds, tileBounds).drawBitmap(local, matrix, clonedDstOrNull, paint);
    });
}

void SkThreadedBMPDevice::drawBitmapRect(const SkBitmap& bitmap, const SkRect* src,
        const SkRect& dst, const SkPaint& paint, SkCanvas::SrcRectConstraint constraint) {
    // SkBitmapDevice::drawBitmapRect may use shader and drawRect. In that case, we need to snap
    // the bitmap here because we won't go into SkThreadedBMPDevice::drawBitmap.
    SkBitmap snap = this->snapBitmap(bitmap);
    this->SkBitmapDevice::drawBitmapRect(snap, src, dst, paint, constraint);
}


void SkThreadedBMPDevice::drawSprite(const SkBitmap& bitmap, int x, int y, const SkPaint& paint) {
    SkRect drawBounds = SkRect::MakeXYWH(x, y, bitmap.width(), bitmap.height());
    SkBitmap snap = this->snapBitmap(bitmap);
    fQueue.push<false>(drawBounds, [=](SkArenaAlloc*, const DrawState& ds,
                                       const SkIRect& tileBounds){
        SkBitmap local = snap; // bitmap is not thread safe; copy a local one.
        TileDraw(ds, tileBounds).drawSprite(local, x, y, paint);
    });
}

void SkThreadedBMPDevice::drawPosText(const void* text, size_t len, const SkScalar xpos[],
        int scalarsPerPos, const SkPoint& offset, const SkPaint& paint) {
    char* clonedText = this->cloneArray((const char*)text, len);
    SkScalar* clonedXpos = this->cloneArray(xpos, paint.countText(text, len) * scalarsPerPos);
    SkRect drawBounds = SkRectPriv::MakeLargest(); // TODO tighter drawBounds
    SkSurfaceProps prop(SkBitmapDeviceFilteredSurfaceProps(fBitmap, paint, this->surfaceProps())());
    fQueue.push(drawBounds, [=](SkArenaAlloc*, const DrawState& ds, const SkIRect& tileBounds){
        TileDraw(ds, tileBounds).drawPosText(clonedText, len, clonedXpos, scalarsPerPos, offset,
                                             paint, &prop);
    });
}

void SkThreadedBMPDevice::drawVertices(const SkVertices* vertices, const SkMatrix* bones,
                                       int boneCount, SkBlendMode bmode, const SkPaint& paint) {
    const sk_sp<SkVertices> verts = sk_ref_sp(vertices);  // retain vertices until flush
    SkRect drawBounds = SkRectPriv::MakeLargest(); // TODO tighter drawBounds

    // Make a copy of the bone matrices.
    SkMatrix* clonedBones = bones ? this->cloneArray(bones, boneCount) : nullptr;

    // Make the bone matrices thread-safe.
    for (int i = 0; i < boneCount; i ++) {
        clonedBones[i].getType();
    }

    fQueue.push(drawBounds, [=](SkArenaAlloc*, const DrawState& ds, const SkIRect& tileBounds){
        TileDraw(ds, tileBounds).drawVertices(verts->mode(), verts->vertexCount(),
                                              verts->positions(), verts->texCoords(),
                                              verts->colors(), verts->boneIndices(),
                                              verts->boneWeights(), bmode, verts->indices(),
                                              verts->indexCount(), paint, clonedBones, boneCount);
    });
}

sk_sp<SkSpecialImage> SkThreadedBMPDevice::snapSpecial() {
    this->flush();
    return this->makeSpecial(fBitmap);
}
