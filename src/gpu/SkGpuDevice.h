/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGpuDevice_DEFINED
#define SkGpuDevice_DEFINED

#include "GrClipStackClip.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrRenderTargetContext.h"
#include "GrTypes.h"
#include "SkBitmap.h"
#include "SkClipStackDevice.h"
#include "SkGr.h"
#include "SkPicture.h"
#include "SkRegion.h"
#include "SkSurface.h"

class GrAccelData;
class GrTextureMaker;
class GrTextureProducer;
struct GrCachedLayer;

class SkSpecialImage;

/**
 *  Subclass of SkBaseDevice, which directs all drawing to the GrGpu owned by the
 *  canvas.
 */
class SkGpuDevice : public SkClipStackDevice {
public:
    enum InitContents {
        kClear_InitContents,
        kUninit_InitContents
    };

    /**
     * Creates an SkGpuDevice from a GrRenderTargetContext whose backing width/height is
     * different than its actual width/height (e.g., approx-match scratch texture).
     */
    static sk_sp<SkGpuDevice> Make(GrContext*, sk_sp<GrRenderTargetContext> renderTargetContext,
                                   int width, int height, InitContents);

    /**
     * New device that will create an offscreen renderTarget based on the ImageInfo and
     * sampleCount. The mipMapped flag tells the gpu to create the underlying render target with
     * mips. The Budgeted param controls whether the device's backing store counts against the
     * resource cache budget. On failure, returns nullptr.
     * This entry point creates a kExact backing store. It is used when creating SkGpuDevices
     * for SkSurfaces.
     */
    static sk_sp<SkGpuDevice> Make(GrContext*, SkBudgeted, const SkImageInfo&,
                                   int sampleCount, GrSurfaceOrigin, const SkSurfaceProps*,
                                   GrMipMapped mipMapped, InitContents);

    ~SkGpuDevice() override {}

    GrContext* context() const override { return fContext.get(); }

    // set all pixels to 0
    void clearAll();

    void replaceRenderTargetContext(bool shouldRetainContent);

    GrRenderTargetContext* accessRenderTargetContext() override;

    void drawPaint(const SkPaint& paint) override;
    void drawPoints(SkCanvas::PointMode mode, size_t count, const SkPoint[],
                    const SkPaint& paint) override;
    void drawRect(const SkRect& r, const SkPaint& paint) override;
    void drawEdgeAARect(const SkRect& r, SkCanvas::QuadAAFlags edgeAA, SkColor color,
                        SkBlendMode mode) override;
    void drawRRect(const SkRRect& r, const SkPaint& paint) override;
    void drawDRRect(const SkRRect& outer, const SkRRect& inner, const SkPaint& paint) override;
    void drawRegion(const SkRegion& r, const SkPaint& paint) override;
    void drawOval(const SkRect& oval, const SkPaint& paint) override;
    void drawArc(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle,
                 bool useCenter, const SkPaint& paint) override;
    void drawPath(const SkPath& path, const SkPaint& paint, bool pathIsMutable) override;
    void drawBitmapRect(const SkBitmap&, const SkRect* srcOrNull, const SkRect& dst,
                        const SkPaint& paint, SkCanvas::SrcRectConstraint) override;
    void drawSprite(const SkBitmap& bitmap, int x, int y,
                    const SkPaint& paint) override;
    void drawGlyphRunList(const SkGlyphRunList& glyphRunList) override;
    void drawVertices(const SkVertices*, const SkVertices::Bone bones[], int boneCount, SkBlendMode,
                      const SkPaint&) override;
    void drawShadow(const SkPath&, const SkDrawShadowRec&) override;
    void drawAtlas(const SkImage* atlas, const SkRSXform[], const SkRect[],
                   const SkColor[], int count, SkBlendMode, const SkPaint&) override;
    void drawDevice(SkBaseDevice*, int x, int y, const SkPaint&) override;

    void drawImageRect(const SkImage*, const SkRect* src, const SkRect& dst,
                       const SkPaint&, SkCanvas::SrcRectConstraint) override;

    void drawImageNine(const SkImage* image, const SkIRect& center,
                       const SkRect& dst, const SkPaint& paint) override;
    void drawBitmapNine(const SkBitmap& bitmap, const SkIRect& center,
                        const SkRect& dst, const SkPaint& paint) override;

    void drawImageLattice(const SkImage*, const SkCanvas::Lattice&,
                          const SkRect& dst, const SkPaint&) override;
    void drawBitmapLattice(const SkBitmap&, const SkCanvas::Lattice&,
                           const SkRect& dst, const SkPaint&) override;
    void drawImageSet(const SkCanvas::ImageSetEntry[], int count, SkFilterQuality,
                      SkBlendMode) override;

    void drawDrawable(SkDrawable*, const SkMatrix*, SkCanvas* canvas) override;

    void drawSpecial(SkSpecialImage*, int left, int top, const SkPaint& paint,
                     SkImage*, const SkMatrix&) override;
    sk_sp<SkSpecialImage> makeSpecial(const SkBitmap&) override;
    sk_sp<SkSpecialImage> makeSpecial(const SkImage*) override;
    sk_sp<SkSpecialImage> snapSpecial() override;
    sk_sp<SkSpecialImage> snapBackImage(const SkIRect&) override;

    void flush() override;
    GrSemaphoresSubmitted flushAndSignalSemaphores(SkSurface::BackendSurfaceAccess access,
                                                   SkSurface::FlushFlags flags,
                                                   int numSemaphores,
                                                   GrBackendSemaphore signalSemaphores[]);
    bool wait(int numSemaphores, const GrBackendSemaphore* waitSemaphores);

    bool onAccessPixels(SkPixmap*) override;

    // Temporary interface until it gets lifted up to SkDevice and exposed in SkCanvas

    /*
     * dstClipCounts[] is a parallel array to the image entries, acting like the intended
     * dstClipCount field in ImageSetEntry. Similarly, preViewMatrixIdx is parallel and will
     * become an index field in ImageSetEntry that specifies an entry in the matrix array.
     */
    void tmp_drawImageSetV3(const SkCanvas::ImageSetEntry[],
            int dstClipCounts[], int preViewMatrixIdx[], int count,
            const SkPoint dstClips[], const SkMatrix preViewMatrices[], const SkPaint& paint,
            SkCanvas::SrcRectConstraint constraint = SkCanvas::kStrict_SrcRectConstraint);
    void tmp_drawEdgeAAQuad(const SkRect& rect, const SkPoint clip[], int clipCount,
                            SkCanvas::QuadAAFlags aaFlags, SkColor color, SkBlendMode mode);
protected:
    bool onReadPixels(const SkPixmap&, int, int) override;
    bool onWritePixels(const SkPixmap&, int, int) override;

private:
    // We want these unreffed in RenderTargetContext, GrContext order.
    sk_sp<GrContext>             fContext;
    sk_sp<GrRenderTargetContext> fRenderTargetContext;

    SkISize                      fSize;

    enum Flags {
        kNeedClear_Flag = 1 << 0,  //!< Surface requires an initial clear
        kIsOpaque_Flag  = 1 << 1,  //!< Hint from client that rendering to this device will be
                                   //   opaque even if the config supports alpha.
    };
    static bool CheckAlphaTypeAndGetFlags(const SkImageInfo* info, InitContents init,
                                          unsigned* flags);

    SkGpuDevice(GrContext*, sk_sp<GrRenderTargetContext>, int width, int height, unsigned flags);

    SkBaseDevice* onCreateDevice(const CreateInfo&, const SkPaint*) override;

    sk_sp<SkSurface> makeSurface(const SkImageInfo&, const SkSurfaceProps&) override;

    SkImageFilterCache* getImageFilterCache() override;

    bool forceConservativeRasterClip() const override { return true; }

    GrClipStackClip clip() const { return GrClipStackClip(&this->cs()); }

    const GrCaps* caps() const;

    /**
     * Helper functions called by drawBitmapCommon. By the time these are called the SkDraw's
     * matrix, clip, and the device's render target has already been set on GrContext.
     */

    // The tileSize and clippedSrcRect will be valid only if true is returned.
    bool shouldTileImageID(uint32_t imageID,
                           const SkIRect& imageRect,
                           const SkMatrix& viewMatrix,
                           const SkMatrix& srcToDstRectMatrix,
                           const GrSamplerState& params,
                           const SkRect* srcRectPtr,
                           int maxTileSize,
                           int* tileSize,
                           SkIRect* clippedSubset) const;
    // Just returns the predicate, not the out-tileSize or out-clippedSubset, as they are not
    // needed at the moment.
    bool shouldTileImage(const SkImage* image, const SkRect* srcRectPtr,
                         SkCanvas::SrcRectConstraint constraint, SkFilterQuality quality,
                         const SkMatrix& viewMatrix, const SkMatrix& srcToDstRect) const;

    sk_sp<SkSpecialImage> filterTexture(SkSpecialImage*,
                                        int left, int top,
                                        SkIPoint* offset,
                                        const SkImageFilter* filter);

    // Splits bitmap into tiles of tileSize and draws them using separate textures for each tile.
    void drawTiledBitmap(const SkBitmap& bitmap,
                         const SkMatrix& viewMatrix,
                         const SkMatrix& srcToDstMatrix,
                         const SkRect& srcRect,
                         const SkIRect& clippedSrcRect,
                         const GrSamplerState& params,
                         const SkPaint& paint,
                         SkCanvas::SrcRectConstraint,
                         int tileSize,
                         bool bicubic);

    // Used by drawTiledBitmap to draw each tile.
    void drawBitmapTile(const SkBitmap&,
                        const SkMatrix& viewMatrix,
                        const SkRect& dstRect,
                        const SkRect& srcRect,
                        const GrSamplerState& samplerState,
                        const SkPaint& paint,
                        SkCanvas::SrcRectConstraint,
                        bool bicubic,
                        bool needsTextureDomain);

    // If not null, dstClip must be contained inside dst and will also respect the edge AA flags.
    // If 'preViewMatrix' is not null, final CTM will be this->ctm() * preViewMatrix.
    void drawImageQuad(const SkImage*, const SkRect* src, const SkRect* dst,
                       const SkPoint dstClip[4], GrAA aa, GrQuadAAFlags aaFlags,
                       const SkMatrix* preViewMatrix, const SkPaint&, SkCanvas::SrcRectConstraint);

    // TODO(michaelludwig): This can be removed once drawBitmapRect is removed from SkDevice
    // so that drawImageQuad is the sole entry point into the draw-single-image op
    void drawTextureProducer(GrTextureProducer*,
                             const SkRect* srcRect,
                             const SkRect* dstRect,
                             SkCanvas::SrcRectConstraint,
                             const SkMatrix& viewMatrix,
                             const SkPaint&,
                             bool attemptDrawTexture);

    void drawProducerLattice(GrTextureProducer*, std::unique_ptr<SkLatticeIter>, const SkRect& dst,
                             const SkPaint&);

    void drawStrokedLine(const SkPoint pts[2], const SkPaint&);

    void wireframeVertices(SkVertices::VertexMode, int vertexCount, const SkPoint verts[],
                           const SkVertices::Bone bones[], int boneCount, SkBlendMode,
                           const uint16_t indices[], int indexCount, const SkPaint&);

    static sk_sp<GrRenderTargetContext> MakeRenderTargetContext(GrContext*,
                                                                SkBudgeted,
                                                                const SkImageInfo&,
                                                                int sampleCount,
                                                                GrSurfaceOrigin,
                                                                const SkSurfaceProps*,
                                                                GrMipMapped);

    friend class GrAtlasTextContext;
    friend class SkSurface_Gpu;      // for access to surfaceProps
    typedef SkClipStackDevice INHERITED;
};

#endif
