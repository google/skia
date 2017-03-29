/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkThreadedDraw.h"
#include "SkBitmapDevice.h"
#include "SkTaskGroup.h"
#include <functional>

int gSkThreadCnt = 1;

// We use stripes as tiles for now. Later, we can explore what tile shape is better.
void SkThreadedAccelerator::updateThreadBounds() {
    int h = (fDevice->fBitmap.height() + fThreadCnt - 1) / std::max(fThreadCnt, 1);
    int w = fDevice->fBitmap.width();
    int top = 0;
    fThreadBounds.reset();
    for(int tid = 0; tid < fThreadCnt; ++tid, top += h)
        fThreadBounds.push_back(SkIRect::MakeLTRB(0, top, w, top + h));
}

SkThreadedAccelerator::SkThreadedAccelerator(SkBitmapDevice* dev, int threadCnt)
        : fThreadCnt(threadCnt)
        , fDevice(dev)
{
    updateThreadBounds();
}

void SkThreadedAccelerator::setThreadCnt(int cnt) {
    fThreadCnt = cnt;
    updateThreadBounds();
}

void SkThreadedAccelerator::flush() {
    return;
    SkTaskGroup group;
    group.batch(fThreadCnt, [this](int i) {
        for(auto& element : fQueue)
            if (SkIRect::Intersects(fThreadBounds[i], element.fDrawBounds))
                element.fDrawFn(fThreadBounds[i]);
    });
    group.wait();
    fQueue.reset();
}

void SkThreadedAccelerator::pushDrawFn(std::function<void(const SkIRect&)>&& drawFn,
                                       SkRect drawBounds) {
    SkIRect devIBounds = SkIRect::MakeLargest();
    if (!drawBounds.isLargest()) {
        SkRect devBounds;
        fDevice->ctm().mapRect(&devBounds, drawBounds);
        devBounds.roundOut(&devIBounds);
    }
    fQueue.push_back({devIBounds, std::move(drawFn)});
}

SkThreadedDraw::SkThreadedDraw(SkBitmapDevice* dev) {
    fAccelerator = dev->fAccelerator;
    fDevice = dev;
}

void SkThreadedDraw::drawPaint(const SkPaint& paint) const {
    DrawState ds(fDevice);
    fAccelerator->pushDrawFn([=](const SkIRect& threadBounds) {
        SkRasterClip threadRC;
        SkDraw draw = ds.getThreadDraw(threadRC, threadBounds);
        draw.drawPaint(paint);
    });
}

void SkThreadedDraw::drawPoints(SkCanvas::PointMode mode, size_t count, const SkPoint points[],
                                const SkPaint& paint, SkBaseDevice* device) const {
    DrawState ds(fDevice);
    // TODO compute draw bounds
    fAccelerator->pushDrawFn([=](const SkIRect& threadBounds) {
        SkRasterClip threadRC;
        SkDraw draw = ds.getThreadDraw(threadRC, threadBounds);
        draw.drawPoints(mode, count, points, paint, device);
    });
}

void SkThreadedDraw::drawRect(const SkRect& prePaintRect, const SkPaint& paint,
        const SkMatrix* paintMatrix, const SkRect* postPaintRect) const {
    DrawState ds(fDevice);
    // TODO compute draw bounds
    fAccelerator->pushDrawFn([=](const SkIRect& threadBounds) {
        SkRasterClip threadRC;
        SkDraw draw = ds.getThreadDraw(threadRC, threadBounds);
        draw.drawRect(prePaintRect, paint, paintMatrix, postPaintRect);
    });
}

void SkThreadedDraw::drawRRect(const SkRRect& rrect, const SkPaint& paint) const {
    DrawState ds(fDevice);
    // TODO compute draw bounds
    fAccelerator->pushDrawFn([=](const SkIRect& threadBounds) {
        SkRasterClip threadRC;
        SkDraw draw = ds.getThreadDraw(threadRC, threadBounds);
        draw.drawRRect(rrect, paint);
    });
}

void SkThreadedDraw::drawBitmap(const SkBitmap& bitmap, const SkMatrix& matrix,
                                const SkRect* dstOrNull, const SkPaint& paint) const {
    DrawState ds(fDevice);
    // TODO compute draw bounds
    fAccelerator->pushDrawFn([=](const SkIRect& threadBounds) {
        SkRasterClip threadRC;
        SkDraw draw = ds.getThreadDraw(threadRC, threadBounds);
        draw.drawBitmap(bitmap, matrix, dstOrNull, paint);
    });
}

void SkThreadedDraw::drawSprite(const SkBitmap& bitmap, int x, int y, const SkPaint& paint) const {
    DrawState ds(fDevice);
    // TODO compute draw bounds
    fAccelerator->pushDrawFn([=](const SkIRect& threadBounds) {
        SkRasterClip threadRC;
        SkDraw draw = ds.getThreadDraw(threadRC, threadBounds);
        draw.drawSprite(bitmap, x, y, paint);
    });
}

void SkThreadedDraw::drawText(const char text[], size_t byteLength, SkScalar x,
                              SkScalar y, const SkPaint& paint, const SkSurfaceProps* props) const {
    DrawState ds(fDevice);
    // TODO compute draw bounds
    fAccelerator->pushDrawFn([=](const SkIRect& threadBounds) {
        SkRasterClip threadRC;
        SkDraw draw = ds.getThreadDraw(threadRC, threadBounds);
        draw.drawText(text, byteLength, x, y, paint, props);
    });
}

void SkThreadedDraw::drawPosText(const char text[], size_t byteLength, const SkScalar pos[],
                                 int scalarsPerPosition, const SkPoint& offset, const SkPaint& p,
                                 const SkSurfaceProps* props) const {
    DrawState ds(fDevice);
    // TODO compute draw bounds
    fAccelerator->pushDrawFn([=](const SkIRect& threadBounds) {
        SkRasterClip threadRC;
        SkDraw draw = ds.getThreadDraw(threadRC, threadBounds);
        draw.drawPosText(text, byteLength, pos, scalarsPerPosition, offset, p, props);
    });
}

void SkThreadedDraw::drawVertices(SkCanvas::VertexMode mode, int count,
                                  const SkPoint vertices[], const SkPoint textures[],
                                  const SkColor colors[], SkBlendMode bmode,
                                  const uint16_t indices[], int ptCount,
                                  const SkPaint& paint) const {
    DrawState ds(fDevice);
    // TODO compute draw bounds
    fAccelerator->pushDrawFn([=](const SkIRect& threadBounds) {
        SkRasterClip threadRC;
        SkDraw draw = ds.getThreadDraw(threadRC, threadBounds);
        draw.drawVertices(mode, count, vertices, textures, colors, bmode, indices, ptCount, paint);
    });
}

void SkThreadedDraw::drawText_asPaths(const char text[], size_t byteLength, SkScalar x, SkScalar y,
                                      const SkPaint& paint) const {
    DrawState ds(fDevice);
    // TODO compute draw bounds
    fAccelerator->pushDrawFn([=](const SkIRect& threadBounds) {
        SkRasterClip threadRC;
        SkDraw draw = ds.getThreadDraw(threadRC, threadBounds);
        draw.drawText_asPaths(text, byteLength, x, y, paint);
    });
}

void SkThreadedDraw::drawPosText_asPaths(const char text[], size_t byteLength, const SkScalar pos[],
                                         int scalarsPerPosition, const SkPoint& offset,
                                         const SkPaint& paint, const SkSurfaceProps* props) const {
    DrawState ds(fDevice);
    // TODO compute draw bounds
    fAccelerator->pushDrawFn([=](const SkIRect& threadBounds) {
        SkRasterClip threadRC;
        SkDraw draw = ds.getThreadDraw(threadRC, threadBounds);
        draw.drawPosText_asPaths(text, byteLength, pos, scalarsPerPosition, offset, paint, props);
    });
}

// void SkThreadedDraw::drawPath(const SkPath& path, const SkPaint& paint, const SkMatrix* preMatrix,
//               bool pathIsMutable, bool drawCoverage, SkBlitter* customBlitter) const {
//     DrawState ds(fDevice);
//     SkRect drawBounds = path.getBounds().makeOutset(paint.getStrokeWidth(), paint.getStrokeWidth());

//     // fAccelerator->pushDrawFn(
//     //     [=](const SkIRect& threadBounds) {
//     //         SkRasterClip threadRC;
//     //         SkDraw draw = ds.getThreadDraw(threadRC, threadBounds);
//     //         draw.drawPath(path, paint, preMatrix, pathIsMutable, drawCoverage, customBlitter);
//     //     },
//     //     drawBounds
//     // );

//     fAccelerator->fQueue.push_back({
//         drawBounds.roundOut(),
//         [=](const SkIRect& threadBounds) {
//             SkRasterClip threadRC;
//             SkDraw draw = ds.getThreadDraw(threadRC, threadBounds);
//             draw.drawPath(path, paint, preMatrix, pathIsMutable, drawCoverage, customBlitter);
//         },
//     });
// }
