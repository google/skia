/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkPicture.h"

class SkPrerollCanvas : public SkCanvas {
public:
    SkPrerollCanvas(int width, int height, const SkSurfaceProps* props)
        : SkCanvas(width, height, props)
    {}

protected:
    void onDrawDRRect(const SkRRect&, const SkRRect&, const SkPaint& paint) SK_OVERRIDE {
        this->handlePaint(paint);
    }
    
    void onDrawText(const void*, size_t, SkScalar, SkScalar, const SkPaint& paint) SK_OVERRIDE {
        this->handlePaint(paint);
    }
    
    void onDrawPosText(const void*, size_t, const SkPoint[], const SkPaint& paint) SK_OVERRIDE {
        this->handlePaint(paint);
    }
    
    void onDrawPosTextH(const void*, size_t, const SkScalar[], SkScalar,
                        const SkPaint& paint) SK_OVERRIDE {
        this->handlePaint(paint);
    }
    
    void onDrawTextOnPath(const void*, size_t, const SkPath&, const SkMatrix*,
                          const SkPaint& paint) SK_OVERRIDE {
        this->handlePaint(paint);
    }
    
    void onDrawTextBlob(const SkTextBlob*, SkScalar, SkScalar, const SkPaint& paint) SK_OVERRIDE {
        this->handlePaint(paint);
    }
    
    void onDrawPatch(const SkPoint[12], const SkColor[4], const SkPoint[4], SkXfermode*,
                     const SkPaint& paint) SK_OVERRIDE {
        this->handlePaint(paint);
    }
    
    void onDrawPaint(const SkPaint& paint) SK_OVERRIDE {
        this->handlePaint(paint);
    }

    void onDrawRect(const SkRect&, const SkPaint& paint) SK_OVERRIDE {
        this->handlePaint(paint);
    }

    void onDrawOval(const SkRect&, const SkPaint& paint) SK_OVERRIDE {
        this->handlePaint(paint);
    }

    void onDrawRRect(const SkRRect&, const SkPaint& paint) SK_OVERRIDE {
        this->handlePaint(paint);
    }

    void onDrawPoints(PointMode, size_t, const SkPoint[], const SkPaint& paint) SK_OVERRIDE {
        this->handlePaint(paint);
    }

    void onDrawVertices(VertexMode, int, const SkPoint[], const SkPoint[], const SkColor[],
                        SkXfermode*, const uint16_t[], int, const SkPaint& paint) SK_OVERRIDE {
        this->handlePaint(paint);
    }

    void onDrawPath(const SkPath&, const SkPaint& paint) SK_OVERRIDE {
        this->handlePaint(paint);
    }

    void onDrawImage(const SkImage* image, SkScalar, SkScalar, const SkPaint* paint) SK_OVERRIDE {
        this->handleImage(image);
        if (paint) {
            this->handlePaint(*paint);
        }
    }

    void onDrawImageRect(const SkImage* image, const SkRect*, const SkRect&,
                         const SkPaint* paint) SK_OVERRIDE {
        this->handleImage(image);
        if (paint) {
            this->handlePaint(*paint);
        }
    }

    void onDrawBitmap(const SkBitmap& bm, SkScalar, SkScalar, const SkPaint* paint) SK_OVERRIDE {
        this->handleBitmap(bm);
        if (paint) {
            this->handlePaint(*paint);
        }
    }

    void onDrawBitmapRect(const SkBitmap& bm, const SkRect*, const SkRect&, const SkPaint* paint,
                          DrawBitmapRectFlags) SK_OVERRIDE {
        this->handleBitmap(bm);
        if (paint) {
            this->handlePaint(*paint);
        }
    }

    void onDrawBitmapNine(const SkBitmap& bm, const SkIRect&, const SkRect&,
                          const SkPaint* paint) SK_OVERRIDE {
        this->handleBitmap(bm);
        if (paint) {
            this->handlePaint(*paint);
        }
    }

    void onDrawSprite(const SkBitmap& bm, int, int, const SkPaint* paint) SK_OVERRIDE {
        this->handleBitmap(bm);
        if (paint) {
            this->handlePaint(*paint);
        }
    }

private:
    void handlePaint(const SkPaint& paint) {
        const SkShader* shader = paint.getShader();
        if (shader) {
            shader->preroll();
        }
    }

    void handleImage(const SkImage* image) {
        image->preroll();
    }

    void handleBitmap(const SkBitmap& bitmap) {
        SkBitmap bm(bitmap);
        bm.lockPixels();
    }

    typedef SkCanvas INHERITED;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

void SkPicture::preroll(const SkRect* srcBounds, const SkMatrix* initialMatrix,
                        const SkSurfaceProps* props, void* gpuCacheAccessor) const {
    SkRect bounds = this->cullRect();
    if (srcBounds && !bounds.intersect(*srcBounds)) {
        return;
    }

    const SkIRect ibounds = bounds.roundOut();
    if (ibounds.isEmpty()) {
        return;
    }

    SkPrerollCanvas canvas(ibounds.width(), ibounds.height(), props);

    canvas.translate(-SkIntToScalar(ibounds.left()), -SkIntToScalar(ibounds.top()));
    canvas.drawPicture(this, initialMatrix, NULL);
}

