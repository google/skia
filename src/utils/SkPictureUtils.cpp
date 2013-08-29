/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapDevice.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkPictureUtils.h"
#include "SkPixelRef.h"
#include "SkRRect.h"
#include "SkShader.h"

class PixelRefSet {
public:
    PixelRefSet(SkTDArray<SkPixelRef*>* array) : fArray(array) {}

    // This does a linear search on existing pixelrefs, so if this list gets big
    // we should use a more complex sorted/hashy thing.
    //
    void add(SkPixelRef* pr) {
        uint32_t genID = pr->getGenerationID();
        if (fGenID.find(genID) < 0) {
            *fArray->append() = pr;
            *fGenID.append() = genID;
//            SkDebugf("--- adding [%d] %x %d\n", fArray->count() - 1, pr, genID);
        } else {
//            SkDebugf("--- already have %x %d\n", pr, genID);
        }
    }

private:
    SkTDArray<SkPixelRef*>* fArray;
    SkTDArray<uint32_t>     fGenID;
};

static void not_supported() {
    SkDEBUGFAIL("this method should never be called");
}

static void nothing_to_do() {}

/**
 *  This device will route all bitmaps (primitives and in shaders) to its PRSet.
 *  It should never actually draw anything, so there need not be any pixels
 *  behind its device-bitmap.
 */
class GatherPixelRefDevice : public SkBitmapDevice {
private:
    PixelRefSet*  fPRSet;

    void addBitmap(const SkBitmap& bm) {
        fPRSet->add(bm.pixelRef());
    }

    void addBitmapFromPaint(const SkPaint& paint) {
        SkShader* shader = paint.getShader();
        if (shader) {
            SkBitmap bm;
            // Check whether the shader is a gradient in order to short-circuit
            // call to asABitmap to prevent generation of bitmaps from
            // gradient shaders, which implement asABitmap.
            if (SkShader::kNone_GradientType == shader->asAGradient(NULL) &&
                shader->asABitmap(&bm, NULL, NULL)) {
                fPRSet->add(bm.pixelRef());
            }
        }
    }

public:
    GatherPixelRefDevice(const SkBitmap& bm, PixelRefSet* prset) : SkBitmapDevice(bm) {
        fPRSet = prset;
    }

    virtual void clear(SkColor color) SK_OVERRIDE {
        nothing_to_do();
    }
    virtual void writePixels(const SkBitmap& bitmap, int x, int y,
                             SkCanvas::Config8888 config8888) SK_OVERRIDE {
        not_supported();
    }

    virtual void drawPaint(const SkDraw&, const SkPaint& paint) SK_OVERRIDE {
        this->addBitmapFromPaint(paint);
    }
    virtual void drawPoints(const SkDraw&, SkCanvas::PointMode mode, size_t count,
                            const SkPoint[], const SkPaint& paint) SK_OVERRIDE {
        this->addBitmapFromPaint(paint);
    }
    virtual void drawRect(const SkDraw&, const SkRect&,
                          const SkPaint& paint) SK_OVERRIDE {
        this->addBitmapFromPaint(paint);
    }
    virtual void drawOval(const SkDraw&, const SkRect&,
                          const SkPaint& paint) SK_OVERRIDE {
        this->addBitmapFromPaint(paint);
    }
    virtual void drawPath(const SkDraw&, const SkPath& path,
                          const SkPaint& paint, const SkMatrix* prePathMatrix,
                          bool pathIsMutable) SK_OVERRIDE {
        this->addBitmapFromPaint(paint);
    }
    virtual void drawBitmap(const SkDraw&, const SkBitmap& bitmap,
                            const SkMatrix&, const SkPaint&) SK_OVERRIDE {
        this->addBitmap(bitmap);
    }
    virtual void drawBitmapRect(const SkDraw&, const SkBitmap& bitmap,
                                const SkRect* srcOrNull, const SkRect& dst,
                                const SkPaint&,
                                SkCanvas::DrawBitmapRectFlags flags) SK_OVERRIDE {
        this->addBitmap(bitmap);
    }
    virtual void drawSprite(const SkDraw&, const SkBitmap& bitmap,
                            int x, int y, const SkPaint& paint) SK_OVERRIDE {
        this->addBitmap(bitmap);
    }
    virtual void drawText(const SkDraw&, const void* text, size_t len,
                          SkScalar x, SkScalar y,
                          const SkPaint& paint) SK_OVERRIDE {
        this->addBitmapFromPaint(paint);
    }
    virtual void drawPosText(const SkDraw&, const void* text, size_t len,
                             const SkScalar pos[], SkScalar constY,
                             int, const SkPaint& paint) SK_OVERRIDE {
        this->addBitmapFromPaint(paint);
    }
    virtual void drawTextOnPath(const SkDraw&, const void* text, size_t len,
                                const SkPath& path, const SkMatrix* matrix,
                                const SkPaint& paint) SK_OVERRIDE {
        this->addBitmapFromPaint(paint);
    }
    virtual void drawVertices(const SkDraw&, SkCanvas::VertexMode, int vertexCount,
                              const SkPoint verts[], const SkPoint texs[],
                              const SkColor colors[], SkXfermode* xmode,
                              const uint16_t indices[], int indexCount,
                              const SkPaint& paint) SK_OVERRIDE {
        this->addBitmapFromPaint(paint);
    }
    virtual void drawDevice(const SkDraw&, SkBaseDevice*, int x, int y,
                            const SkPaint&) SK_OVERRIDE {
        nothing_to_do();
    }

protected:
    virtual bool onReadPixels(const SkBitmap& bitmap,
                              int x, int y,
                              SkCanvas::Config8888 config8888) SK_OVERRIDE {
        not_supported();
        return false;
    }

private:
    typedef SkBitmapDevice INHERITED;
};

class NoSaveLayerCanvas : public SkCanvas {
public:
    NoSaveLayerCanvas(SkBaseDevice* device) : INHERITED(device) {}

    // turn saveLayer() into save() for speed, should not affect correctness.
    virtual int saveLayer(const SkRect* bounds, const SkPaint* paint,
                          SaveFlags flags) SK_OVERRIDE {

        // Like SkPictureRecord, we don't want to create layers, but we do need
        // to respect the save and (possibly) its rect-clip.

        int count = this->INHERITED::save(flags);
        if (bounds) {
            this->INHERITED::clipRectBounds(bounds, flags, NULL);
        }
        return count;
    }

    // disable aa for speed
    virtual bool clipRect(const SkRect& rect, SkRegion::Op op,
                          bool doAA) SK_OVERRIDE {
        return this->INHERITED::clipRect(rect, op, false);
    }

    // for speed, just respect the bounds, and disable AA. May give us a few
    // false positives and negatives.
    virtual bool clipPath(const SkPath& path, SkRegion::Op op,
                          bool doAA) SK_OVERRIDE {
        return this->updateClipConservativelyUsingBounds(path.getBounds(), op, path.isInverseFillType());
    }
    virtual bool clipRRect(const SkRRect& rrect, SkRegion::Op op,
                           bool doAA) SK_OVERRIDE {
        return this->updateClipConservativelyUsingBounds(rrect.getBounds(), op, false);
    }

private:
    typedef SkCanvas INHERITED;
};

SkData* SkPictureUtils::GatherPixelRefs(SkPicture* pict, const SkRect& area) {
    if (NULL == pict) {
        return NULL;
    }

    // this test also handles if either area or pict's width/height are empty
    if (!SkRect::Intersects(area,
                            SkRect::MakeWH(SkIntToScalar(pict->width()),
                                           SkIntToScalar(pict->height())))) {
        return NULL;
    }

    SkTDArray<SkPixelRef*> array;
    PixelRefSet prset(&array);

    SkBitmap emptyBitmap;
    emptyBitmap.setConfig(SkBitmap::kARGB_8888_Config, pict->width(), pict->height());
    // note: we do not set any pixels (shouldn't need to)

    GatherPixelRefDevice device(emptyBitmap, &prset);
    NoSaveLayerCanvas canvas(&device);

    canvas.clipRect(area, SkRegion::kIntersect_Op, false);
    canvas.drawPicture(*pict);

    SkData* data = NULL;
    int count = array.count();
    if (count > 0) {
        data = SkData::NewFromMalloc(array.detach(), count * sizeof(SkPixelRef*));
    }
    return data;
}
