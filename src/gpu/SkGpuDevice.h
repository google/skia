
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGpuDevice_DEFINED
#define SkGpuDevice_DEFINED

#include "SkGr.h"
#include "SkBitmap.h"
#include "SkDevice.h"
#include "SkPicture.h"
#include "SkRegion.h"
#include "SkSurface.h"
#include "GrContext.h"
#include "GrSurfacePriv.h"

struct SkDrawProcs;
struct GrSkDrawProcs;

class GrAccelData;
struct GrCachedLayer;
class GrTextContext;

/**
 *  Subclass of SkBaseDevice, which directs all drawing to the GrGpu owned by the
 *  canvas.
 */
class SK_API SkGpuDevice : public SkBaseDevice {
public:
    enum Flags {
        kNeedClear_Flag = 1 << 0,  //!< Surface requires an initial clear
    };

    /**
     * Creates an SkGpuDevice from a GrRenderTarget.
     */
    static SkGpuDevice* Create(GrRenderTarget* target, const SkSurfaceProps*, unsigned flags = 0);

    /**
     * New device that will create an offscreen renderTarget based on the ImageInfo and
     * sampleCount. The Budgeted param controls whether the device's backing store counts against
     * the resource cache budget. On failure, returns NULL.
     */
    static SkGpuDevice* Create(GrContext*, SkSurface::Budgeted, const SkImageInfo&,
                               int sampleCount, const SkSurfaceProps*, unsigned flags = 0);

    virtual ~SkGpuDevice();

    SkGpuDevice* cloneDevice(const SkSurfaceProps& props) {
        SkBaseDevice* dev = this->onCreateCompatibleDevice(CreateInfo(this->imageInfo(),
                                                                      kGeneral_Usage,
                                                                      props.pixelGeometry()));
        return static_cast<SkGpuDevice*>(dev);
    }

    GrContext* context() const { return fContext; }

    // set all pixels to 0
    void clearAll();

    void replaceRenderTarget(bool shouldRetainContent);

    GrRenderTarget* accessRenderTarget() SK_OVERRIDE;

    SkImageInfo imageInfo() const SK_OVERRIDE {
        return fRenderTarget ? fRenderTarget->surfacePriv().info() : SkImageInfo::MakeUnknown();
    }

    const SkSurfaceProps& surfaceProps() const { return fSurfaceProps; }

    void drawPaint(const SkDraw&, const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPoints(const SkDraw&, SkCanvas::PointMode mode, size_t count,
                            const SkPoint[], const SkPaint& paint) SK_OVERRIDE;
    virtual void drawRect(const SkDraw&, const SkRect& r,
                          const SkPaint& paint) SK_OVERRIDE;
    virtual void drawRRect(const SkDraw&, const SkRRect& r,
                           const SkPaint& paint) SK_OVERRIDE;
    virtual void drawDRRect(const SkDraw& draw, const SkRRect& outer,
                            const SkRRect& inner, const SkPaint& paint) SK_OVERRIDE;
    virtual void drawOval(const SkDraw&, const SkRect& oval,
                          const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPath(const SkDraw&, const SkPath& path,
                          const SkPaint& paint, const SkMatrix* prePathMatrix,
                          bool pathIsMutable) SK_OVERRIDE;
    virtual void drawBitmap(const SkDraw&, const SkBitmap& bitmap,
                            const SkMatrix&, const SkPaint&) SK_OVERRIDE;
    virtual void drawBitmapRect(const SkDraw&, const SkBitmap&,
                                const SkRect* srcOrNull, const SkRect& dst,
                                const SkPaint& paint,
                                SkCanvas::DrawBitmapRectFlags flags) SK_OVERRIDE;
    virtual void drawSprite(const SkDraw&, const SkBitmap& bitmap,
                            int x, int y, const SkPaint& paint) SK_OVERRIDE;
    virtual void drawText(const SkDraw&, const void* text, size_t len,
                          SkScalar x, SkScalar y, const SkPaint&) SK_OVERRIDE;
    virtual void drawPosText(const SkDraw&, const void* text, size_t len,
                             const SkScalar pos[], int scalarsPerPos,
                             const SkPoint& offset, const SkPaint&) SK_OVERRIDE;
    virtual void drawVertices(const SkDraw&, SkCanvas::VertexMode, int vertexCount,
                              const SkPoint verts[], const SkPoint texs[],
                              const SkColor colors[], SkXfermode* xmode,
                              const uint16_t indices[], int indexCount,
                              const SkPaint&) SK_OVERRIDE;
    virtual void drawDevice(const SkDraw&, SkBaseDevice*, int x, int y,
                            const SkPaint&) SK_OVERRIDE;

    void flush() SK_OVERRIDE;

    void onAttachToCanvas(SkCanvas* canvas) SK_OVERRIDE;
    void onDetachFromCanvas() SK_OVERRIDE;

    const SkBitmap& onAccessBitmap() SK_OVERRIDE;

    bool canHandleImageFilter(const SkImageFilter*) SK_OVERRIDE;
    virtual bool filterImage(const SkImageFilter*, const SkBitmap&,
                             const SkImageFilter::Context&,
                             SkBitmap*, SkIPoint*) SK_OVERRIDE;

    bool filterTexture(GrContext*, GrTexture*, const SkImageFilter*,
                       const SkImageFilter::Context&,
                       SkBitmap* result, SkIPoint* offset);

protected:
    bool onReadPixels(const SkImageInfo&, void*, size_t, int, int) SK_OVERRIDE;
    bool onWritePixels(const SkImageInfo&, const void*, size_t, int, int) SK_OVERRIDE;
    bool onShouldDisableLCD(const SkPaint&) const SK_OVERRIDE;

    /**  PRIVATE / EXPERIMENTAL -- do not call */
    virtual bool EXPERIMENTAL_drawPicture(SkCanvas* canvas, const SkPicture* picture,
                                          const SkMatrix*, const SkPaint*) SK_OVERRIDE;

private:
    GrContext*                      fContext;
    GrSkDrawProcs*                  fDrawProcs;
    SkAutoTUnref<const SkClipStack> fClipStack;
    SkIPoint                        fClipOrigin;
    GrClip                          fClip;
    GrTextContext*                  fTextContext;
    SkSurfaceProps                  fSurfaceProps;
    GrRenderTarget*                 fRenderTarget;
    // remove when our clients don't rely on accessBitmap()
    SkBitmap                        fLegacyBitmap;
    bool                            fNeedClear;

    SkGpuDevice(GrRenderTarget*, const SkSurfaceProps*, unsigned flags);

    SkBaseDevice* onCreateCompatibleDevice(const CreateInfo&) SK_OVERRIDE;

    SkSurface* newSurface(const SkImageInfo&, const SkSurfaceProps&) SK_OVERRIDE;

    SkImageFilter::Cache* getImageFilterCache() SK_OVERRIDE;

    bool forceConservativeRasterClip() const SK_OVERRIDE { return true; }

    // sets the render target and clip on context
    void prepareDraw(const SkDraw&);

    /**
     * Implementation for both drawBitmap and drawBitmapRect.
     */
    void drawBitmapCommon(const SkDraw&,
                          const SkBitmap& bitmap,
                          const SkRect* srcRectPtr,
                          const SkSize* dstSizePtr,      // ignored iff srcRectPtr == NULL
                          const SkPaint&,
                          SkCanvas::DrawBitmapRectFlags flags);

    /**
     * Helper functions called by drawBitmapCommon. By the time these are called the SkDraw's
     * matrix, clip, and the device's render target has already been set on GrContext.
     */

    // The tileSize and clippedSrcRect will be valid only if true is returned.
    bool shouldTileBitmap(const SkBitmap& bitmap,
                          const SkMatrix& viewMatrix,
                          const GrTextureParams& sampler,
                          const SkRect* srcRectPtr,
                          int maxTileSize,
                          int* tileSize,
                          SkIRect* clippedSrcRect) const;
    void internalDrawBitmap(const SkBitmap&,
                            const SkMatrix& viewMatrix,
                            const SkRect&,
                            const GrTextureParams& params,
                            const SkPaint& paint,
                            SkCanvas::DrawBitmapRectFlags flags,
                            bool bicubic,
                            bool needsTextureDomain);
    void drawTiledBitmap(const SkBitmap& bitmap,
                         const SkMatrix& viewMatrix,
                         const SkRect& srcRect,
                         const SkIRect& clippedSrcRect,
                         const GrTextureParams& params,
                         const SkPaint& paint,
                         SkCanvas::DrawBitmapRectFlags flags,
                         int tileSize,
                         bool bicubic);

    bool drawDashLine(const SkPoint pts[2], const SkPaint& paint);

    static SkPicture::AccelData::Key ComputeAccelDataKey();

    static GrRenderTarget* CreateRenderTarget(GrContext*, SkSurface::Budgeted, const SkImageInfo&,
                                              int sampleCount);

    typedef SkBaseDevice INHERITED;
};

#endif
