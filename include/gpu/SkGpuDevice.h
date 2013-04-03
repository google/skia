
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
#include "SkRegion.h"
#include "GrContext.h"

struct SkDrawProcs;
struct GrSkDrawProcs;
class GrTextContext;

/**
 *  Subclass of SkDevice, which directs all drawing to the GrGpu owned by the
 *  canvas.
 */
class SK_API SkGpuDevice : public SkDevice {
public:

    /**
     * Creates an SkGpuDevice from a GrSurface. This will fail if the surface is not a render
     * target. The caller owns a ref on the returned device.
     */
    static SkGpuDevice* Create(GrSurface* surface);

    /**
     *  New device that will create an offscreen renderTarget based on the
     *  config, width, height, and sampleCount. The device's storage will not
     *  count against the GrContext's texture cache budget. The device's pixels
     *  will be uninitialized. TODO: This can fail, replace with a factory function.
     */
    SkGpuDevice(GrContext*, SkBitmap::Config, int width, int height, int sampleCount = 0);

    /**
     *  New device that will render to the specified renderTarget.
     *  DEPRECATED: Use Create(surface)
     */
    SkGpuDevice(GrContext*, GrRenderTarget*);

    /**
     *  New device that will render to the texture (as a rendertarget).
     *  The GrTexture's asRenderTarget() must be non-NULL or device will not
     *  function.
     *  DEPRECATED: Use Create(surface)
     */
    SkGpuDevice(GrContext*, GrTexture*);

    virtual ~SkGpuDevice();

    GrContext* context() const { return fContext; }

    virtual SkGpuRenderTarget* accessRenderTarget() SK_OVERRIDE;

    // overrides from SkDevice

    virtual void clear(SkColor color) SK_OVERRIDE;
    virtual void writePixels(const SkBitmap& bitmap, int x, int y,
                             SkCanvas::Config8888 config8888) SK_OVERRIDE;

    virtual void drawPaint(const SkDraw&, const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPoints(const SkDraw&, SkCanvas::PointMode mode, size_t count,
                            const SkPoint[], const SkPaint& paint) SK_OVERRIDE;
    virtual void drawRect(const SkDraw&, const SkRect& r,
                          const SkPaint& paint) SK_OVERRIDE;
    virtual void drawOval(const SkDraw&, const SkRect& oval,
                          const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPath(const SkDraw&, const SkPath& path,
                          const SkPaint& paint, const SkMatrix* prePathMatrix,
                          bool pathIsMutable) SK_OVERRIDE;
    virtual void drawBitmap(const SkDraw&, const SkBitmap& bitmap,
                            const SkIRect* srcRectOrNull,
                            const SkMatrix&, const SkPaint&) SK_OVERRIDE;
    virtual void drawBitmapRect(const SkDraw&, const SkBitmap&,
                                const SkRect* srcOrNull, const SkRect& dst,
                                const SkPaint& paint) SK_OVERRIDE;
    virtual void drawSprite(const SkDraw&, const SkBitmap& bitmap,
                            int x, int y, const SkPaint& paint);
    virtual void drawText(const SkDraw&, const void* text, size_t len,
                          SkScalar x, SkScalar y, const SkPaint&) SK_OVERRIDE;
    virtual void drawPosText(const SkDraw&, const void* text, size_t len,
                             const SkScalar pos[], SkScalar constY,
                             int scalarsPerPos, const SkPaint&) SK_OVERRIDE;
    virtual void drawTextOnPath(const SkDraw&, const void* text, size_t len,
                                const SkPath& path, const SkMatrix* matrix,
                                const SkPaint&) SK_OVERRIDE;
    virtual void drawVertices(const SkDraw&, SkCanvas::VertexMode, int vertexCount,
                              const SkPoint verts[], const SkPoint texs[],
                              const SkColor colors[], SkXfermode* xmode,
                              const uint16_t indices[], int indexCount,
                              const SkPaint&) SK_OVERRIDE;
    virtual void drawDevice(const SkDraw&, SkDevice*, int x, int y,
                            const SkPaint&) SK_OVERRIDE;
    virtual bool filterTextFlags(const SkPaint&, TextFlags*) SK_OVERRIDE;

    virtual void flush();

    virtual void onAttachToCanvas(SkCanvas* canvas) SK_OVERRIDE;
    virtual void onDetachFromCanvas() SK_OVERRIDE;

    /**
     * Make's this device's rendertarget current in the underlying 3D API.
     * Also implicitly flushes.
     */
    virtual void makeRenderTargetCurrent();

    virtual bool canHandleImageFilter(SkImageFilter*) SK_OVERRIDE;
    virtual bool filterImage(SkImageFilter*, const SkBitmap&, const SkMatrix&,
                             SkBitmap*, SkIPoint*) SK_OVERRIDE;

    class SkAutoCachedTexture; // used internally

protected:
    // overrides from SkDevice
    virtual bool onReadPixels(const SkBitmap& bitmap,
                              int x, int y,
                              SkCanvas::Config8888 config8888) SK_OVERRIDE;

private:
    GrContext*      fContext;

    GrSkDrawProcs*  fDrawProcs;

    GrClipData      fClipData;

    // state for our render-target
    GrRenderTarget*     fRenderTarget;
    bool                fNeedClear;

    // called from rt and tex cons
    void initFromRenderTarget(GrContext*, GrRenderTarget*, bool cached);

    // used by createCompatibleDevice
    SkGpuDevice(GrContext*, GrTexture* texture, bool needClear);

    // override from SkDevice
    virtual SkDevice* onCreateCompatibleDevice(SkBitmap::Config config,
                                               int width, int height,
                                               bool isOpaque,
                                               Usage usage) SK_OVERRIDE;

    SkDrawProcs* initDrawForText(GrTextContext*);
    bool bindDeviceAsTexture(GrPaint* paint);

    // sets the render target, clip, and matrix on GrContext. Use forceIdenity to override
    // SkDraw's matrix and draw in device coords.
    void prepareDraw(const SkDraw&, bool forceIdentity);

    /**
     * Implementation for both drawBitmap and drawBitmapRect.
     */
    void drawBitmapCommon(const SkDraw&,
                          const SkBitmap& bitmap,
                          const SkRect* srcRectPtr,
                          const SkMatrix&,
                          const SkPaint&);

    /**
     * Helper functions called by drawBitmapCommon. By the time these are called the SkDraw's
     * matrix has already been set on GrContext
     */
    bool shouldTileBitmap(const SkBitmap& bitmap,
                          const GrTextureParams& sampler,
                          const SkRect* srcRectPtr) const;
    void internalDrawBitmap(const SkBitmap&,
                            const SkRect&,
                            const SkMatrix&,
                            const GrTextureParams& params,
                            GrPaint* grPaint);
    void drawTiledBitmap(const SkBitmap& bitmap,
                         const SkRect& srcRect,
                         const SkMatrix& m,
                         const GrTextureParams& params,
                         GrPaint* grPaint);

    /**
     * Returns non-initialized instance.
     */
    GrTextContext* getTextContext();

    typedef SkDevice INHERITED;
};

#endif
