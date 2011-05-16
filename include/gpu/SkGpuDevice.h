/*
    Copyright 2010 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#ifndef SkGpuDevice_DEFINED
#define SkGpuDevice_DEFINED

#include "SkGr.h"
#include "SkDevice.h"
#include "SkRegion.h"

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
     * The SkGpuDevice will render to the GrRenderTarget, or if the paremeter is
     * null it will create its own render target and manage that target's
     * lifetime.
     */
    SkGpuDevice(GrContext*,
                const SkBitmap& bitmap,
                GrRenderTarget* renderTargetOrNull);

    /**
     * Magic value that can be passed to constructor. Causes
     * the device to infer rendertarget from underlying 3D API (e.g. GL or D3D).
     * This isn't a valid pointer, don't attempt to dereference.
     */
    static GrRenderTarget* Current3DApiRenderTarget();

    virtual ~SkGpuDevice();

    GrContext* context() const { return fContext; }

    /**
     *  If this device was built for rendering as a layer (i.e. offscreen),
     *  then this will return the platform-specific handle to that GPU resource.
     *  For example, in OpenGL, this will return the FBO's texture ID.
     *  If this device was not built for rendering as a layer, then 0
     *  is returned.
     */
    intptr_t getLayerTextureHandle() const;

    // call to set the clip to the specified rect
    void scissor(const SkIRect&);

    /**
     *  Override from SkGpuDevice, so we can set our FBO to be the render target
     *  The canvas parameter must be a SkGpuCanvas
     */
    virtual void gainFocus(SkCanvas*, const SkMatrix&, const SkRegion&,
                           const SkClipStack& clipStack);

    virtual SkGpuTexture* accessTexture() { return (SkGpuTexture*)fTexture; }

    // overrides from SkDevice

    virtual void clear(SkColor color);
    virtual bool readPixels(const SkIRect& srcRect, SkBitmap* bitmap);
    virtual void writePixels(const SkBitmap& bitmap, int x, int y);

    virtual void setMatrixClip(const SkMatrix& matrix, const SkRegion& clip,
                               const SkClipStack&);

    virtual void drawPaint(const SkDraw&, const SkPaint& paint);
    virtual void drawPoints(const SkDraw&, SkCanvas::PointMode mode, size_t count,
                            const SkPoint[], const SkPaint& paint);
    virtual void drawRect(const SkDraw&, const SkRect& r,
                          const SkPaint& paint);
    virtual void drawPath(const SkDraw&, const SkPath& path,
                          const SkPaint& paint, const SkMatrix* prePathMatrix,
                          bool pathIsMutable);
    virtual void drawBitmap(const SkDraw&, const SkBitmap& bitmap,
                            const SkIRect* srcRectOrNull,
                            const SkMatrix& matrix, const SkPaint& paint);
    virtual void drawSprite(const SkDraw&, const SkBitmap& bitmap,
                            int x, int y, const SkPaint& paint);
    virtual void drawText(const SkDraw&, const void* text, size_t len,
                          SkScalar x, SkScalar y, const SkPaint& paint);
    virtual void drawPosText(const SkDraw&, const void* text, size_t len,
                             const SkScalar pos[], SkScalar constY,
                             int scalarsPerPos, const SkPaint& paint);
    virtual void drawTextOnPath(const SkDraw&, const void* text, size_t len,
                                const SkPath& path, const SkMatrix* matrix,
                                const SkPaint& paint);
    virtual void drawVertices(const SkDraw&, SkCanvas::VertexMode, int vertexCount,
                              const SkPoint verts[], const SkPoint texs[],
                              const SkColor colors[], SkXfermode* xmode,
                              const uint16_t indices[], int indexCount,
                              const SkPaint& paint);
    virtual void drawDevice(const SkDraw&, SkDevice*, int x, int y,
                            const SkPaint&);
    virtual bool filterTextFlags(const SkPaint& paint, TextFlags*);

    virtual void flush() { fContext->flush(false); }

    /**
     * Make's this device's rendertarget current in the underlying 3D API.
     * Also implicitly flushes.
     */
    virtual void makeRenderTargetCurrent();

protected:
    // override
    virtual SkDeviceFactory* onNewDeviceFactory();

    class TexCache;
    TexCache* lockCachedTexture(const SkBitmap& bitmap,
                                const GrSamplerState& sampler,
                                GrTexture** texture,
                                bool forDeviceRenderTarget = false);
    void unlockCachedTexture(TexCache*);

    class SkAutoCachedTexture {
    public:
        SkAutoCachedTexture();
        SkAutoCachedTexture(SkGpuDevice* device,
                            const SkBitmap& bitmap,
                            const GrSamplerState& sampler,
                            GrTexture** texture);
        ~SkAutoCachedTexture();

        GrTexture* set(SkGpuDevice*, const SkBitmap&, const GrSamplerState&);

    private:
        SkGpuDevice*    fDevice;
        TexCache*       fTex;
    };
    friend class SkAutoTexCache;

private:
    GrContext*      fContext;

    GrSkDrawProcs*  fDrawProcs;

    // state for our offscreen render-target
    TexCache*       fCache;
    GrTexture*      fTexture;
    GrRenderTarget* fRenderTarget;
    bool            fNeedClear;
    bool            fNeedPrepareRenderTarget;

    // doesn't set the texture/sampler/matrix state
    // caller needs to null out GrPaint's texture if
    // non-textured drawing is desired.
    // Set constantColor to true if a constant color
    // will be used.  This is an optimization, and can 
    // always be set to false. constantColor should 
    // never be true if justAlpha is true.
    bool skPaint2GrPaintNoShader(const SkPaint& skPaint,
                                 bool justAlpha,
                                 GrPaint* grPaint,
                                 bool constantColor);

    // uses the SkShader to setup paint, act used to
    // hold lock on cached texture and free it when
    // destroyed.
    // If there is no shader, constantColor will
    // be passed to skPaint2GrPaintNoShader.  Otherwise
    // it is ignored.
    bool skPaint2GrPaintShader(const SkPaint& skPaint,
                               SkAutoCachedTexture* act,
                               const SkMatrix& ctm,
                               GrPaint* grPaint,
                               bool constantColor);

    SkDrawProcs* initDrawForText(GrTextContext*);
    bool bindDeviceAsTexture(GrPaint* paint);

    void prepareRenderTarget(const SkDraw&);
    void internalDrawBitmap(const SkDraw&, const SkBitmap&,
                            const SkIRect&, const SkMatrix&, GrPaint* grPaint);

    typedef SkDevice INHERITED;
};

#endif

