#ifndef SkGLDevice_DEFINED
#define SkGLDevice_DEFINED

#include "SkDevice.h"
#include "SkGL.h"
#include "SkRegion.h"

#ifdef SK_BUILD_FOR_MAC
    #include <OpenGL/gl.h>
#elif defined(ANDROID)
    #include <GLES/gl.h>
#endif

class SkGLDeviceFactory : public SkDeviceFactory {
public:
    virtual SkDevice* newDevice(SkBitmap::Config config, int width, int height,
                                bool isOpaque, bool isForLayer);
};

struct SkGLDrawProcs;

class SkGLDevice : public SkDevice {
public:
    SkGLDevice(const SkBitmap& bitmap, bool offscreen);
    virtual ~SkGLDevice();

    virtual SkDeviceFactory* getDeviceFactory() {
        return SkNEW(SkGLDeviceFactory);
    }

    virtual uint32_t getDeviceCapabilities() { return kGL_Capability; }

    // used to identify GLTextCache data in the glyphcache
    static void GlyphCacheAuxProc(void* data);    
    
    enum TexOrientation {
        kNo_TexOrientation,
        kTopToBottom_TexOrientation,
        kBottomToTop_TexOrientation
    };

    /** Called when this device is no longer a candidate for a render target,
        but will instead be used as a texture to be drawn. Be sure to call
        the base impl if you override, as it will compute size and max.
    */
    virtual TexOrientation bindDeviceAsTexture();

    // returns true if complex
    SkGLClipIter* updateMatrixClip();
    // call to set the clip to the specified rect
    void scissor(const SkIRect&);

    // overrides from SkDevice
    virtual void gainFocus(SkCanvas*);
    virtual void setMatrixClip(const SkMatrix& matrix, const SkRegion& clip);

    virtual void drawPaint(const SkDraw&, const SkPaint& paint);
    virtual void drawPoints(const SkDraw&, SkCanvas::PointMode mode, size_t count,
                            const SkPoint[], const SkPaint& paint);
    virtual void drawRect(const SkDraw&, const SkRect& r,
                          const SkPaint& paint);
    virtual void drawPath(const SkDraw&, const SkPath& path,
                          const SkPaint& paint);
    virtual void drawBitmap(const SkDraw&, const SkBitmap& bitmap,
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

    // settings for the global texture cache

    static size_t GetTextureCacheMaxCount();
    static void SetTextureCacheMaxCount(size_t count);

    static size_t GetTextureCacheMaxSize();
    static void SetTextureCacheMaxSize(size_t size);

    /** Call glDeleteTextures for all textures (including those for text)
        This should be called while the gl-context is still valid. Its purpose
        is to free up gl resources. Note that if a bitmap or text is drawn after
        this call, new caches will be created.
    */
    static void DeleteAllTextures();

    /** Forget all textures without calling delete (including those for text).
        This should be called if the gl-context has changed, and the texture
        IDs that have been cached are no longer valid.
    */
    static void AbandonAllTextures();

protected:
    /** Return the current glmatrix, from a previous call to setMatrixClip */
    const SkMatrix& matrix() const { return fMatrix; }
    /** Return the current clip, from a previous call to setMatrixClip */
    const SkRegion& clip() const { return fClip; }

private:
    SkGLMatrix  fGLMatrix;
    SkMatrix    fMatrix;
    SkRegion    fClip;
    bool        fDirty;

    SkGLClipIter fClipIter;
    SkGLDrawProcs* fDrawProcs;

    void setupForText(SkDraw* draw, const SkPaint& paint);

    // global texture cache methods
    class TexCache;
    static TexCache* LockTexCache(const SkBitmap&, GLuint* name,
                                    SkPoint* size);
    static void UnlockTexCache(TexCache*);
    class SkAutoLockTexCache {
    public:
        SkAutoLockTexCache(const SkBitmap& bitmap, GLuint* name,
                       SkPoint* size) {
            fTex = SkGLDevice::LockTexCache(bitmap, name, size);
        }
        ~SkAutoLockTexCache() {
            if (fTex) {
                SkGLDevice::UnlockTexCache(fTex);
            }
        }
        TexCache* get() const { return fTex; }
    private:
        TexCache* fTex;
    };
    friend class SkAutoTexCache;
    
    // returns cache if the texture is bound for the shader
    TexCache* setupGLPaintShader(const SkPaint& paint);
    
    class AutoPaintShader {
    public:
        AutoPaintShader(SkGLDevice*, const SkPaint& paint);
        ~AutoPaintShader();
        
        bool useTex() const { return fTexCache != 0; }
    private:
        SkGLDevice* fDevice;
        TexCache*   fTexCache;
    };
    friend class AutoPaintShader;
        
    typedef SkDevice INHERITED;
};

#endif

