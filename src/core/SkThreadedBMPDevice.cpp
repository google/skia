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

SkThreadedBMPDevice::SkThreadedBMPDevice(const SkBitmap& bitmap, int threads)
        : INHERITED(bitmap)
        , fThreadCnt(threads)
{
    // Tiling using stripes for now; we'll explore better tiling in the future.
    int h = (bitmap.height() + fThreadCnt - 1) / SkTMax(fThreadCnt, 1);
    int w = bitmap.width();
    int top = 0;
    for(int tid = 0; tid < fThreadCnt; ++tid, top += h) {
        fThreadBounds.push_back(SkIRect::MakeLTRB(0, top, w, top + h));
    }
}

void SkThreadedBMPDevice::flush() {
    SkTaskGroup().batch(fThreadCnt, [this](int i) {
        for(auto& element : fQueue) {
            if (SkIRect::Intersects(fThreadBounds[i], element.fDrawBounds)) {
                element.fDrawFn(fThreadBounds[i]);
            }
        }
    });
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
    if (drawBounds.isLargest()) {
        return SkIRect::MakeLargest();
    }
    SkRect transformedBounds;
    this->ctm().mapRect(&transformedBounds, drawBounds);
    return transformedBounds.roundOut();
}

// The do {...} while (false) is to enforce trailing semicolon as suggested by mtklein@
#define THREADED_DRAW(drawBounds, actualDrawCall)                                                  \
    do {                                                                                           \
        DrawState ds(this);                                                                        \
        fQueue.push_back({                                                                         \
            this->transformDrawBounds(drawBounds),                                                 \
            [=](const SkIRect& threadBounds) {                                                     \
                SkRasterClip threadRC;                                                             \
                SkDraw draw = ds.getThreadDraw(threadRC, threadBounds);                            \
                draw.actualDrawCall;                                                               \
            },                                                                                     \
        });                                                                                        \
    } while (false)

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
    THREADED_DRAW(SkRect::MakeLargest(), drawPaint(paint));
}

void SkThreadedBMPDevice::drawPoints(SkCanvas::PointMode mode, size_t count,
        const SkPoint pts[], const SkPaint& paint) {
    // TODO tighter drawBounds
    SkRect drawBounds = SkRect::MakeLargest();
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
    SkRect drawBounds = path.isInverseFillType() ? SkRect::MakeLargest()
                                                 : get_fast_bounds(path.getBounds(), paint);
    // For thread safety, make path imutable
    THREADED_DRAW(drawBounds, drawPath(path, paint, prePathMatrix, false));
}

void SkThreadedBMPDevice::drawBitmap(const SkBitmap& bitmap, const SkMatrix& matrix,
        const SkPaint& paint) {
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
    SkRect drawBounds = SkRect::MakeLargest(); // TODO tighter drawBounds
    THREADED_DRAW(drawBounds, drawText((const char*)text, len, x, y, paint, &this->surfaceProps()));
}

void SkThreadedBMPDevice::drawPosText(const void* text, size_t len, const SkScalar xpos[],
        int scalarsPerPos, const SkPoint& offset, const SkPaint& paint) {
    SkRect drawBounds = SkRect::MakeLargest(); // TODO tighter drawBounds
    THREADED_DRAW(drawBounds, drawPosText((const char*)text, len, xpos, scalarsPerPos, offset,
                                          paint, &surfaceProps()));
}

void SkThreadedBMPDevice::drawVertices(const SkVertices* vertices, SkBlendMode bmode,
        const SkPaint& paint) {
    SkRect drawBounds = SkRect::MakeLargest(); // TODO tighter drawBounds
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
