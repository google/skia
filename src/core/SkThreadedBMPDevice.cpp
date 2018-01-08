/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkThreadedBMPDevice.h"

#include "SkPath.h"
#include "SkRectPriv.h"
#include "SkTaskGroup.h"
#include "SkVertices.h"

void SkThreadedBMPDevice::DrawQueue::reset() {
    if (fTasks) {
        fTasks->finish();
    }

    fSize = 0;

    // using TaskGroup2D = SkSpinningTaskGroup2D;
    using TaskGroup2D = SkFlexibleTaskGroup2D;
    auto draw2D = [this](int row, int column){
        SkThreadedBMPDevice::DrawElement& drawElement = fElements[column];
        if (!SkIRect::Intersects(fDevice->fTileBounds[row], drawElement.fDrawBounds)) {
            return;
        }
        drawElement.fDrawFn(fDevice->fTileBounds[row]);
    };
    fTasks.reset(new TaskGroup2D(draw2D, fDevice->fTileCnt, fDevice->fExecutor,
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

// Having this captured in lambda seems to be faster than saving this in DrawElement
struct SkThreadedBMPDevice::DrawState {
    SkPixmap fDst;
    SkMatrix fMatrix;
    SkRasterClip fRC;

    explicit DrawState(SkThreadedBMPDevice* dev) {
        // we need fDst to be set, and if we're actually drawing, to dirty the genID
        if (!dev->accessPixels(&fDst)) {
            // NoDrawDevice uses us (why?) so we have to catch this case w/ no pixels
            fDst.reset(dev->imageInfo(), nullptr, 0);
        }
        fMatrix = dev->ctm();
        fRC = dev->fRCStack.rc();
    }

    SkDraw getThreadDraw(SkRasterClip& threadRC, const SkIRect& threadBounds) const {
        SkDraw draw;
        draw.fDst = fDst;
        draw.fMatrix = &fMatrix;
        threadRC = fRC;
        threadRC.op(threadBounds, SkRegion::kIntersect_Op);
        draw.fRC = &threadRC;
        return draw;
    }
};

SkIRect SkThreadedBMPDevice::transformDrawBounds(const SkRect& drawBounds) const {
    SkRect transformedBounds;
    this->ctm().mapRect(&transformedBounds, drawBounds);
    if (!transformedBounds.isFinite()) {
        transformedBounds = SkRectPriv::MakeLargestS32();
    }
    return transformedBounds.roundOut();
}

// The do {...} while (false) is to enforce trailing semicolon as suggested by mtklein@
#define THREADED_DRAW(drawBounds, actualDrawCall)                                                  \
    do {                                                                                           \
        DrawState ds(this);                                                                        \
        fQueue.push({                                                                              \
            this->transformDrawBounds(drawBounds),                                                 \
            [=](const SkIRect& tileBounds) {                                                       \
                SkRasterClip tileRC;                                                               \
                SkDraw draw = ds.getThreadDraw(tileRC, tileBounds);                                \
                draw.actualDrawCall;                                                               \
            },                                                                                     \
        });                                                                                        \
    } while (false)

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
    THREADED_DRAW(SkRectPriv::MakeLargest(), drawPaint(paint));
}

void SkThreadedBMPDevice::drawPoints(SkCanvas::PointMode mode, size_t count,
        const SkPoint pts[], const SkPaint& paint) {
    // TODO tighter drawBounds
    SkRect drawBounds = SkRectPriv::MakeLargest();
    THREADED_DRAW(drawBounds, drawPoints(mode, count, pts, paint, nullptr));
}

void SkThreadedBMPDevice::drawRect(const SkRect& r, const SkPaint& paint) {
    SkRect drawBounds = get_fast_bounds(r, paint);
    THREADED_DRAW(drawBounds, drawRect(r, paint));
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
    THREADED_DRAW(drawBounds, drawRRect(rrect, paint));
#endif
}

void SkThreadedBMPDevice::drawPath(const SkPath& path, const SkPaint& paint,
        const SkMatrix* prePathMatrix, bool pathIsMutable) {
    SkRect drawBounds = path.isInverseFillType() ? SkRectPriv::MakeLargest()
                                                 : get_fast_bounds(path.getBounds(), paint);
    // For thread safety, make path imutable
    THREADED_DRAW(drawBounds, drawPath(path, paint, prePathMatrix, false));
}

void SkThreadedBMPDevice::drawBitmap(const SkBitmap& bitmap, SkScalar x, SkScalar y,
        const SkPaint& paint) {
    SkMatrix matrix = SkMatrix::MakeTrans(x, y);
    LogDrawScaleFactor(SkMatrix::Concat(this->ctm(), matrix), paint.getFilterQuality());
    SkRect drawBounds = SkRect::MakeWH(bitmap.width(), bitmap.height());
    matrix.mapRect(&drawBounds);
    THREADED_DRAW(drawBounds, drawBitmap(bitmap, matrix, nullptr, paint));
}

void SkThreadedBMPDevice::drawSprite(const SkBitmap& bitmap, int x, int y, const SkPaint& paint) {
    SkRect drawBounds = SkRect::MakeXYWH(x, y, bitmap.width(), bitmap.height());
    THREADED_DRAW(drawBounds, drawSprite(bitmap, x, y, paint));
}

void SkThreadedBMPDevice::drawText(const void* text, size_t len, SkScalar x, SkScalar y,
        const SkPaint& paint) {
    SkRect drawBounds = SkRectPriv::MakeLargest(); // TODO tighter drawBounds
    THREADED_DRAW(drawBounds, drawText((const char*)text, len, x, y, paint, &this->surfaceProps()));
}

void SkThreadedBMPDevice::drawPosText(const void* text, size_t len, const SkScalar xpos[],
        int scalarsPerPos, const SkPoint& offset, const SkPaint& paint) {
    SkRect drawBounds = SkRectPriv::MakeLargest(); // TODO tighter drawBounds
    THREADED_DRAW(drawBounds, drawPosText((const char*)text, len, xpos, scalarsPerPos, offset,
                                          paint, &surfaceProps()));
}

void SkThreadedBMPDevice::drawVertices(const SkVertices* vertices, SkBlendMode bmode,
        const SkPaint& paint) {
    SkRect drawBounds = SkRectPriv::MakeLargest(); // TODO tighter drawBounds
    THREADED_DRAW(drawBounds, drawVertices(vertices->mode(), vertices->vertexCount(),
                                           vertices->positions(), vertices->texCoords(),
                                           vertices->colors(), bmode, vertices->indices(),
                                           vertices->indexCount(), paint));
}

void SkThreadedBMPDevice::drawDevice(SkBaseDevice* device, int x, int y, const SkPaint& paint) {
    SkASSERT(!paint.getImageFilter());
    SkRect drawBounds = SkRect::MakeXYWH(x, y, device->width(), device->height());
    THREADED_DRAW(drawBounds,
                  drawSprite(static_cast<SkBitmapDevice*>(device)->fBitmap, x, y, paint));
}
