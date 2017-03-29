
/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkThreadedDraw_DEFINED
#define SkThreadedDraw_DEFINED

#include "SkDraw.h"
#include "SkPath.h"
#include "SkRRect.h"
#include "SkBitmapDevice.h"

struct DrawElement {
    SkIRect fDrawBounds;
    std::function<void(const SkIRect& threadBounds)> fDrawFn;
    // As mtklein@ suggested, we may later have std::atomic<std::funciton*> fInitFn
};

// Having this captured in lambda seems to be faster than saving this in DrawElement
struct DrawState {
    SkPixmap fDst;
    SkMatrix fMatrix;
    SkRasterClip fRC;

    DrawState(SkBitmapDevice* dev) {
        SkBitmapDevice::SetDrawDst(dev, fDst);
        fMatrix = dev->ctm();
        fRC = dev->fRCStack.rc();
    }

    SK_ALWAYS_INLINE
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

// Currently, we have the simplest two-pass approach.
// Later we'll add concurrent queue pushing and draw execution, as well as init once.
class SkThreadedAccelerator {
public:
    static constexpr int kDefaultThreadCnt = 0;

    SkThreadedAccelerator(SkBitmapDevice* dev, int threadCnt = kDefaultThreadCnt);

    void flush();

    void setThreadCnt(int cnt);
    inline int getThreadCnt() { return fThreadCnt; }

private:
    friend class SkThreadedDraw;

    void updateThreadBounds();

    int fThreadCnt;
    SkTArray<SkIRect> fThreadBounds;
    SkTArray<DrawElement> fQueue;

    SkBitmapDevice* fDevice;
};

// The API is exactly the same as SkDraw.
// We re-declare everyone of them because SkDraw's API isn't virtual.
class SkThreadedDraw {
public:
    SkThreadedDraw(SkBitmapDevice* dev);

    void drawPaint(const SkPaint&) const;
    void drawPoints(SkCanvas::PointMode, size_t count, const SkPoint[],
                    const SkPaint&, SkBaseDevice*) const;
    void drawRect(const SkRect& prePaintRect, const SkPaint&, const SkMatrix* paintMatrix,
                  const SkRect* postPaintRect) const;
    void drawRect(const SkRect& rect, const SkPaint& paint) const {
        this->drawRect(rect, paint, NULL, NULL);
    }
    void drawRRect(const SkRRect&, const SkPaint&) const;
    SK_ALWAYS_INLINE void drawPath(const SkPath& path, const SkPaint& paint,
                  const SkMatrix* prePathMatrix, bool pathIsMutable) const {
        this->drawPath(path, paint, prePathMatrix, pathIsMutable, false);
    }

    SK_ALWAYS_INLINE void drawPath(const SkPath& path, const SkPaint& paint,
                  SkBlitter* customBlitter = NULL) const {
        this->drawPath(path, paint, NULL, false, false, customBlitter);
    }

    void drawBitmap(const SkBitmap&, const SkMatrix&, const SkRect* dstOrNull,
                    const SkPaint&) const;
    void drawSprite(const SkBitmap&, int x, int y, const SkPaint&) const;
    void drawText(const char text[], size_t byteLength, SkScalar x,
                  SkScalar y, const SkPaint& paint, const SkSurfaceProps*) const;
    void drawPosText(const char text[], size_t byteLength,
                     const SkScalar pos[], int scalarsPerPosition,
                     const SkPoint& offset, const SkPaint&, const SkSurfaceProps*) const;
    void drawVertices(SkCanvas::VertexMode mode, int count,
                      const SkPoint vertices[], const SkPoint textures[],
                      const SkColor colors[], SkBlendMode bmode,
                      const uint16_t indices[], int ptCount,
                      const SkPaint& paint) const;

    void drawPathCoverage(const SkPath& src, const SkPaint& paint,
                          SkBlitter* customBlitter = NULL) const {
        this->drawPath(src, paint, NULL, false, true, customBlitter);
    }


    void drawText_asPaths(const char text[], size_t byteLength, SkScalar x, SkScalar y,
                          const SkPaint&) const;
    void drawPosText_asPaths(const char text[], size_t byteLength, const SkScalar pos[],
                             int scalarsPerPosition, const SkPoint& offset,
                             const SkPaint&, const SkSurfaceProps*) const;

private:
    SkThreadedAccelerator* fAccelerator;
    SkBitmapDevice* fDevice;

    SK_ALWAYS_INLINE
    void drawPath(const SkPath& path, const SkPaint& paint, const SkMatrix* preMatrix,
                  bool pathIsMutable, bool drawCoverage, SkBlitter* customBlitter = NULL) const {
        DrawState ds(fDevice);
        SkRect drawBounds = path.getBounds().makeOutset(paint.getStrokeWidth(), paint.getStrokeWidth());

        // fAccelerator->pushDrawFn(
        //     [=](const SkIRect& threadBounds) {
        //         SkRasterClip threadRC;
        //         SkDraw draw = ds.getThreadDraw(threadRC, threadBounds);
        //         draw.drawPath(path, paint, preMatrix, pathIsMutable, drawCoverage, customBlitter);
        //     },
        //     drawBounds
        // );

        fAccelerator->fQueue.push_back({
            drawBounds.roundOut(),
            [=](const SkIRect& threadBounds) {
                SkRasterClip threadRC;
                SkDraw draw = ds.getThreadDraw(threadRC, threadBounds);
                draw.drawPath(path, paint, preMatrix, pathIsMutable, drawCoverage, customBlitter);
            },
        });
    }
};

#endif
