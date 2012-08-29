
/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkDevice_DEFINED
#define SkDevice_DEFINED

#include "SkRefCnt.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColor.h"

class SkClipStack;
class SkDraw;
struct SkIRect;
class SkMatrix;
class SkMetaData;
class SkRegion;

// This is an opaque class, not interpreted by skia
class SkGpuRenderTarget;

class SK_API SkDevice : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(SkDevice)

    /**
     *  Construct a new device with the specified bitmap as its backend. It is
     *  valid for the bitmap to have no pixels associated with it. In that case,
     *  any drawing to this device will have no effect.
    */
    SkDevice(const SkBitmap& bitmap);

    /**
     *  Create a new raster device and have the pixels be automatically
     *  allocated. The rowBytes of the device will be computed automatically
     *  based on the config and the width.
     *
     *  @param config   The desired config for the pixels. If the request cannot
     *                  be met, the closest matching support config will be used.
     *  @param width    width (in pixels) of the device
     *  @param height   height (in pixels) of the device
     *  @param isOpaque Set to true if it is known that all of the pixels will
     *                  be drawn to opaquely. Used as an accelerator when drawing
     *                  these pixels to another device.
     */
    SkDevice(SkBitmap::Config config, int width, int height, bool isOpaque = false);

    virtual ~SkDevice();

    /**
     *  Creates a device that is of the same type as this device (e.g. SW-raster,
     *  GPU, or PDF). The backing store for this device is created automatically
     *  (e.g. offscreen pixels or FBO or whatever is appropriate).
     *
     *  @param width    width of the device to create
     *  @param height   height of the device to create
     *  @param isOpaque performance hint, set to true if you know that you will
     *                  draw into this device such that all of the pixels will
     *                  be opaque.
     */
    SkDevice* createCompatibleDevice(SkBitmap::Config config,
                                     int width, int height,
                                     bool isOpaque);

    SkMetaData& getMetaData();

    enum Capabilities {
        kGL_Capability     = 0x1,  //!< mask indicating GL support
        kVector_Capability = 0x2,  //!< mask indicating a vector representation
        kAll_Capabilities  = 0x3
    };
    virtual uint32_t getDeviceCapabilities() { return 0; }

    /** Return the width of the device (in pixels).
    */
    virtual int width() const { return fBitmap.width(); }
    /** Return the height of the device (in pixels).
    */
    virtual int height() const { return fBitmap.height(); }

    /**
     *  Return the bounds of the device in the coordinate space of the root
     *  canvas. The root device will have its top-left at 0,0, but other devices
     *  such as those associated with saveLayer may have a non-zero origin.
     */
    void getGlobalBounds(SkIRect* bounds) const;

    /** Returns true if the device's bitmap's config treats every pixels as
        implicitly opaque.
    */
    bool isOpaque() const { return fBitmap.isOpaque(); }

    /** Return the bitmap config of the device's pixels
    */
    SkBitmap::Config config() const { return fBitmap.getConfig(); }

    /** Return the bitmap associated with this device. Call this each time you need
        to access the bitmap, as it notifies the subclass to perform any flushing
        etc. before you examine the pixels.
        @param changePixels set to true if the caller plans to change the pixels
        @return the device's bitmap
    */
    const SkBitmap& accessBitmap(bool changePixels);

    /**
     *  DEPRECATED: This will be made protected once WebKit stops using it.
     *              Instead use Canvas' writePixels method.
     *
     *  Similar to draw sprite, this method will copy the pixels in bitmap onto
     *  the device, with the top/left corner specified by (x, y). The pixel
     *  values in the device are completely replaced: there is no blending.
     *
     *  Currently if bitmap is backed by a texture this is a no-op. This may be
     *  relaxed in the future.
     *
     *  If the bitmap has config kARGB_8888_Config then the config8888 param
     *  will determines how the pixel valuess are intepreted. If the bitmap is
     *  not kARGB_8888_Config then this parameter is ignored.
     */
    virtual void writePixels(const SkBitmap& bitmap, int x, int y,
                             SkCanvas::Config8888 config8888 = SkCanvas::kNative_Premul_Config8888);

    /**
     * Return the device's associated gpu render target, or NULL.
     */
    virtual SkGpuRenderTarget* accessRenderTarget() { return NULL; }


    /**
     *  Return the device's origin: its offset in device coordinates from
     *  the default origin in its canvas' matrix/clip
     */
    const SkIPoint& getOrigin() const { return fOrigin; }

    /**
     * onAttachToCanvas is invoked whenever a device is installed in a canvas
     * (i.e., setDevice, saveLayer (for the new device created by the save),
     * and SkCanvas' SkDevice & SkBitmap -taking ctors). It allows the
     * devices to prepare for drawing (e.g., locking their pixels, etc.)
     */
    virtual void onAttachToCanvas(SkCanvas* canvas) {
        SkASSERT(!fAttachedToCanvas);
        this->lockPixels();
#ifdef SK_DEBUG
        fAttachedToCanvas = true;
#endif
    };

    /**
     * onDetachFromCanvas notifies a device that it will no longer be drawn to.
     * It gives the device a chance to clean up (e.g., unlock its pixels). It
     * is invoked from setDevice (for the displaced device), restore and
     * possibly from SkCanvas' dtor.
     */
    virtual void onDetachFromCanvas() {
        SkASSERT(fAttachedToCanvas);
        this->unlockPixels();
#ifdef SK_DEBUG
        fAttachedToCanvas = false;
#endif
    };

protected:
    enum Usage {
       kGeneral_Usage,
       kSaveLayer_Usage  // <! internal use only
    };

    struct TextFlags {
        uint32_t            fFlags;     // SkPaint::getFlags()
        SkPaint::Hinting    fHinting;
    };

    /**
     *  Device may filter the text flags for drawing text here. If it wants to
     *  make a change to the specified values, it should write them into the
     *  textflags parameter (output) and return true. If the paint is fine as
     *  is, then ignore the textflags parameter and return false.
     *
     *  The baseclass SkDevice filters based on its depth and blitters.
     */
    virtual bool filterTextFlags(const SkPaint& paint, TextFlags*);

    /**
     *  Called with the correct matrix and clip before this device is drawn
     *  to using those settings. If your subclass overrides this, be sure to
     *  call through to the base class as well.
     *
     *  The clipstack is another view of the clip. It records the actual
     *  geometry that went into building the region. It is present for devices
     *  that want to parse it, but is not required: the region is a complete
     *  picture of the current clip. (i.e. if you regionize all of the geometry
     *  in the clipstack, you will arrive at an equivalent region to the one
     *  passed in).
    */
    virtual void setMatrixClip(const SkMatrix&, const SkRegion&,
                               const SkClipStack&);

    /** Called when this device gains focus (i.e becomes the current device
        for drawing).
    */
    virtual void gainFocus(const SkMatrix&, const SkRegion&) {
        SkASSERT(fAttachedToCanvas);
    }

    /** Clears the entire device to the specified color (including alpha).
     *  Ignores the clip.
     */
    virtual void clear(SkColor color);

    /**
     * Deprecated name for clear.
     */
    void eraseColor(SkColor eraseColor) { this->clear(eraseColor); }

    /** These are called inside the per-device-layer loop for each draw call.
     When these are called, we have already applied any saveLayer operations,
     and are handling any looping from the paint, and any effects from the
     DrawFilter.
     */
    virtual void drawPaint(const SkDraw&, const SkPaint& paint);
    virtual void drawPoints(const SkDraw&, SkCanvas::PointMode mode, size_t count,
                            const SkPoint[], const SkPaint& paint);
    virtual void drawRect(const SkDraw&, const SkRect& r,
                          const SkPaint& paint);
    /**
     *  If pathIsMutable, then the implementation is allowed to cast path to a
     *  non-const pointer and modify it in place (as an optimization). Canvas
     *  may do this to implement helpers such as drawOval, by placing a temp
     *  path on the stack to hold the representation of the oval.
     *
     *  If prePathMatrix is not null, it should logically be applied before any
     *  stroking or other effects. If there are no effects on the paint that
     *  affect the geometry/rasterization, then the pre matrix can just be
     *  pre-concated with the current matrix.
     */
    virtual void drawPath(const SkDraw&, const SkPath& path,
                          const SkPaint& paint,
                          const SkMatrix* prePathMatrix = NULL,
                          bool pathIsMutable = false);
    virtual void drawBitmap(const SkDraw&, const SkBitmap& bitmap,
                            const SkIRect* srcRectOrNull,
                            const SkMatrix& matrix, const SkPaint& paint);
    virtual void drawSprite(const SkDraw&, const SkBitmap& bitmap,
                            int x, int y, const SkPaint& paint);
    /**
     *  Does not handle text decoration.
     *  Decorations (underline and stike-thru) will be handled by SkCanvas.
     */
    virtual void drawText(const SkDraw&, const void* text, size_t len,
                          SkScalar x, SkScalar y, const SkPaint& paint);
    virtual void drawPosText(const SkDraw&, const void* text, size_t len,
                             const SkScalar pos[], SkScalar constY,
                             int scalarsPerPos, const SkPaint& paint);
    virtual void drawTextOnPath(const SkDraw&, const void* text, size_t len,
                                const SkPath& path, const SkMatrix* matrix,
                                const SkPaint& paint);
#ifdef SK_BUILD_FOR_ANDROID
    virtual void drawPosTextOnPath(const SkDraw& draw, const void* text, size_t len,
                                   const SkPoint pos[], const SkPaint& paint,
                                   const SkPath& path, const SkMatrix* matrix);
#endif
    virtual void drawVertices(const SkDraw&, SkCanvas::VertexMode, int vertexCount,
                              const SkPoint verts[], const SkPoint texs[],
                              const SkColor colors[], SkXfermode* xmode,
                              const uint16_t indices[], int indexCount,
                              const SkPaint& paint);
    /** The SkDevice passed will be an SkDevice which was returned by a call to
        onCreateCompatibleDevice on this device with kSaveLayer_Usage.
     */
    virtual void drawDevice(const SkDraw&, SkDevice*, int x, int y,
                            const SkPaint&);

    /**
     *  On success (returns true), copy the device pixels into the bitmap.
     *  On failure, the bitmap parameter is left unchanged and false is
     *  returned.
     *
     *  The device's pixels are converted to the bitmap's config. The only
     *  supported config is kARGB_8888_Config, though this is likely to be
     *  relaxed in  the future. The meaning of config kARGB_8888_Config is
     *  modified by the enum param config8888. The default value interprets
     *  kARGB_8888_Config as SkPMColor
     *
     *  If the bitmap has pixels already allocated, the device pixels will be
     *  written there. If not, bitmap->allocPixels() will be called
     *  automatically. If the bitmap is backed by a texture readPixels will
     *  fail.
     *
     *  The actual pixels written is the intersection of the device's bounds,
     *  and the rectangle formed by the bitmap's width,height and the specified
     *  x,y. If bitmap pixels extend outside of that intersection, they will not
     *  be modified.
     *
     *  Other failure conditions:
     *    * If the device is not a raster device (e.g. PDF) then readPixels will
     *      fail.
     *    * If bitmap is texture-backed then readPixels will fail. (This may be
     *      relaxed in the future.)
     */
    bool readPixels(SkBitmap* bitmap,
                    int x, int y,
                    SkCanvas::Config8888 config8888);

    ///////////////////////////////////////////////////////////////////////////

    /** Update as needed the pixel value in the bitmap, so that the caller can
        access the pixels directly. Note: only the pixels field should be
        altered. The config/width/height/rowbytes must remain unchanged.
        @param bitmap The device's bitmap
        @return Echo the bitmap parameter, or an alternate (shadow) bitmap
            maintained by the subclass.
    */
    virtual const SkBitmap& onAccessBitmap(SkBitmap*);

    SkPixelRef* getPixelRef() const { return fBitmap.pixelRef(); }
    // just for subclasses, to assign a custom pixelref
    SkPixelRef* setPixelRef(SkPixelRef* pr, size_t offset) {
        fBitmap.setPixelRef(pr, offset);
        return pr;
    }

    /**
     * Implements readPixels API. The caller will ensure that:
     *  1. bitmap has pixel config kARGB_8888_Config.
     *  2. bitmap has pixels.
     *  3. The rectangle (x, y, x + bitmap->width(), y + bitmap->height()) is
     *     contained in the device bounds.
     */
    virtual bool onReadPixels(const SkBitmap& bitmap,
                              int x, int y,
                              SkCanvas::Config8888 config8888);

    /** Called when this device is installed into a Canvas. Balanaced by a call
        to unlockPixels() when the device is removed from a Canvas.
    */
    virtual void lockPixels();
    virtual void unlockPixels();

    /**
     *  Returns true if the device allows processing of this imagefilter. If
     *  false is returned, then the filter is ignored. This may happen for
     *  some subclasses that do not support pixel manipulations after drawing
     *  has occurred (e.g. printing). The default implementation returns true.
     */
    virtual bool allowImageFilter(SkImageFilter*);

    /**
     *  Override and return true for filters that the device can handle
     *  intrinsically. Doing so means that SkCanvas will pass-through this
     *  filter to drawSprite and drawDevice (and potentially filterImage).
     *  Returning false means the SkCanvas will have apply the filter itself,
     *  and just pass the resulting image to the device.
     */
    virtual bool canHandleImageFilter(SkImageFilter*);

    /**
     *  Related (but not required) to canHandleImageFilter, this method returns
     *  true if the device could apply the filter to the src bitmap and return
     *  the result (and updates offset as needed).
     *  If the device does not recognize or support this filter,
     *  it just returns false and leaves result and offset unchanged.
     */
    virtual bool filterImage(SkImageFilter*, const SkBitmap&, const SkMatrix&,
                             SkBitmap* result, SkIPoint* offset);

    // This is equal kBGRA_Premul_Config8888 or kRGBA_Premul_Config8888 if
    // either is identical to kNative_Premul_Config8888. Otherwise, -1.
    static const SkCanvas::Config8888 kPMColorAlias;

private:
    friend class SkCanvas;
    friend struct DeviceCM; //for setMatrixClip
    friend class SkDraw;
    friend class SkDrawIter;
    friend class SkDeviceFilteredPaint;
    friend class DeviceImageFilterProxy;

    friend class SkSurface_Raster;
    // used to change the backend's pixels (and possibly config/rowbytes)
    // but cannot change the width/height, so there should be no change to
    // any clip information.
    void replaceBitmapBackendForRasterSurface(const SkBitmap&);

    // just called by SkCanvas when built as a layer
    void setOrigin(int x, int y) { fOrigin.set(x, y); }
    // just called by SkCanvas for saveLayer
    SkDevice* createCompatibleDeviceForSaveLayer(SkBitmap::Config config,
                                                 int width, int height,
                                                 bool isOpaque);

    /**
     * Subclasses should override this to implement createCompatibleDevice.
     */
    virtual SkDevice* onCreateCompatibleDevice(SkBitmap::Config config,
                                               int width, int height,
                                               bool isOpaque,
                                               Usage usage);

    /** Causes any deferred drawing to the device to be completed.
     */
    virtual void flush() {}

    SkBitmap    fBitmap;
    SkIPoint    fOrigin;
    SkMetaData* fMetaData;

#ifdef SK_DEBUG
    bool        fAttachedToCanvas;
#endif

    typedef SkRefCnt INHERITED;
};

#endif
