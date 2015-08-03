/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTrackDevice_DEFINED
#define SkTrackDevice_DEFINED

#include "SkBitmapDevice.h"
#include "SkTracker.h"

/** \class SkTrackDevice
 *
 *   A Track Device is used to track the callstack of an operation that affected some pixels.
 *   It can be used with SampleApp to investigate bugs (CL not checked in yet).
 *
 *   every drawFoo is implemented as such:
 *      before();   // - collects state of interesting pixels
 *      INHERITED::drawFoo(...);
 *      after();  // - checks if pixels of interest, and issue a breakpoint.
 *
 */
// TODO: can this be derived from SkBaseDevice instead?
class SkTrackDevice : public SkBitmapDevice {
public:
    SkTrackDevice(const SkBitmap& bitmap)
        : INHERITED(bitmap, SkSurfaceProps(0, kUnknown_SkPixelGeometry))
        , fTracker(NULL) {
    }

    virtual ~SkTrackDevice() {}

    // Install a tracker - we can reuse the tracker between multiple devices, and the state of the
    // tracker is preserved - number and location of poinbts, ...
    void installTracker(SkTracker* tracker) {
        fTracker = tracker;
        fTracker->newFrame();
    }

protected:
#if 0   // clear is deprecated (and private)
    virtual void clear(SkColor color) {
        before();
        INHERITED::clear(color);
        after();
    }
#endif

    void drawPaint(const SkDraw& dummy1, const SkPaint& paint) override {
        before();
        INHERITED::drawPaint(dummy1, paint);
        after();
    }

    void drawPoints(const SkDraw& dummy1, SkCanvas::PointMode mode, size_t count,
                    const SkPoint dummy2[], const SkPaint& paint) override {
        before();
        INHERITED::drawPoints(dummy1, mode, count, dummy2, paint);
        after();
    }

    void drawRect(const SkDraw& dummy1, const SkRect& r, const SkPaint& paint) override {
        before();
        INHERITED::drawRect(dummy1, r, paint);
        after();
    }


    void drawOval(const SkDraw& dummy1, const SkRect& oval, const SkPaint& paint) override {
        before();
        INHERITED::drawOval(dummy1, oval, paint);
        after();
    }

    void drawRRect(const SkDraw& dummy1, const SkRRect& rr, const SkPaint& paint) override {
        before();
        INHERITED::drawRRect(dummy1, rr, paint);
        after();
    }

    void drawPath(const SkDraw& dummy1, const SkPath& path,
                  const SkPaint& paint,
                  const SkMatrix* prePathMatrix = NULL,
                  bool pathIsMutable = false) override {
        before();
        INHERITED::drawPath(dummy1, path, paint, prePathMatrix, pathIsMutable);
        after();
    }

    void drawBitmap(const SkDraw& dummy1, const SkBitmap& bitmap,
                    const SkMatrix& matrix, const SkPaint& paint) override {
        before();
        INHERITED::drawBitmap(dummy1, bitmap, matrix, paint);
        after();
    }

    void drawSprite(const SkDraw& dummy1, const SkBitmap& bitmap,
                    int x, int y, const SkPaint& paint) override {
        before();
        INHERITED::drawSprite(dummy1, bitmap, x, y, paint);
        after();
    }

    void drawBitmapRect(const SkDraw& dummy1, const SkBitmap& dummy2,
                        const SkRect* srcOrNull, const SkRect& dst,
                        const SkPaint& paint,
                        SkCanvas::SrcRectConstraint flags) override {
        before();
        INHERITED::drawBitmapRect(dummy1, dummy2, srcOrNull, dst, paint, flags);
        after();
    }

    void drawText(const SkDraw& dummy1, const void* text, size_t len,
                  SkScalar x, SkScalar y, const SkPaint& paint) override {
        before();
        INHERITED::drawText(dummy1, text, len, x, y, paint);
        after();
    }

    void drawPosText(const SkDraw& dummy1, const void* text, size_t len,
                     const SkScalar pos[], int scalarsPerPos,
                     const SkPoint& offset, const SkPaint& paint) override {
        before();
        INHERITED::drawPosText(dummy1, text, len, pos, scalarsPerPos, offset, paint);
        after();
    }

    void drawTextOnPath(const SkDraw& dummy1, const void* text, size_t len,
                        const SkPath& path, const SkMatrix* matrix,
                        const SkPaint& paint) override {
        before();
        INHERITED::drawTextOnPath(dummy1, text, len, path, matrix, paint);
        after();
    }

    void drawVertices(const SkDraw& dummy1, SkCanvas::VertexMode dummy2, int vertexCount,
                      const SkPoint verts[], const SkPoint texs[],
                      const SkColor colors[], SkXfermode* xmode,
                      const uint16_t indices[], int indexCount,
                      const SkPaint& paint) override {
        before();
        INHERITED::drawVertices(dummy1, dummy2, vertexCount,verts, texs,colors, xmode, indices,
                                indexCount, paint);
        after();
    }

    void drawDevice(const SkDraw& dummy1, SkBaseDevice* dummy2, int x, int y,
                    const SkPaint& dummy3) override {
        before();
        INHERITED::drawDevice(dummy1, dummy2, x, y, dummy3);
        after();
    }

private:
    void before() {
        if (fTracker) {
            fTracker->before(accessBitmap(false));
        }
    }

    // any/all of the expected touched has to be changed, and all expected untouched must be intact
    void after() {
        if (fTracker) {
            fTracker->after(accessBitmap(false));
        }
    }

private:
    SkTracker* fTracker;

    typedef SkBitmapDevice INHERITED;
};

#endif  // SkTrackDevice_DEFINED
