
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
     *  New device that will create an offscreen renderTarget based on the
     *  config, width, height. The device's storage will not count against
     *  the GrContext's texture cache budget. The device's pixels will be
     *  uninitialized.
     */
    SkGpuDevice(GrContext*, SkBitmap::Config, int width, int height);

    /**
     *  New device that will render to the specified renderTarget.
     */
    SkGpuDevice(GrContext*, GrRenderTarget*);

    /**
     *  New device that will render to the texture (as a rendertarget).
     *  The GrTexture's asRenderTarget() must be non-NULL or device will not
     *  function.
     */
    SkGpuDevice(GrContext*, GrTexture*);

    virtual ~SkGpuDevice();

    GrContext* context() const { return fContext; }

    /**
     *  Override from SkGpuDevice, so we can set our FBO to be the render target
     *  The canvas parameter must be a SkGpuCanvas
     */
    virtual void gainFocus(SkCanvas*, const SkMatrix&, const SkRegion&,
                           const SkClipStack& clipStack) SK_OVERRIDE;

    virtual SkGpuRenderTarget* accessRenderTarget() SK_OVERRIDE;

    // overrides from SkDevice

    virtual void clear(SkColor color) SK_OVERRIDE;
    virtual void writePixels(const SkBitmap& bitmap, int x, int y,
                             SkCanvas::Config8888 config8888) SK_OVERRIDE;

    virtual void setMatrixClip(const SkMatrix& matrix, const SkRegion& clip,
                               const SkClipStack&) SK_OVERRIDE;

    virtual void drawPaint(const SkDraw&, const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPoints(const SkDraw&, SkCanvas::PointMode mode, size_t count,
                            const SkPoint[], const SkPaint& paint) SK_OVERRIDE;
    virtual void drawRect(const SkDraw&, const SkRect& r,
                          const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPath(const SkDraw&, const SkPath& path,
                          const SkPaint& paint, const SkMatrix* prePathMatrix,
                          bool pathIsMutable) SK_OVERRIDE;
    virtual void drawBitmap(const SkDraw&, const SkBitmap& bitmap,
                            const SkIRect* srcRectOrNull,
                            const SkMatrix&, const SkPaint&) SK_OVERRIDE;
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
    typedef GrContext::TextureCacheEntry TexCache;
    TexCache lockCachedTexture(const SkBitmap& bitmap,
                               const GrSamplerState* sampler);
    bool isBitmapInTextureCache(const SkBitmap& bitmap,
                                const GrSamplerState& sampler) const;
    void unlockCachedTexture(TexCache);

    // overrides from SkDevice
    virtual bool onReadPixels(const SkBitmap& bitmap,
                              int x, int y,
                              SkCanvas::Config8888 config8888) SK_OVERRIDE;

private:
    GrContext*      fContext;

    GrSkDrawProcs*  fDrawProcs;

    // state for our offscreen render-target
    TexCache            fCache;
    GrTexture*          fTexture;
    GrRenderTarget*     fRenderTarget;
    bool                fNeedClear;
    bool                fNeedPrepareRenderTarget;

    GrTextContext*      fTextContext;

    // called from rt and tex cons
    void initFromRenderTarget(GrContext*, GrRenderTarget*);

    // used by createCompatibleDevice
    SkGpuDevice(GrContext*, GrTexture* texture, TexCache, bool needClear);

    // overrides from SkDevice
    virtual void postSave() SK_OVERRIDE {
        fContext->postClipPush();
    }
    virtual void preRestore() SK_OVERRIDE {
        fContext->preClipPop();
    }

    virtual SkDevice* onCreateCompatibleDevice(SkBitmap::Config config,
                                               int width, int height,
                                               bool isOpaque,
                                               Usage usage) SK_OVERRIDE;

    SkDrawProcs* initDrawForText(GrTextContext*);
    bool bindDeviceAsTexture(GrPaint* paint);

    void prepareRenderTarget(const SkDraw&);
    bool shouldTileBitmap(const SkBitmap& bitmap,
                          const GrSamplerState& sampler,
                          const SkIRect* srcRectPtr,
                          int* tileSize) const;
    void internalDrawBitmap(const SkDraw&, const SkBitmap&,
                            const SkIRect&, const SkMatrix&, GrPaint* grPaint);

    /**
     * Returns non-initialized instance.
     */
    GrTextContext* getTextContext();

    typedef SkDevice INHERITED;
};

#endif

