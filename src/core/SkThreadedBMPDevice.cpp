/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkThreadedBMPDevice.h"

#include "SkPath.h"
#include "SkTaskGroup.h"
#include "SkVertices.h"

void SkThreadedBMPDevice::DrawQueue::reset() {
    if (fTasks) {
        fTasks->finish();
    }

    fThreadAllocs.reset(fDevice->fThreadCnt);
    fSize = 0;

    // using TaskGroup2D = SkSpinningTaskGroup2D;
    using TaskGroup2D = SkFlexibleTaskGroup2D;

    // Calling drawFn(i, j, k) would draw j-th element the i-th tile on k-th thead.
    // If the element still needs to be initialized, drawFn will return false without drawing.
    auto drawFn = [this](int row, int column, int threadId){
        SkThreadedBMPDevice::DrawElement& element = fElements[column];
        if (!SkIRect::Intersects(fDevice->fTileBounds[row], element.fDrawBounds)) {
            return true;
        }
        if (element.fInitialized) {
            element.fDrawFn(&fThreadAllocs[threadId], element.fDS, fDevice->fTileBounds[row]);
            return true;
        }
        return false;
    };

    // Calling initFn(j, k) would initialize the j-th element on k-th thread. It returns false if no
    // initialization is done either because the element is already initialized, or because some
    // other thread is initializing it.
    auto initFn = [this](int column, int threadId){
        SkThreadedBMPDevice::DrawElement& element = fElements[column];
        // element.fInitialized and element.fNeedInit are two atomic booleans that makes sure that
        // each element is only initialized once. They are initialized to (false, true) for elements
        // that need initialization. Once initialized, fInitialized will be set to true. During the
        // initialization, they'll be (false, false).
        if (!element.fInitialized && element.fNeedInit) {
            bool needInit = true;
            // If there are multiple threads reaching this point simutaneously,
            // compare_exchange_strong ensures that only one thread can enter the if condition and
            // do the initialization.
            if (element.fNeedInit.compare_exchange_strong(needInit, false)) {
                element.fInitFn(&fThreadAllocs[threadId], &element);
                element.fInitialized = true;
                return true;
            }
        }
        return false;
    };

    fTasks.reset(new TaskGroup2D(drawFn, initFn, fDevice->fTileCnt, fDevice->fExecutor,
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
}

SkThreadedBMPDevice::DrawState::DrawState(SkThreadedBMPDevice* dev) {
    // we need fDst to be set, and if we're actually drawing, to dirty the genID
    if (!dev->accessPixels(&fDst)) {
        // NoDrawDevice uses us (why?) so we have to catch this case w/ no pixels
        fDst.reset(dev->imageInfo(), nullptr, 0);
    }
    fMatrix = dev->ctm();
    fRC = dev->fRCStack.rc();
}

SkIRect SkThreadedBMPDevice::transformDrawBounds(const SkRect& drawBounds) const {
    if (drawBounds.isLargest()) {
        return SkIRect::MakeLargest();
    }
    SkRect transformedBounds;
    this->ctm().mapRect(&transformedBounds, drawBounds);
    return transformedBounds.roundOut();
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
        result = SkRect::MakeLargest();
    }
    return result;
}

void SkThreadedBMPDevice::drawPaint(const SkPaint& paint) {
    SkRect drawBounds = SkRect::MakeLargest();
    fQueue.push(drawBounds, [=](SkArenaAlloc*, const DrawState& ds, const SkIRect& tileBounds){
        TileDraw(ds, tileBounds).drawPaint(paint);
    });
}

void SkThreadedBMPDevice::drawPoints(SkCanvas::PointMode mode, size_t count,
        const SkPoint pts[], const SkPaint& paint) {
    SkRect drawBounds = SkRect::MakeLargest(); // TODO tighter drawBounds
    fQueue.push(drawBounds, [=](SkArenaAlloc*, const DrawState& ds, const SkIRect& tileBounds){
        TileDraw(ds, tileBounds).drawPoints(mode, count, pts, paint, nullptr);
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
    SkRect drawBounds = path.isInverseFillType() ? SkRect::MakeLargest()
                                                 : get_fast_bounds(path.getBounds(), paint);
    if (path.countVerbs() < 100) { // when path is small, init-once has too much overhead
        fQueue.push(drawBounds, [=](SkArenaAlloc*, const DrawState& ds, const SkIRect& tileBounds) {
            TileDraw(ds, tileBounds).drawPath(path, paint, prePathMatrix, false);
        });
    } else {
        fQueue.push(drawBounds, [=](SkArenaAlloc* alloc, DrawElement* elem) {
            SkInitOnceData data = {alloc, elem};
            elem->fDS.getDraw().drawPath(path, paint, prePathMatrix, false, false, nullptr, &data);
        });
    }
}

void SkThreadedBMPDevice::drawBitmap(const SkBitmap& bitmap, SkScalar x, SkScalar y,
        const SkPaint& paint) {
    SkMatrix matrix = SkMatrix::MakeTrans(x, y);
    LogDrawScaleFactor(SkMatrix::Concat(this->ctm(), matrix), paint.getFilterQuality());
    SkRect drawBounds = SkRect::MakeWH(bitmap.width(), bitmap.height());
    matrix.mapRect(&drawBounds);
    fQueue.push(drawBounds, [=](SkArenaAlloc*, const DrawState& ds, const SkIRect& tileBounds){
        TileDraw(ds, tileBounds).drawBitmap(bitmap, matrix, nullptr, paint);
    });
}

void SkThreadedBMPDevice::drawSprite(const SkBitmap& bitmap, int x, int y, const SkPaint& paint) {
    SkRect drawBounds = SkRect::MakeXYWH(x, y, bitmap.width(), bitmap.height());
    fQueue.push(drawBounds, [=](SkArenaAlloc*, const DrawState& ds, const SkIRect& tileBounds){
        TileDraw(ds, tileBounds).drawSprite(bitmap, x, y, paint);
    });
}

void SkThreadedBMPDevice::drawText(const void* text, size_t len, SkScalar x, SkScalar y,
        const SkPaint& paint) {
    SkRect drawBounds = SkRect::MakeLargest(); // TODO tighter drawBounds
    fQueue.push(drawBounds, [=](SkArenaAlloc*, const DrawState& ds, const SkIRect& tileBounds){
        TileDraw(ds, tileBounds).drawText((const char*)text, len, x, y, paint,
                                          &this->surfaceProps());
    });
}

void SkThreadedBMPDevice::drawPosText(const void* text, size_t len, const SkScalar xpos[],
        int scalarsPerPos, const SkPoint& offset, const SkPaint& paint) {
    SkRect drawBounds = SkRect::MakeLargest(); // TODO tighter drawBounds
    fQueue.push(drawBounds, [=](SkArenaAlloc*, const DrawState& ds, const SkIRect& tileBounds){
        TileDraw(ds, tileBounds).drawPosText((const char*)text, len, xpos, scalarsPerPos, offset,
                                             paint, &surfaceProps());
    });
}

void SkThreadedBMPDevice::drawVertices(const SkVertices* vertices, SkBlendMode bmode,
        const SkPaint& paint) {
    SkRect drawBounds = SkRect::MakeLargest(); // TODO tighter drawBounds
    fQueue.push(drawBounds, [=](SkArenaAlloc*, const DrawState& ds, const SkIRect& tileBounds){
        TileDraw(ds, tileBounds).drawVertices(vertices->mode(), vertices->vertexCount(),
                                              vertices->positions(), vertices->texCoords(),
                                              vertices->colors(), bmode, vertices->indices(),
                                              vertices->indexCount(), paint);
    });
}

void SkThreadedBMPDevice::drawDevice(SkBaseDevice* device, int x, int y, const SkPaint& paint) {
    SkASSERT(!paint.getImageFilter());
    SkRect drawBounds = SkRect::MakeXYWH(x, y, device->width(), device->height());
    fQueue.push(drawBounds, [=](SkArenaAlloc*, const DrawState& ds, const SkIRect& tileBounds){
        TileDraw(ds, tileBounds).drawSprite(static_cast<SkBitmapDevice*>(device)->fBitmap,
                                            x, y, paint);
    });
}
