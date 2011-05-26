#include "SkDevice.h"
#include "SkDraw.h"
#include "SkMetaData.h"
#include "SkRect.h"

//#define TRACE_FACTORY_LIFETIME

#ifdef TRACE_FACTORY_LIFETIME
    static int gFactoryCounter;
#endif

SkDeviceFactory::SkDeviceFactory() {
#ifdef TRACE_FACTORY_LIFETIME
    SkDebugf("+++ factory index %d\n", gFactoryCounter);
    ++gFactoryCounter;
#endif
}

SkDeviceFactory::~SkDeviceFactory() {
#ifdef TRACE_FACTORY_LIFETIME
    --gFactoryCounter;
    SkDebugf("--- factory index %d\n", gFactoryCounter);
#endif
}

///////////////////////////////////////////////////////////////////////////////

SkDevice::SkDevice(SkCanvas* canvas) : fCanvas(canvas), fMetaData(NULL) {
    fOrigin.setZero();
    fCachedDeviceFactory = NULL;
}

SkDevice::SkDevice(SkCanvas* canvas, const SkBitmap& bitmap, bool isForLayer)
        : fCanvas(canvas), fBitmap(bitmap), fMetaData(NULL) {
    fOrigin.setZero();
    // auto-allocate if we're for offscreen drawing
    if (isForLayer) {
        if (NULL == fBitmap.getPixels() && NULL == fBitmap.pixelRef()) {
            fBitmap.allocPixels();
            if (!fBitmap.isOpaque()) {
                fBitmap.eraseColor(0);
            }
        }
    }
    fCachedDeviceFactory = NULL;
}

SkDevice::~SkDevice() {
    delete fMetaData;
    SkSafeUnref(fCachedDeviceFactory);
}

SkDeviceFactory* SkDevice::onNewDeviceFactory() {
    return SkNEW(SkRasterDeviceFactory);
}

SkDeviceFactory* SkDevice::getDeviceFactory() {
    if (NULL == fCachedDeviceFactory) {
        fCachedDeviceFactory = this->onNewDeviceFactory();
    }
    return fCachedDeviceFactory;
}

SkMetaData& SkDevice::getMetaData() {
    // metadata users are rare, so we lazily allocate it. If that changes we
    // can decide to just make it a field in the device (rather than a ptr)
    if (NULL == fMetaData) {
        fMetaData = new SkMetaData;
    }
    return *fMetaData;
}

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

void SkDevice::clear(SkColor color) {
    fBitmap.eraseColor(color);
}

void SkDevice::onAccessBitmap(SkBitmap* bitmap) {}

void SkDevice::setMatrixClip(const SkMatrix& matrix, const SkRegion& region,
                             const SkClipStack& clipStack) {
}

///////////////////////////////////////////////////////////////////////////////

bool SkDevice::readPixels(const SkIRect& srcRect, SkBitmap* bitmap) {
    const SkBitmap& src = this->accessBitmap(false);

    SkIRect bounds;
    bounds.set(0, 0, src.width(), src.height());
    if (!bounds.intersect(srcRect)) {
        return false;
    }

    SkBitmap subset;
    if (!src.extractSubset(&subset, bounds)) {
        return false;
    }

    SkBitmap tmp;
    if (!subset.copyTo(&tmp, SkBitmap::kARGB_8888_Config)) {
        return false;
    }

    tmp.swap(*bitmap);
    return true;
}

void SkDevice::writePixels(const SkBitmap& bitmap, int x, int y) {
    SkPaint paint;
    paint.setXfermodeMode(SkXfermode::kSrc_Mode);

    SkCanvas canvas(this);
    canvas.drawSprite(bitmap, x, y, &paint);
}

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
                        const SkPaint& paint, const SkMatrix* prePathMatrix,
                        bool pathIsMutable) {
    draw.drawPath(path, paint, prePathMatrix, pathIsMutable);
}

void SkDevice::drawBitmap(const SkDraw& draw, const SkBitmap& bitmap,
                          const SkIRect* srcRect,
                          const SkMatrix& matrix, const SkPaint& paint) {
    SkBitmap        tmp;    // storage if we need a subset of bitmap
    const SkBitmap* bitmapPtr = &bitmap;

    if (srcRect) {
        if (!bitmap.extractSubset(&tmp, *srcRect)) {
            return;     // extraction failed
        }
        bitmapPtr = &tmp;
    }
    draw.drawBitmap(*bitmapPtr, matrix, paint);
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

#ifdef ANDROID
void SkDevice::drawPosTextOnPath(const SkDraw& draw, const void* text, size_t len,
                                     const SkPoint pos[], const SkPaint& paint,
                                     const SkPath& path, const SkMatrix* matrix) {
    draw.drawPosTextOnPath((const char*)text, len, pos, paint, path, matrix);
}
#endif

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

///////////////////////////////////////////////////////////////////////////////

bool SkDevice::filterTextFlags(const SkPaint& paint, TextFlags* flags) {
    if (!paint.isLCDRenderText()) {
        // we're cool with the paint as is
        return false;
    }

    if (SkBitmap::kARGB_8888_Config != fBitmap.config() ||
        paint.getShader() ||
        paint.getXfermode() || // unless its srcover
        paint.getMaskFilter() ||
        paint.getRasterizer() ||
        paint.getColorFilter() ||
        paint.getPathEffect() ||
        paint.isFakeBoldText() ||
        paint.getStyle() != SkPaint::kFill_Style) {
        // turn off lcd
        flags->fFlags = paint.getFlags() & ~SkPaint::kLCDRenderText_Flag;
        flags->fHinting = paint.getHinting();
        return true;
    }
    // we're cool with the paint as is
    return false;
}

///////////////////////////////////////////////////////////////////////////////

SkDevice* SkRasterDeviceFactory::newDevice(SkCanvas* canvas,
                                           SkBitmap::Config config, int width,
                                           int height, bool isOpaque,
                                           bool isForLayer) {
    SkBitmap bitmap;
    bitmap.setConfig(config, width, height);
    bitmap.setIsOpaque(isOpaque);

    return SkNEW_ARGS(SkDevice, (canvas, bitmap, isForLayer));
}
