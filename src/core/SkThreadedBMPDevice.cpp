/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkThreadedBMPDevice.h"

#include "SkPath.h"
#include "SkTaskGroup.h"

SkThreadedBMPDevice::SkThreadedBMPDevice(const SkBitmap& bitmap, const SkSurfaceProps& surfaceProps,
                                         SkRasterHandleAllocator::Handle hndl)
        : INHERITED(bitmap, surfaceProps, hndl)
        , fThreadCnt(surfaceProps.threads())
{
    // Tiling using stripes for now; we'll explore better tiling in the future.
    int h = (fBitmap.height() + fThreadCnt - 1) / std::max(fThreadCnt, 1);
    int w = fBitmap.width();
    int top = 0;
    fThreadBounds.reset();
    for(int tid = 0; tid < fThreadCnt; ++tid, top += h) {
        fThreadBounds.push_back(SkIRect::MakeLTRB(0, top, w, top + h));
    }
}

void SkThreadedBMPDevice::flush() {
    SkTaskGroup group;
    group.batch(fThreadCnt, [this](int i) {
        for(auto& element : fQueue) {
            if (SkIRect::Intersects(fThreadBounds[i], element.fDrawBounds)) {
                element.fDrawFn(fThreadBounds[i]);
            }
        }
    });
    group.wait();
    fQueue.reset();
}

// Having this captured in lambda seems to be faster than saving this in DrawElement
struct SkThreadedBMPDevice::DrawState {
    SkPixmap fDst;
    SkMatrix fMatrix;
    SkRasterClip fRC;

    DrawState(SkThreadedBMPDevice* dev) {
        dev->setDrawDst(fDst);
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
    if (drawBounds.isLargest())
        return SkIRect::MakeLargest();
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

void SkThreadedBMPDevice::drawPaint(const SkPaint& paint) {
    THREADED_DRAW(SkRect::MakeLargest(), drawPaint(paint));
}

void SkThreadedBMPDevice::drawPoints(SkCanvas::PointMode mode, size_t count,
        const SkPoint pts[], const SkPaint& paint) {
    // TO BE IMPLEMENTED
    // BDDraw(this).drawPoints(mode, count, pts, paint, nullptr);
}

void SkThreadedBMPDevice::drawRect(const SkRect& r, const SkPaint& paint) {
    SkRect drawBounds = r.makeOutset(paint.getStrokeWidth(), paint.getStrokeWidth());
    THREADED_DRAW(drawBounds, drawRect(r, paint));
}

void SkThreadedBMPDevice::drawRRect(const SkRRect& rrect, const SkPaint& paint) {
    // TO BE IMPLEMENTED
#ifdef SK_IGNORE_BLURRED_RRECT_OPT
    // SkPath  path;

    // path.addRRect(rrect);
    // // call the VIRTUAL version, so any subclasses who do handle drawPath aren't
    // // required to override drawRRect.
    // this->drawPath(path, paint, nullptr, true);
#else
    // BDDraw(this).drawRRect(rrect, paint);
#endif
}

void SkThreadedBMPDevice::drawPath(const SkPath& path, const SkPaint& paint,
        const SkMatrix* prePathMatrix, bool pathIsMutable) {
    SkRect drawBounds = path.getBounds().makeOutset(paint.getStrokeWidth(), paint.getStrokeWidth());
    THREADED_DRAW(drawBounds, drawPath(path, paint, prePathMatrix, pathIsMutable));
}

void SkThreadedBMPDevice::drawBitmap(const SkBitmap& bitmap, const SkMatrix& matrix,
        const SkPaint& paint) {
    // TO BE IMPLEMENTED
    // LogDrawScaleFactor(SkMatrix::Concat(this->ctm(), matrix), paint.getFilterQuality());
    // BDDraw(this).drawBitmap(bitmap, matrix, nullptr, paint);
}

void SkThreadedBMPDevice::drawSprite(const SkBitmap& bitmap, int x, int y, const SkPaint& paint) {
    // TO BE IMPLEMENTED
    // BDDraw(this).drawSprite(bitmap, x, y, paint);
}

void SkThreadedBMPDevice::drawText(const void* text, size_t len, SkScalar x, SkScalar y,
        const SkPaint& paint) {
    // TO BE IMPLEMENTED
    // BDDraw(this).drawText((const char*)text, len, x, y, paint, &fSurfaceProps);
}

void SkThreadedBMPDevice::drawPosText(const void* text, size_t len, const SkScalar xpos[],
        int scalarsPerPos, const SkPoint& offset, const SkPaint& paint) {
    // TO BE IMPLEMENTED
    // BDDraw(this).drawPosText((const char*)text, len, xpos, scalarsPerPos, offset, paint,
    //                          &fSurfaceProps);
}

void SkThreadedBMPDevice::drawVertices(const SkVertices* vertices, SkBlendMode bmode,
        const SkPaint& paint) {
    // TO BE IMPLEMENTED
    // BDDraw(this).drawVertices(vertices->mode(), vertices->vertexCount(), vertices->positions(),
    //                           vertices->texCoords(), vertices->colors(), bmode,
    //                           vertices->indices(), vertices->indexCount(), paint);
}

void SkThreadedBMPDevice::drawDevice(SkBaseDevice* device, int x, int y, const SkPaint& paint) {
    // TO BE IMPLEMENTED
    // SkASSERT(!paint.getImageFilter());
    // BDDraw(this).drawSprite(static_cast<SkBitmapDevice*>(device)->fBitmap, x, y, paint);
}
