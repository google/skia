/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPictureUtils.h"
#include "SkDevice.h"
#include "SkDraw.h"
#include "SkPaintPriv.h"
#include "SkPictureData.h"
#include "SkPicturePlayback.h"

SkPicture::AccelData::Key GPUAccelData::ComputeAccelDataKey() {
    static const SkPicture::AccelData::Key gGPUID = SkPicture::AccelData::GenerateDomain();

    return gGPUID;
}

// The GrGather device performs GPU-backend-specific preprocessing on
// a picture. The results are stored in a GPUAccelData.
//
// Currently the only interesting work is done in drawDevice (i.e., when a
// saveLayer is collapsed back into its parent) and, maybe, in onCreateDevice.
// All the current work could be done much more efficiently by just traversing the
// raw op codes in the SkPicture (although we would still need to replay all the
// clip calls).
class GrGatherDevice : public SkBaseDevice {
public:
    SK_DECLARE_INST_COUNT(GrGatherDevice)

    GrGatherDevice(int width, int height, SkPicturePlayback* playback, GPUAccelData* accelData,
                   int saveLayerDepth) {
        fPlayback = playback;
        fSaveLayerDepth = saveLayerDepth;
        fInfo.fValid = true;
        fInfo.fSize.set(width, height);
        fInfo.fPaint = NULL;
        fInfo.fSaveLayerOpID = fPlayback->curOpID();
        fInfo.fRestoreOpID = 0;
        fInfo.fHasNestedLayers = false;
        fInfo.fIsNested = (2 == fSaveLayerDepth);

        fEmptyBitmap.setInfo(SkImageInfo::MakeUnknown(fInfo.fSize.fWidth, fInfo.fSize.fHeight));
        fAccelData = accelData;
        fAlreadyDrawn = false;
    }

    virtual ~GrGatherDevice() { }

    virtual SkImageInfo imageInfo() const SK_OVERRIDE {
        return fEmptyBitmap.info();
    }

#ifdef SK_SUPPORT_LEGACY_WRITEPIXELSCONFIG
    virtual void writePixels(const SkBitmap& bitmap, int x, int y,
                             SkCanvas::Config8888 config8888) SK_OVERRIDE {
        NotSupported();
    }
#endif
    virtual GrRenderTarget* accessRenderTarget() SK_OVERRIDE { return NULL; }

protected:
    virtual bool filterTextFlags(const SkPaint& paint, TextFlags*) SK_OVERRIDE {
        return false;
    }
    virtual void clear(SkColor color) SK_OVERRIDE {
        NothingToDo();
    }
    virtual void drawPaint(const SkDraw& draw, const SkPaint& paint) SK_OVERRIDE {
    }
    virtual void drawPoints(const SkDraw& draw, SkCanvas::PointMode mode, size_t count,
                            const SkPoint points[], const SkPaint& paint) SK_OVERRIDE {
    }
    virtual void drawRect(const SkDraw& draw, const SkRect& rect,
                          const SkPaint& paint) SK_OVERRIDE {
    }
    virtual void drawOval(const SkDraw& draw, const SkRect& rect,
                          const SkPaint& paint) SK_OVERRIDE {
    }
    virtual void drawRRect(const SkDraw& draw, const SkRRect& rrect,
                           const SkPaint& paint) SK_OVERRIDE {
    }
    virtual void drawPath(const SkDraw& draw, const SkPath& path,
                          const SkPaint& paint, const SkMatrix* prePathMatrix,
                          bool pathIsMutable) SK_OVERRIDE {
    }
    virtual void drawBitmap(const SkDraw& draw, const SkBitmap& bitmap,
                            const SkMatrix& matrix, const SkPaint& paint) SK_OVERRIDE {
    }
    virtual void drawSprite(const SkDraw&, const SkBitmap& bitmap,
                            int x, int y, const SkPaint& paint) SK_OVERRIDE {
    }
    virtual void drawBitmapRect(const SkDraw& draw, const SkBitmap& bitmap,
                                const SkRect* srcOrNull, const SkRect& dst,
                                const SkPaint& paint,
                                SkCanvas::DrawBitmapRectFlags flags) SK_OVERRIDE {
    }
    virtual void drawText(const SkDraw& draw, const void* text, size_t len,
                          SkScalar x, SkScalar y,
                          const SkPaint& paint) SK_OVERRIDE {
    }
    virtual void drawPosText(const SkDraw& draw, const void* text, size_t len,
                             const SkScalar pos[], SkScalar constY,
                             int scalarsPerPos, const SkPaint& paint) SK_OVERRIDE {
    }
    virtual void drawTextOnPath(const SkDraw& draw, const void* text, size_t len,
                                const SkPath& path, const SkMatrix* matrix,
                                const SkPaint& paint) SK_OVERRIDE {
    }
    virtual void drawVertices(const SkDraw& draw, SkCanvas::VertexMode, int vertexCount,
                              const SkPoint verts[], const SkPoint texs[],
                              const SkColor colors[], SkXfermode* xmode,
                              const uint16_t indices[], int indexCount,
                              const SkPaint& paint) SK_OVERRIDE {
    }
    virtual void drawDevice(const SkDraw& draw, SkBaseDevice* deviceIn, int x, int y,
                            const SkPaint& paint) SK_OVERRIDE {
        // deviceIn is the one that is being "restored" back to its parent
        GrGatherDevice* device = static_cast<GrGatherDevice*>(deviceIn);

        if (device->fAlreadyDrawn) {
            return;
        }

        device->fInfo.fRestoreOpID = fPlayback->curOpID();
        device->fInfo.fCTM = *draw.fMatrix;
        device->fInfo.fCTM.postTranslate(SkIntToScalar(-device->getOrigin().fX),
                                         SkIntToScalar(-device->getOrigin().fY));

        device->fInfo.fOffset = device->getOrigin();

        if (NeedsDeepCopy(paint)) {
            // This NULL acts as a signal that the paint was uncopyable (for now)
            device->fInfo.fPaint = NULL;
            device->fInfo.fValid = false;
        } else {
            device->fInfo.fPaint = SkNEW_ARGS(SkPaint, (paint));
        }

        fAccelData->addSaveLayerInfo(device->fInfo);
        device->fAlreadyDrawn = true;
    }
    // TODO: allow this call to return failure, or move to SkBitmapDevice only.
    virtual const SkBitmap& onAccessBitmap() SK_OVERRIDE {
        return fEmptyBitmap;
    }
#ifdef SK_SUPPORT_LEGACY_READPIXELSCONFIG
    virtual bool onReadPixels(const SkBitmap& bitmap,
                              int x, int y,
                              SkCanvas::Config8888 config8888) SK_OVERRIDE {
        NotSupported();
        return false;
    }
#endif
    virtual void lockPixels() SK_OVERRIDE { NothingToDo(); }
    virtual void unlockPixels() SK_OVERRIDE { NothingToDo(); }
    virtual bool allowImageFilter(const SkImageFilter*) SK_OVERRIDE { return false; }
    virtual bool canHandleImageFilter(const SkImageFilter*) SK_OVERRIDE { return false; }
    virtual bool filterImage(const SkImageFilter*, const SkBitmap&, const SkImageFilter::Context&,
                             SkBitmap* result, SkIPoint* offset) SK_OVERRIDE {
        return false;
    }

private:
    // The playback object driving this rendering
    SkPicturePlayback *fPlayback;

    SkBitmap fEmptyBitmap; // legacy -- need to remove

    // All information gathered during the gather process is stored here
    GPUAccelData* fAccelData;

    // true if this device has already been drawn back to its parent(s) at least
    // once.
    bool   fAlreadyDrawn;

    // The information regarding the saveLayer call this device represents.
    GPUAccelData::SaveLayerInfo fInfo;

    // The depth of this device in the saveLayer stack
    int fSaveLayerDepth;

    virtual void replaceBitmapBackendForRasterSurface(const SkBitmap&) SK_OVERRIDE {
        NotSupported();
    }

    virtual SkBaseDevice* onCreateDevice(const SkImageInfo& info, Usage usage) SK_OVERRIDE {
        // we expect to only get called via savelayer, in which case it is fine.
        SkASSERT(kSaveLayer_Usage == usage);

        fInfo.fHasNestedLayers = true;
        return SkNEW_ARGS(GrGatherDevice, (info.width(), info.height(), fPlayback,
                                           fAccelData, fSaveLayerDepth+1));
    }

    virtual void flush() SK_OVERRIDE {}

    static void NotSupported() {
        SkDEBUGFAIL("this method should never be called");
    }

    static void NothingToDo() {}

    typedef SkBaseDevice INHERITED;
};

// The GrGatherCanvas allows saveLayers but simplifies clipping. It is really
// only intended to be used as:
//
//      GrGatherDevice dev(w, h, picture, accelData);
//      GrGatherCanvas canvas(..., picture);
//      canvas.gather();
//
// which is all just to fill in 'accelData'
class SK_API GrGatherCanvas : public SkCanvas {
public:
    GrGatherCanvas(GrGatherDevice* device) : INHERITED(device) {}

protected:
    // disable aa for speed
    virtual void onClipRect(const SkRect& rect, SkRegion::Op op, ClipEdgeStyle) SK_OVERRIDE {
        this->INHERITED::onClipRect(rect, op, kHard_ClipEdgeStyle);
    }

    // for speed, just respect the bounds, and disable AA. May give us a few
    // false positives and negatives.
    virtual void onClipPath(const SkPath& path, SkRegion::Op op, ClipEdgeStyle) SK_OVERRIDE {
        this->updateClipConservativelyUsingBounds(path.getBounds(), op,
                                                  path.isInverseFillType());
    }
    virtual void onClipRRect(const SkRRect& rrect, SkRegion::Op op, ClipEdgeStyle) SK_OVERRIDE {
        this->updateClipConservativelyUsingBounds(rrect.getBounds(), op, false);
    }

    virtual void onDrawPicture(const SkPicture* picture) SK_OVERRIDE {
        if (NULL != picture->fData.get()) {
            // Disable the BBH for the old path so all the draw calls
            // will be seen. The stock SkPicture::draw method can't be
            // invoked since it just uses a vanilla SkPicturePlayback.
            SkPicturePlayback playback(picture);
            playback.setUseBBH(false);
            playback.draw(this, NULL);
        } else {
            // Since we know this is the SkRecord path we can just call
            // SkPicture::draw.
            picture->draw(this);
        }
    }

private:
    typedef SkCanvas INHERITED;
};

// GatherGPUInfo is only intended to be called within the context of SkGpuDevice's
// EXPERIMENTAL_optimize method.
void GatherGPUInfo(const SkPicture* pict, GPUAccelData* accelData) {
    if (NULL == pict || 0 == pict->width() || 0 == pict->height()) {
        return ;
    }

    // BBH-based rendering doesn't re-issue many of the operations the gather
    // process cares about (e.g., saves and restores) so it must be disabled.
    SkPicturePlayback playback(pict);
    playback.setUseBBH(false);

    GrGatherDevice device(pict->width(), pict->height(), &playback, accelData, 0);
    GrGatherCanvas canvas(&device);

    canvas.clipRect(SkRect::MakeWH(SkIntToScalar(pict->width()),
                                   SkIntToScalar(pict->height())),
                    SkRegion::kIntersect_Op, false);
    playback.draw(&canvas, NULL);
}
