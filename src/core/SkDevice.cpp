#include "SkDevice.h"
#include "SkDraw.h"
#include "SkRect.h"

SkDevice::SkDevice() {}

SkDevice::SkDevice(const SkBitmap& bitmap) : fBitmap(bitmap) {}

void SkDevice::lockPixels() {
    fBitmap.lockPixels();
}

void SkDevice::unlockPixels() {
    fBitmap.unlockPixels();
}

const SkBitmap& SkDevice::accessBitmap(bool changePixels) {
    this->onAccessBitmap(&fBitmap);
    if (changePixels) {
        fBitmap.notifyPixelsChanged();
    }
    return fBitmap;
}

void SkDevice::getBounds(SkIRect* bounds) const {
    if (bounds) {
        bounds->set(0, 0, fBitmap.width(), fBitmap.height());
    }
}

bool SkDevice::intersects(const SkIRect& r, SkIRect* sect) const {
    SkIRect bounds;
    
    this->getBounds(&bounds);
    return sect ? sect->intersect(r, bounds) : SkIRect::Intersects(r, bounds);
}

void SkDevice::eraseColor(SkColor eraseColor) {
    fBitmap.eraseColor(eraseColor);
}

void SkDevice::onAccessBitmap(SkBitmap* bitmap) {}

void SkDevice::setMatrixClip(const SkMatrix&, const SkRegion&) {}

///////////////////////////////////////////////////////////////////////////////

void SkDevice::drawPaint(const SkDraw& draw, const SkPaint& paint) {
    draw.drawPaint(paint);
}

void SkDevice::drawPoints(const SkDraw& draw, SkCanvas::PointMode mode, size_t count,
                              const SkPoint pts[], const SkPaint& paint) {
    draw.drawPoints(mode, count, pts, paint);
}

void SkDevice::drawRect(const SkDraw& draw, const SkRect& r,
                            const SkPaint& paint) {
    draw.drawRect(r, paint);
}

void SkDevice::drawPath(const SkDraw& draw, const SkPath& path,
                            const SkPaint& paint) {
    draw.drawPath(path, paint);
}

void SkDevice::drawBitmap(const SkDraw& draw, const SkBitmap& bitmap,
                              const SkMatrix& matrix, const SkPaint& paint) {
    draw.drawBitmap(bitmap, matrix, paint);
}

void SkDevice::drawSprite(const SkDraw& draw, const SkBitmap& bitmap,
                              int x, int y, const SkPaint& paint) {
    draw.drawSprite(bitmap, x, y, paint);
}

void SkDevice::drawText(const SkDraw& draw, const void* text, size_t len,
                            SkScalar x, SkScalar y, const SkPaint& paint) {
    draw.drawText((const char*)text, len, x, y, paint);
}

void SkDevice::drawPosText(const SkDraw& draw, const void* text, size_t len,
                               const SkScalar xpos[], SkScalar y,
                               int scalarsPerPos, const SkPaint& paint) {
    draw.drawPosText((const char*)text, len, xpos, y, scalarsPerPos, paint);
}

void SkDevice::drawTextOnPath(const SkDraw& draw, const void* text,
                                  size_t len, const SkPath& path,
                                  const SkMatrix* matrix,
                                  const SkPaint& paint) {
    draw.drawTextOnPath((const char*)text, len, path, matrix, paint);
}

void SkDevice::drawVertices(const SkDraw& draw, SkCanvas::VertexMode vmode,
                                int vertexCount,
                                const SkPoint verts[], const SkPoint textures[],
                                const SkColor colors[], SkXfermode* xmode,
                                const uint16_t indices[], int indexCount,
                                const SkPaint& paint) {
    draw.drawVertices(vmode, vertexCount, verts, textures, colors, xmode,
                      indices, indexCount, paint);
}

void SkDevice::drawDevice(const SkDraw& draw, SkDevice* device,
                              int x, int y, const SkPaint& paint) {
    draw.drawSprite(device->accessBitmap(false), x, y, paint);
}


