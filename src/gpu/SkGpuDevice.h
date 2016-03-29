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
#include "GrDrawContext.h"
#include "GrContext.h"
#include "GrSurfacePriv.h"
#include "GrTypes.h"

class GrAccelData;
class GrTextureProducer;
struct GrCachedLayer;

/**
 *  Subclass of SkBaseDevice, which directs all drawing to the GrGpu owned by the
 *  canvas.
 */
class SK_API SkGpuDevice : public SkBaseDevice {
public:
    enum InitContents {
        kClear_InitContents,
        kUninit_InitContents
    };

    /**
     * Creates an SkGpuDevice from a GrRenderTarget.
     */
    static SkGpuDevice* Create(GrRenderTarget* target, const SkSurfaceProps*, InitContents);

    /**
     * Creates an SkGpuDevice from a GrRenderTarget whose texture width/height is
     * different than its actual width/height (e.g., approx-match scratch texture).
     */
    static SkGpuDevice* Create(GrRenderTarget* target, int width, int height,
                               const SkSurfaceProps*, InitContents);

    /**
     * New device that will create an offscreen renderTarget based on the ImageInfo and
     * sampleCount. The Budgeted param controls whether the device's backing store counts against
     * the resource cache budget. On failure, returns nullptr.
     */
    static SkGpuDevice* Create(GrContext*, SkBudgeted, const SkImageInfo&,
                               int sampleCount, const SkSurfaceProps*,
                               InitContents, GrTextureStorageAllocator = GrTextureStorageAllocator());

    ~SkGpuDevice() override {}

    SkGpuDevice* cloneDevice(const SkSurfaceProps& props) {
        SkBaseDevice* dev = this->onCreateDevice(CreateInfo(this->imageInfo(), kPossible_TileUsage,
                                                            props.pixelGeometry()),
                                                 nullptr);
        return static_cast<SkGpuDevice*>(dev);
    }

    GrContext* context() const { return fContext; }

    // set all pixels to 0
    void clearAll();

    void replaceRenderTarget(bool shouldRetainContent);

    GrRenderTarget* accessRenderTarget() override;

    SkImageInfo imageInfo() const override {
        return fLegacyBitmap.info();
    }

    void drawPaint(const SkDraw&, const SkPaint& paint) override;
    virtual void drawPoints(const SkDraw&, SkCanvas::PointMode mode, size_t count,
                            const SkPoint[], const SkPaint& paint) override;
    virtual void drawRect(const SkDraw&, const SkRect& r,
                          const SkPaint& paint) override;
    virtual void drawRRect(const SkDraw&, const SkRRect& r,
                           const SkPaint& paint) override;
    virtual void drawDRRect(const SkDraw& draw, const SkRRect& outer,
                            const SkRRect& inner, const SkPaint& paint) override;
    virtual void drawOval(const SkDraw&, const SkRect& oval,
                          const SkPaint& paint) override;
    virtual void drawPath(const SkDraw&, const SkPath& path,
                          const SkPaint& paint, const SkMatrix* prePathMatrix,
                          bool pathIsMutable) override;
    virtual void drawBitmap(const SkDraw&, const SkBitmap& bitmap,
                            const SkMatrix&, const SkPaint&) override;
    virtual void drawBitmapRect(const SkDraw&, const SkBitmap&,
                                const SkRect* srcOrNull, const SkRect& dst,
                                const SkPaint& paint, SkCanvas::SrcRectConstraint) override;
    virtual void drawSprite(const SkDraw&, const SkBitmap& bitmap,
                            int x, int y, const SkPaint& paint) override;
    virtual void drawText(const SkDraw&, const void* text, size_t len,
                          SkScalar x, SkScalar y, const SkPaint&) override;
    virtual void drawPosText(const SkDraw&, const void* text, size_t len,
                             const SkScalar pos[], int scalarsPerPos,
                             const SkPoint& offset, const SkPaint&) override;
    virtual void drawTextBlob(const SkDraw&, const SkTextBlob*, SkScalar x, SkScalar y,
                              const SkPaint& paint, SkDrawFilter* drawFilter) override;
    virtual void drawVertices(const SkDraw&, SkCanvas::VertexMode, int vertexCount,
                              const SkPoint verts[], const SkPoint texs[],
                              const SkColor colors[], SkXfermode* xmode,
                              const uint16_t indices[], int indexCount,
                              const SkPaint&) override;
    void drawAtlas(const SkDraw&, const SkImage* atlas, const SkRSXform[], const SkRect[],
                       const SkColor[], int count, SkXfermode::Mode, const SkPaint&) override;
    virtual void drawDevice(const SkDraw&, SkBaseDevice*, int x, int y,
                            const SkPaint&) override;
    void drawImage(const SkDraw&, const SkImage*, SkScalar x, SkScalar y, const SkPaint&) override;
    void drawImageRect(const SkDraw&, const SkImage*, const SkRect* src, const SkRect& dst,
                       const SkPaint&, SkCanvas::SrcRectConstraint) override;

    void drawImageNine(const SkDraw& draw, const SkImage* image, const SkIRect& center,
                       const SkRect& dst, const SkPaint& paint) override;
    void drawBitmapNine(const SkDraw& draw, const SkBitmap& bitmap, const SkIRect& center,
                        const SkRect& dst, const SkPaint& paint) override;

    void flush() override;

    void onAttachToCanvas(SkCanvas* canvas) override;
    void onDetachFromCanvas() override;

    const SkBitmap& onAccessBitmap() override;
    bool onAccessPixels(SkPixmap*) override;

    bool canHandleImageFilter(const SkImageFilter*) override;
    virtual bool filterImage(const SkImageFilter*, const SkBitmap&,
                             const SkImageFilter::Context&,
                             SkBitmap*, SkIPoint*) override;

    bool filterTexture(GrContext*, GrTexture*, int width, int height, const SkImageFilter*,
                       const SkImageFilter::Context&,
                       SkBitmap* result, SkIPoint* offset);

    static SkImageFilter::Cache* NewImageFilterCache();

    // for debugging purposes only
    void drawTexture(GrTexture*, const SkRect& dst, const SkPaint&);

protected:
    bool onReadPixels(const SkImageInfo&, void*, size_t, int, int) override;
    bool onWritePixels(const SkImageInfo&, const void*, size_t, int, int) override;
    bool onShouldDisableLCD(const SkPaint&) const final;

    /**  PRIVATE / EXPERIMENTAL -- do not call */
    virtual bool EXPERIMENTAL_drawPicture(SkCanvas* canvas, const SkPicture* picture,
                                          const SkMatrix*, const SkPaint*) override;

private:
    // We want these unreffed in DrawContext, RenderTarget, GrContext order.
    SkAutoTUnref<GrContext>         fContext;
    SkAutoTUnref<GrRenderTarget>    fRenderTarget;
    SkAutoTUnref<GrDrawContext>     fDrawContext;

    SkAutoTUnref<const SkClipStack> fClipStack;
    SkIPoint                        fClipOrigin;
    GrClip                          fClip;;
    // remove when our clients don't rely on accessBitmap()
    SkBitmap                        fLegacyBitmap;
    bool                            fOpaque;

    enum Flags {
        kNeedClear_Flag = 1 << 0,  //!< Surface requires an initial clear
        kIsOpaque_Flag  = 1 << 1,  //!< Hint from client that rendering to this device will be
                                   //   opaque even if the config supports alpha.
    };
    static bool CheckAlphaTypeAndGetFlags(const SkImageInfo* info, InitContents init,
                                          unsigned* flags);

    SkGpuDevice(GrRenderTarget*, int width, int height, const SkSurfaceProps*, unsigned flags);

    SkBaseDevice* onCreateDevice(const CreateInfo&, const SkPaint*) override;

    sk_sp<SkSurface> makeSurface(const SkImageInfo&, const SkSurfaceProps&) override;

    SkImageFilter::Cache* getImageFilterCache() override;

    bool forceConservativeRasterClip() const override { return true; }

    // sets the render target and clip on context
    void prepareDraw(const SkDraw&);

    /**
     * Helper functions called by drawBitmapCommon. By the time these are called the SkDraw's
     * matrix, clip, and the device's render target has already been set on GrContext.
     */

    // The tileSize and clippedSrcRect will be valid only if true is returned.
    bool shouldTileImageID(uint32_t imageID, const SkIRect& imageRect,
                           const SkMatrix& viewMatrix,
                           const GrTextureParams& params,
                           const SkRect* srcRectPtr,
                           int maxTileSize,
                           int* tileSize,
                           SkIRect* clippedSubset) const;
    bool shouldTileBitmap(const SkBitmap& bitmap,
                          const SkMatrix& viewMatrix,
                          const GrTextureParams& sampler,
                          const SkRect* srcRectPtr,
                          int maxTileSize,
                          int* tileSize,
                          SkIRect* clippedSrcRect) const;
    // Just returns the predicate, not the out-tileSize or out-clippedSubset, as they are not
    // needed at the moment.
    bool shouldTileImage(const SkImage* image, const SkRect* srcRectPtr,
                         SkCanvas::SrcRectConstraint constraint, SkFilterQuality quality,
                         const SkMatrix& viewMatrix) const;

    void internalDrawBitmap(const SkBitmap&,
                            const SkMatrix& viewMatrix,
                            const SkRect&,
                            const GrTextureParams& params,
                            const SkPaint& paint,
                            SkCanvas::SrcRectConstraint,
                            bool bicubic,
                            bool needsTextureDomain);

    void drawTiledBitmap(const SkBitmap& bitmap,
                         const SkMatrix& viewMatrix,
                         const SkRect& srcRect,
                         const SkIRect& clippedSrcRect,
                         const GrTextureParams& params,
                         const SkPaint& paint,
                         SkCanvas::SrcRectConstraint,
                         int tileSize,
                         bool bicubic);

    void drawTextureProducer(GrTextureProducer*,
                             const SkRect* srcRect,
                             const SkRect* dstRect,
                             SkCanvas::SrcRectConstraint,
                             const SkMatrix& viewMatrix,
                             const GrClip&,
                             const SkPaint&);

    void drawTextureProducerImpl(GrTextureProducer*,
                                 const SkRect& clippedSrcRect,
                                 const SkRect& clippedDstRect,
                                 SkCanvas::SrcRectConstraint,
                                 const SkMatrix& viewMatrix,
                                 const SkMatrix& srcToDstMatrix,
                                 const GrClip&,
                                 const SkPaint&);

    bool drawFilledDRRect(const SkMatrix& viewMatrix, const SkRRect& outer,
                          const SkRRect& inner, const SkPaint& paint);

    void drawProducerNine(const SkDraw&, GrTextureProducer*, const SkIRect& center,
                          const SkRect& dst, const SkPaint&);

    bool drawDashLine(const SkPoint pts[2], const SkPaint& paint);

    static GrRenderTarget* CreateRenderTarget(GrContext*, SkBudgeted, const SkImageInfo&,
                                              int sampleCount, GrTextureStorageAllocator);

    void drawSpriteWithFilter(const SkDraw&, const SkBitmap&, int x, int y,
                              const SkPaint&) override;

    friend class GrAtlasTextContext;
    friend class SkSurface_Gpu;      // for access to surfaceProps
    typedef SkBaseDevice INHERITED;
};

#endif
