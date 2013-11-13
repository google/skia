
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmapDevice_DEFINED
#define SkBitmapDevice_DEFINED

#include "SkDevice.h"

///////////////////////////////////////////////////////////////////////////////
class SK_API SkBitmapDevice : public SkBaseDevice {
public:
    SK_DECLARE_INST_COUNT(SkBitmapDevice)

    /**
     *  Construct a new device with the specified bitmap as its backend. It is
     *  valid for the bitmap to have no pixels associated with it. In that case,
     *  any drawing to this device will have no effect.
    */
    SkBitmapDevice(const SkBitmap& bitmap);

    /**
     *  Construct a new device with the specified bitmap as its backend. It is
     *  valid for the bitmap to have no pixels associated with it. In that case,
     *  any drawing to this device will have no effect.
    */
    SkBitmapDevice(const SkBitmap& bitmap, const SkDeviceProperties& deviceProperties);

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
    SkBitmapDevice(SkBitmap::Config config, int width, int height, bool isOpaque = false);

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
     *  @param deviceProperties Properties which affect compositing.
     */
    SkBitmapDevice(SkBitmap::Config config, int width, int height, bool isOpaque,
                   const SkDeviceProperties& deviceProperties);

    virtual ~SkBitmapDevice();

    virtual uint32_t getDeviceCapabilities() SK_OVERRIDE { return 0; }

    /** Return the width of the device (in pixels).
    */
    virtual int width() const SK_OVERRIDE { return fBitmap.width(); }
    /** Return the height of the device (in pixels).
    */
    virtual int height() const SK_OVERRIDE { return fBitmap.height(); }

    /** Returns true if the device's bitmap's config treats every pixels as
        implicitly opaque.
    */
    virtual bool isOpaque() const SK_OVERRIDE { return fBitmap.isOpaque(); }

    /** Return the bitmap config of the device's pixels
    */
    virtual SkBitmap::Config config() const SK_OVERRIDE { return fBitmap.config(); }

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
                             SkCanvas::Config8888 config8888) SK_OVERRIDE;

    /**
     * Return the device's associated gpu render target, or NULL.
     */
    virtual GrRenderTarget* accessRenderTarget() SK_OVERRIDE { return NULL; }

protected:
    /**
     *  Device may filter the text flags for drawing text here. If it wants to
     *  make a change to the specified values, it should write them into the
     *  textflags parameter (output) and return true. If the paint is fine as
     *  is, then ignore the textflags parameter and return false.
     *
     *  The baseclass SkDevice filters based on its depth and blitters.
     */
    virtual bool filterTextFlags(const SkPaint& paint, TextFlags*) SK_OVERRIDE;

    /** Clears the entire device to the specified color (including alpha).
     *  Ignores the clip.
     */
    virtual void clear(SkColor color) SK_OVERRIDE;

    /** These are called inside the per-device-layer loop for each draw call.
     When these are called, we have already applied any saveLayer operations,
     and are handling any looping from the paint, and any effects from the
     DrawFilter.
     */
    virtual void drawPaint(const SkDraw&, const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPoints(const SkDraw&, SkCanvas::PointMode mode, size_t count,
                            const SkPoint[], const SkPaint& paint) SK_OVERRIDE;
    virtual void drawRect(const SkDraw&, const SkRect& r,
                          const SkPaint& paint) SK_OVERRIDE;
    virtual void drawOval(const SkDraw&, const SkRect& oval,
                          const SkPaint& paint) SK_OVERRIDE;
    virtual void drawRRect(const SkDraw&, const SkRRect& rr,
                           const SkPaint& paint) SK_OVERRIDE;

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
                          bool pathIsMutable = false) SK_OVERRIDE;
    virtual void drawBitmap(const SkDraw&, const SkBitmap& bitmap,
                            const SkMatrix& matrix, const SkPaint& paint) SK_OVERRIDE;
    virtual void drawSprite(const SkDraw&, const SkBitmap& bitmap,
                            int x, int y, const SkPaint& paint) SK_OVERRIDE;

    /**
     *  The default impl. will create a bitmap-shader from the bitmap,
     *  and call drawRect with it.
     */
    virtual void drawBitmapRect(const SkDraw&, const SkBitmap&,
                                const SkRect* srcOrNull, const SkRect& dst,
                                const SkPaint& paint,
                                SkCanvas::DrawBitmapRectFlags flags) SK_OVERRIDE;

    /**
     *  Does not handle text decoration.
     *  Decorations (underline and stike-thru) will be handled by SkCanvas.
     */
    virtual void drawText(const SkDraw&, const void* text, size_t len,
                          SkScalar x, SkScalar y, const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPosText(const SkDraw&, const void* text, size_t len,
                             const SkScalar pos[], SkScalar constY,
                             int scalarsPerPos, const SkPaint& paint) SK_OVERRIDE;
    virtual void drawTextOnPath(const SkDraw&, const void* text, size_t len,
                                const SkPath& path, const SkMatrix* matrix,
                                const SkPaint& paint) SK_OVERRIDE;
    virtual void drawVertices(const SkDraw&, SkCanvas::VertexMode, int vertexCount,
                              const SkPoint verts[], const SkPoint texs[],
                              const SkColor colors[], SkXfermode* xmode,
                              const uint16_t indices[], int indexCount,
                              const SkPaint& paint) SK_OVERRIDE;
    /** The SkBaseDevice passed will be an SkBaseDevice which was returned by a call to
        onCreateCompatibleDevice on this device with kSaveLayer_Usage.
     */
    virtual void drawDevice(const SkDraw&, SkBaseDevice*, int x, int y,
                            const SkPaint&) SK_OVERRIDE;

    ///////////////////////////////////////////////////////////////////////////

    /** Update as needed the pixel value in the bitmap, so that the caller can
        access the pixels directly. Note: only the pixels field should be
        altered. The config/width/height/rowbytes must remain unchanged.
        @return the device contents as a bitmap
    */
    virtual const SkBitmap& onAccessBitmap() SK_OVERRIDE;

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
                              SkCanvas::Config8888 config8888) SK_OVERRIDE;

    /** Called when this device is installed into a Canvas. Balanced by a call
        to unlockPixels() when the device is removed from a Canvas.
    */
    virtual void lockPixels() SK_OVERRIDE;
    virtual void unlockPixels() SK_OVERRIDE;

    /**
     *  Returns true if the device allows processing of this imagefilter. If
     *  false is returned, then the filter is ignored. This may happen for
     *  some subclasses that do not support pixel manipulations after drawing
     *  has occurred (e.g. printing). The default implementation returns true.
     */
    virtual bool allowImageFilter(SkImageFilter*) SK_OVERRIDE;

    /**
     *  Override and return true for filters that the device can handle
     *  intrinsically. Doing so means that SkCanvas will pass-through this
     *  filter to drawSprite and drawDevice (and potentially filterImage).
     *  Returning false means the SkCanvas will have apply the filter itself,
     *  and just pass the resulting image to the device.
     */
    virtual bool canHandleImageFilter(SkImageFilter*) SK_OVERRIDE;

    /**
     *  Related (but not required) to canHandleImageFilter, this method returns
     *  true if the device could apply the filter to the src bitmap and return
     *  the result (and updates offset as needed).
     *  If the device does not recognize or support this filter,
     *  it just returns false and leaves result and offset unchanged.
     */
    virtual bool filterImage(SkImageFilter*, const SkBitmap&, const SkMatrix&,
                             SkBitmap* result, SkIPoint* offset) SK_OVERRIDE;

private:
    friend class SkCanvas;
    friend struct DeviceCM; //for setMatrixClip
    friend class SkDraw;
    friend class SkDrawIter;
    friend class SkDeviceFilteredPaint;
    friend class SkDeviceImageFilterProxy;

    friend class SkSurface_Raster;

    // used to change the backend's pixels (and possibly config/rowbytes)
    // but cannot change the width/height, so there should be no change to
    // any clip information.
    virtual void replaceBitmapBackendForRasterSurface(const SkBitmap&) SK_OVERRIDE;

    /**
     * Subclasses should override this to implement createCompatibleDevice.
     */
    virtual SkBaseDevice* onCreateCompatibleDevice(SkBitmap::Config config,
                                                   int width, int height,
                                                   bool isOpaque,
                                                   Usage usage) SK_OVERRIDE;

    /** Causes any deferred drawing to the device to be completed.
     */
    virtual void flush() SK_OVERRIDE {}

    SkBitmap    fBitmap;

    typedef SkBaseDevice INHERITED;
};

#endif // SkBitmapDevice_DEFINED
