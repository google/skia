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
 *  behind its device.
 */
class GatherPixelRefDevice : public SkBaseDevice {
public:
    GatherPixelRefDevice(int width, int height, PixelRefSet* prset) {
        fSize.set(width, height);
        fEmptyBitmap.setConfig(SkBitmap::kNo_Config, width, height);
        fPRSet = prset;
    }

    virtual uint32_t getDeviceCapabilities() SK_OVERRIDE { return 0; }
    virtual int width() const SK_OVERRIDE { return fSize.width(); }
    virtual int height() const SK_OVERRIDE { return fSize.height(); }
    virtual bool isOpaque() const SK_OVERRIDE { return false; }
    virtual SkBitmap::Config config() const SK_OVERRIDE {
        return SkBitmap::kNo_Config;
    }
    virtual GrRenderTarget* accessRenderTarget() SK_OVERRIDE { return NULL; }
    virtual bool filterTextFlags(const SkPaint& paint, TextFlags*) SK_OVERRIDE {
        return true;
    }
    // TODO: allow this call to return failure, or move to SkBitmapDevice only.
    virtual const SkBitmap& onAccessBitmap() SK_OVERRIDE {
        return fEmptyBitmap;
    }
    virtual void lockPixels() SK_OVERRIDE { nothing_to_do(); }
    virtual void unlockPixels() SK_OVERRIDE { nothing_to_do(); }
    virtual bool allowImageFilter(SkImageFilter*) SK_OVERRIDE { return false; }
    virtual bool canHandleImageFilter(SkImageFilter*) SK_OVERRIDE { return false; }
    virtual bool filterImage(SkImageFilter*, const SkBitmap&, const SkMatrix&,
                             SkBitmap* result, SkIPoint* offset) SK_OVERRIDE {
        return false;
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
    virtual void drawRRect(const SkDraw&, const SkRRect&,
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

    virtual void replaceBitmapBackendForRasterSurface(const SkBitmap&) SK_OVERRIDE {
        not_supported();
    }
    virtual SkBaseDevice* onCreateCompatibleDevice(SkBitmap::Config config,
                                                   int width, int height,
                                                   bool isOpaque,
                                                   Usage usage) SK_OVERRIDE {
        // we expect to only get called via savelayer, in which case it is fine.
        SkASSERT(kSaveLayer_Usage == usage);
        return SkNEW_ARGS(GatherPixelRefDevice, (width, height, fPRSet));
    }
    virtual void flush() SK_OVERRIDE {}

private:
    PixelRefSet*  fPRSet;
    SkBitmap fEmptyBitmap;  // legacy -- need to remove the need for this guy
    SkISize fSize;

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

    typedef SkBaseDevice INHERITED;
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

    GatherPixelRefDevice device(pict->width(), pict->height(), &prset);
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
