/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGpuDevice_DEFINED
#define SkGpuDevice_DEFINED

#include "SkGr.h"
#include "SkGrPriv.h"
#include "SkBitmap.h"
#include "SkDevice.h"
#include "SkPicture.h"
#include "SkRegion.h"
#include "SkSurface.h"
#include "GrClipStackClip.h"
#include "GrDrawContext.h"
#include "GrContext.h"
#include "GrSurfacePriv.h"
#include "GrTypes.h"

class GrAccelData;
class GrTextureProducer;
struct GrCachedLayer;

class SkSpecialImage;

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
     * Creates an SkGpuDevice from a GrDrawContext whose backing width/height is
     * different than its actual width/height (e.g., approx-match scratch texture).
     */
    static sk_sp<SkGpuDevice> Make(sk_sp<GrDrawContext> drawContext,
                                   int width, int height,
                                   InitContents);

    /**
     * New device that will create an offscreen renderTarget based on the ImageInfo and
     * sampleCount. The Budgeted param controls whether the device's backing store counts against
     * the resource cache budget. On failure, returns nullptr.
     */
    static sk_sp<SkGpuDevice> Make(GrContext*, SkBudgeted, const SkImageInfo&,
                                   int sampleCount, GrSurfaceOrigin, 
                                   const SkSurfaceProps*, InitContents);

    ~SkGpuDevice() override {}

    GrContext* context() const override { return fContext; }

    // set all pixels to 0
    void clearAll();

    void replaceDrawContext(bool shouldRetainContent);

    GrDrawContext* accessDrawContext() override;

    void drawPaint(const SkDraw&, const SkPaint& paint) override;
    void drawPoints(const SkDraw&, SkCanvas::PointMode mode, size_t count, const SkPoint[],
                    const SkPaint& paint) override;
    void drawRect(const SkDraw&, const SkRect& r, const SkPaint& paint) override;
    void drawRRect(const SkDraw&, const SkRRect& r, const SkPaint& paint) override;
    void drawDRRect(const SkDraw& draw, const SkRRect& outer, const SkRRect& inner,
                    const SkPaint& paint) override;
    void drawRegion(const SkDraw&, const SkRegion& r, const SkPaint& paint) override;
    void drawOval(const SkDraw&, const SkRect& oval, const SkPaint& paint) override;
    void drawArc(const SkDraw&, const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle,
                 bool useCenter, const SkPaint& paint) override;
    void drawPath(const SkDraw&, const SkPath& path, const SkPaint& paint,
                  const SkMatrix* prePathMatrix, bool pathIsMutable) override;
    void drawBitmap(const SkDraw&, const SkBitmap& bitmap, const SkMatrix&,
                    const SkPaint&) override;
    void drawBitmapRect(const SkDraw&, const SkBitmap&, const SkRect* srcOrNull, const SkRect& dst,
                        const SkPaint& paint, SkCanvas::SrcRectConstraint) override;
    void drawSprite(const SkDraw&, const SkBitmap& bitmap, int x, int y,
                    const SkPaint& paint) override;
    void drawText(const SkDraw&, const void* text, size_t len, SkScalar x, SkScalar y,
                  const SkPaint&) override;
    void drawPosText(const SkDraw&, const void* text, size_t len, const SkScalar pos[],
                     int scalarsPerPos, const SkPoint& offset, const SkPaint&) override;
    void drawTextBlob(const SkDraw&, const SkTextBlob*, SkScalar x, SkScalar y,
                      const SkPaint& paint, SkDrawFilter* drawFilter) override;
    void drawVertices(const SkDraw&, SkCanvas::VertexMode, int vertexCount, const SkPoint verts[],
                      const SkPoint texs[], const SkColor colors[], SkXfermode* xmode,
                      const uint16_t indices[], int indexCount, const SkPaint&) override;
    void drawAtlas(const SkDraw&, const SkImage* atlas, const SkRSXform[], const SkRect[],
                   const SkColor[], int count, SkXfermode::Mode, const SkPaint&) override;
    void drawDevice(const SkDraw&, SkBaseDevice*, int x, int y, const SkPaint&) override;

    void drawImage(const SkDraw&, const SkImage*, SkScalar x, SkScalar y, const SkPaint&) override;
    void drawImageRect(const SkDraw&, const SkImage*, const SkRect* src, const SkRect& dst,
                       const SkPaint&, SkCanvas::SrcRectConstraint) override;

    void drawImageNine(const SkDraw& draw, const SkImage* image, const SkIRect& center,
                       const SkRect& dst, const SkPaint& paint) override;
    void drawBitmapNine(const SkDraw& draw, const SkBitmap& bitmap, const SkIRect& center,
                        const SkRect& dst, const SkPaint& paint) override;

    void drawImageLattice(const SkDraw&, const SkImage*, const SkCanvas::Lattice&,
                          const SkRect& dst, const SkPaint&) override;
    void drawBitmapLattice(const SkDraw&, const SkBitmap&, const SkCanvas::Lattice&,
                           const SkRect& dst, const SkPaint&) override;

    void drawSpecial(const SkDraw&, SkSpecialImage*,
                     int left, int top, const SkPaint& paint) override;
    sk_sp<SkSpecialImage> makeSpecial(const SkBitmap&) override;
    sk_sp<SkSpecialImage> makeSpecial(const SkImage*) override;
    sk_sp<SkSpecialImage> snapSpecial() override;

    void flush() override;

    bool onAccessPixels(SkPixmap*) override;

    // for debugging purposes only
    void drawTexture(GrTexture*, const SkRect& dst, const SkPaint&);

protected:
    bool onReadPixels(const SkImageInfo&, void*, size_t, int, int) override;
    bool onWritePixels(const SkImageInfo&, const void*, size_t, int, int) override;
    bool onShouldDisableLCD(const SkPaint&) const final;

private:
    // We want these unreffed in DrawContext, GrContext order.
    SkAutoTUnref<GrContext>         fContext;
    sk_sp<GrDrawContext>            fDrawContext;

    SkIPoint                        fClipOrigin;
    GrClipStackClip                 fClip;
    SkISize                         fSize;
    bool                            fOpaque;

    enum Flags {
        kNeedClear_Flag = 1 << 0,  //!< Surface requires an initial clear
        kIsOpaque_Flag  = 1 << 1,  //!< Hint from client that rendering to this device will be
                                   //   opaque even if the config supports alpha.
    };
    static bool CheckAlphaTypeAndGetFlags(const SkImageInfo* info, InitContents init,
                                          unsigned* flags);

    SkGpuDevice(sk_sp<GrDrawContext>, int width, int height, unsigned flags);

    SkBaseDevice* onCreateDevice(const CreateInfo&, const SkPaint*) override;

    sk_sp<SkSurface> makeSurface(const SkImageInfo&, const SkSurfaceProps&) override;

    SkImageFilterCache* getImageFilterCache() override;

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
                           const SkMatrix& srcToDstRectMatrix,
                           const GrTextureParams& params,
                           const SkRect* srcRectPtr,
                           int maxTileSize,
                           int* tileSize,
                           SkIRect* clippedSubset) const;
    // Just returns the predicate, not the out-tileSize or out-clippedSubset, as they are not
    // needed at the moment.
    bool shouldTileImage(const SkImage* image, const SkRect* srcRectPtr,
                         SkCanvas::SrcRectConstraint constraint, SkFilterQuality quality,
                         const SkMatrix& viewMatrix, const SkMatrix& srcToDstRect) const;

    sk_sp<SkSpecialImage> filterTexture(const SkDraw&,
                                        SkSpecialImage*,
                                        int left, int top,
                                        SkIPoint* offset,
                                        const SkImageFilter* filter);

    // Splits bitmap into tiles of tileSize and draws them using separate textures for each tile.
    void drawTiledBitmap(const SkBitmap& bitmap,
                         const SkMatrix& viewMatrix,
                         const SkMatrix& srcToDstMatrix,
                         const SkRect& srcRect,
                         const SkIRect& clippedSrcRect,
                         const GrTextureParams& params,
                         const SkPaint& paint,
                         SkCanvas::SrcRectConstraint,
                         int tileSize,
                         bool bicubic);

    // Used by drawTiledBitmap to draw each tile.
    void drawBitmapTile(const SkBitmap&,
                        const SkMatrix& viewMatrix,
                        const SkRect& dstRect,
                        const SkRect& srcRect,
                        const GrTextureParams& params,
                        const SkPaint& paint,
                        SkCanvas::SrcRectConstraint,
                        bool bicubic,
                        bool needsTextureDomain);

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

    void drawProducerLattice(const SkDraw&, GrTextureProducer*, const SkCanvas::Lattice& lattice,
                             const SkRect& dst, const SkPaint&);

    bool drawDashLine(const SkPoint pts[2], const SkPaint& paint);
    void drawStrokedLine(const SkPoint pts[2], const SkDraw&, const SkPaint&);

    static sk_sp<GrDrawContext> MakeDrawContext(GrContext*,
                                                SkBudgeted,
                                                const SkImageInfo&,
                                                int sampleCount,
                                                GrSurfaceOrigin,
                                                const SkSurfaceProps*);

    friend class GrAtlasTextContext;
    friend class SkSurface_Gpu;      // for access to surfaceProps
    typedef SkBaseDevice INHERITED;
};

#endif
