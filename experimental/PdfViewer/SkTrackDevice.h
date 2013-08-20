#ifndef EXPERIMENTAL_PDFVIEWER_SKTRACKDEVICE_H_
#define EXPERIMENTAL_PDFVIEWER_SKTRACKDEVICE_H_

#include "SkDevice.h"
#include "SkTracker.h"

class SkTrackDevice : public SkDevice {
public:
    SK_DECLARE_INST_COUNT(SkTrackDevice)

    SkTrackDevice(const SkBitmap& bitmap) : SkDevice(bitmap)
                                          , fTracker(NULL) {}

    SkTrackDevice(const SkBitmap& bitmap, const SkDeviceProperties& deviceProperties)
        : SkDevice(bitmap, deviceProperties)
        , fTracker(NULL) {}

    SkTrackDevice(SkBitmap::Config config, int width, int height, bool isOpaque = false)
        : SkDevice(config, width, height, isOpaque)
        , fTracker(NULL) {}

    SkTrackDevice(SkBitmap::Config config, int width, int height, bool isOpaque,
                  const SkDeviceProperties& deviceProperties)
        : SkDevice(config, width, height, isOpaque, deviceProperties)
        , fTracker(NULL) {}

    virtual ~SkTrackDevice() {}

    void installTracker(SkTracker* tracker) {
        fTracker = tracker;
        fTracker->newFrame();
    }

protected:
    virtual void clear(SkColor color) {
        before();
        INHERITED::clear(color);
        after();
    }

    virtual void drawPaint(const SkDraw& dummy1, const SkPaint& paint) {
        before();
        INHERITED::drawPaint(dummy1, paint);
        after();
    }

    virtual void drawPoints(const SkDraw& dummy1, SkCanvas::PointMode mode, size_t count,
                            const SkPoint dummy2[], const SkPaint& paint) {
        before();
        INHERITED::drawPoints(dummy1, mode, count, dummy2, paint);
        after();
    }

    virtual void drawRect(const SkDraw& dummy1, const SkRect& r,
                          const SkPaint& paint) {
        before();
        INHERITED::drawRect(dummy1, r, paint);
        after();
    }


    virtual void drawOval(const SkDraw& dummy1, const SkRect& oval,
                          const SkPaint& paint) {
        before();
        INHERITED::drawOval(dummy1, oval, paint);
        after();
    }

    virtual void drawRRect(const SkDraw& dummy1, const SkRRect& rr,
                           const SkPaint& paint) {
        before();
        INHERITED::drawRRect(dummy1, rr, paint);
        after();
    }

    virtual void drawPath(const SkDraw& dummy1, const SkPath& path,
                          const SkPaint& paint,
                          const SkMatrix* prePathMatrix = NULL,
                          bool pathIsMutable = false) {
        before();
        INHERITED::drawPath(dummy1, path, paint, prePathMatrix, pathIsMutable);
        after();
    }

    virtual void drawBitmap(const SkDraw& dummy1, const SkBitmap& bitmap,
                            const SkMatrix& matrix, const SkPaint& paint) {
        before();
        INHERITED::drawBitmap(dummy1, bitmap, matrix, paint);
        after();
    }

    virtual void drawSprite(const SkDraw& dummy1, const SkBitmap& bitmap,
                            int x, int y, const SkPaint& paint) {
        before();
        INHERITED::drawSprite(dummy1, bitmap, x, y, paint);
        after();
    }

    virtual void drawBitmapRect(const SkDraw& dummy1, const SkBitmap& dummy2,
                                const SkRect* srcOrNull, const SkRect& dst,
                                const SkPaint& paint,
                                SkCanvas::DrawBitmapRectFlags flags) {
        before();
        INHERITED::drawBitmapRect(dummy1, dummy2, srcOrNull, dst, paint, flags);
        after();
    }

    virtual void drawText(const SkDraw& dummy1, const void* text, size_t len,
                          SkScalar x, SkScalar y, const SkPaint& paint) {
        before();
        INHERITED::drawText(dummy1, text, len, x, y, paint);
        after();
    }

    virtual void drawPosText(const SkDraw& dummy1, const void* text, size_t len,
                             const SkScalar pos[], SkScalar constY,
                             int scalarsPerPos, const SkPaint& paint) {
        before();
        INHERITED::drawPosText(dummy1, text, len, pos, constY, scalarsPerPos, paint);
        after();
    }

    virtual void drawTextOnPath(const SkDraw& dummy1, const void* text, size_t len,
                                const SkPath& path, const SkMatrix* matrix,
                                const SkPaint& paint)  {
        before();
        INHERITED::drawTextOnPath(dummy1, text, len, path, matrix, paint);
        after();
    }

#ifdef SK_BUILD_FOR_ANDROID
    virtual void drawPosTextOnPath(const SkDraw& draw, const void* text, size_t len,
                                   const SkPoint pos[], const SkPaint& paint,
                                   const SkPath& path, const SkMatrix* matrix)  {
        before();
        INHERITED::drawPosTextOnPath(draw, text, len, pos, paint, path, matrix);
        after();
    }
#endif
    virtual void drawVertices(const SkDraw& dummy1, SkCanvas::VertexMode dummy2, int vertexCount,
                              const SkPoint verts[], const SkPoint texs[],
                              const SkColor colors[], SkXfermode* xmode,
                              const uint16_t indices[], int indexCount,
                              const SkPaint& paint) {
        before();
        INHERITED::drawVertices(dummy1, dummy2, vertexCount,verts, texs,colors, xmode, indices, indexCount, paint);
        after();
    }

    virtual void drawDevice(const SkDraw& dummy1, SkDevice* dummy2, int x, int y,
                            const SkPaint& dummy3) {
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

    typedef SkDevice INHERITED;
};

#endif  // EXPERIMENTAL_PDFVIEWER_SKTRACKDEVICE_H_
