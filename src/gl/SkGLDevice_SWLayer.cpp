#include "SkGLDevice_SWLayer.h"
#include "SkRegion.h"

SkGLDevice_SWLayer::SkGLDevice_SWLayer(const SkBitmap& bitmap)
        : SkGLDevice(bitmap, true) {
    fTextureID = 0;

    SkASSERT(bitmap.getPixels());
}

SkGLDevice_SWLayer::~SkGLDevice_SWLayer() {
    if (fTextureID) {
        glDeleteTextures(1, &fTextureID);
    }
}

SkGLDevice::TexOrientation SkGLDevice_SWLayer::bindDeviceAsTexture() {
    const SkBitmap& bitmap = this->accessBitmap(false);

    if (0 == fTextureID) {
        fTextureID = SkGL::BindNewTexture(bitmap, NULL);
    }
    return kTopToBottom_TexOrientation;
}

///////////////////////////////////////////////////////////////////////////////

#include "SkDraw.h"

void SkGLDevice_SWLayer::drawPaint(const SkDraw& draw, const SkPaint& paint) {
    draw.drawPaint(paint);
}

void SkGLDevice_SWLayer::drawPoints(const SkDraw& draw, SkCanvas::PointMode mode, size_t count,
                          const SkPoint pts[], const SkPaint& paint) {
    draw.drawPoints(mode, count, pts, paint);
}

void SkGLDevice_SWLayer::drawRect(const SkDraw& draw, const SkRect& r,
                        const SkPaint& paint) {
    draw.drawRect(r, paint);
}

void SkGLDevice_SWLayer::drawPath(const SkDraw& draw, const SkPath& path,
                        const SkPaint& paint) {
    draw.drawPath(path, paint);
}

void SkGLDevice_SWLayer::drawBitmap(const SkDraw& draw, const SkBitmap& bitmap,
                          const SkMatrix& matrix, const SkPaint& paint) {
    draw.drawBitmap(bitmap, matrix, paint);
}

void SkGLDevice_SWLayer::drawSprite(const SkDraw& draw, const SkBitmap& bitmap,
                          int x, int y, const SkPaint& paint) {
    draw.drawSprite(bitmap, x, y, paint);
}

void SkGLDevice_SWLayer::drawText(const SkDraw& draw, const void* text, size_t len,
                        SkScalar x, SkScalar y, const SkPaint& paint) {
    draw.drawText((const char*)text, len, x, y, paint);
}

void SkGLDevice_SWLayer::drawPosText(const SkDraw& draw, const void* text, size_t len,
                           const SkScalar xpos[], SkScalar y,
                           int scalarsPerPos, const SkPaint& paint) {
    draw.drawPosText((const char*)text, len, xpos, y, scalarsPerPos, paint);
}

void SkGLDevice_SWLayer::drawTextOnPath(const SkDraw& draw, const void* text,
                              size_t len, const SkPath& path,
                              const SkMatrix* matrix,
                              const SkPaint& paint) {
    draw.drawTextOnPath((const char*)text, len, path, matrix, paint);
}

void SkGLDevice_SWLayer::drawVertices(const SkDraw& draw, SkCanvas::VertexMode vmode,
                            int vertexCount,
                            const SkPoint verts[], const SkPoint textures[],
                            const SkColor colors[], SkXfermode* xmode,
                            const uint16_t indices[], int indexCount,
                            const SkPaint& paint) {
    draw.drawVertices(vmode, vertexCount, verts, textures, colors, xmode,
                      indices, indexCount, paint);
}

void SkGLDevice_SWLayer::drawDevice(const SkDraw& draw, SkDevice* dev,
                                    int x, int y, const SkPaint& paint) {
    this->SkDevice::drawDevice(draw, dev, x, y, paint);
}

