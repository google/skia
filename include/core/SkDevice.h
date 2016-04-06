/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDevice_DEFINED
#define SkDevice_DEFINED

#include "SkRefCnt.h"
#include "SkCanvas.h"
#include "SkColor.h"
#include "SkImageFilter.h"
#include "SkSurfaceProps.h"

class SkBitmap;
class SkClipStack;
class SkDraw;
class SkDrawFilter;
struct SkIRect;
class SkMatrix;
class SkMetaData;
class SkRegion;
class GrRenderTarget;

class SK_API SkBaseDevice : public SkRefCnt {
public:
    /**
     *  Construct a new device.
    */
    explicit SkBaseDevice(const SkSurfaceProps&);
    virtual ~SkBaseDevice();

    SkMetaData& getMetaData();

    /**
     *  Return ImageInfo for this device. If the canvas is not backed by pixels
     *  (cpu or gpu), then the info's ColorType will be kUnknown_SkColorType.
     */
    virtual SkImageInfo imageInfo() const;

    /**
     *  Return SurfaceProps for this device.
     */
    const SkSurfaceProps& surfaceProps() const {
        return fSurfaceProps;
    }

    /**
     *  Return the bounds of the device in the coordinate space of the root
     *  canvas. The root device will have its top-left at 0,0, but other devices
     *  such as those associated with saveLayer may have a non-zero origin.
     */
    void getGlobalBounds(SkIRect* bounds) const {
        SkASSERT(bounds);
        const SkIPoint& origin = this->getOrigin();
        bounds->setXYWH(origin.x(), origin.y(), this->width(), this->height());
    }

    SkIRect getGlobalBounds() const {
        SkIRect bounds;
        this->getGlobalBounds(&bounds);
        return bounds;
    }

    int width() const {
        return this->imageInfo().width();
    }

    int height() const {
        return this->imageInfo().height();
    }

    bool isOpaque() const {
        return this->imageInfo().isOpaque();
    }

    /** Return the bitmap associated with this device. Call this each time you need
        to access the bitmap, as it notifies the subclass to perform any flushing
        etc. before you examine the pixels.
        @param changePixels set to true if the caller plans to change the pixels
        @return the device's bitmap
    */
    const SkBitmap& accessBitmap(bool changePixels);

    bool writePixels(const SkImageInfo&, const void*, size_t rowBytes, int x, int y);

    /**
     *  Try to get write-access to the pixels behind the device. If successful, this returns true
     *  and fills-out the pixmap parameter. On success it also bumps the genID of the underlying
     *  bitmap.
     *
     *  On failure, returns false and ignores the pixmap parameter.
     */
    bool accessPixels(SkPixmap* pmap);

    /**
     *  Try to get read-only-access to the pixels behind the device. If successful, this returns
     *  true and fills-out the pixmap parameter.
     *
     *  On failure, returns false and ignores the pixmap parameter.
     */
    bool peekPixels(SkPixmap*);

    /**
     * Return the device's associated gpu render target, or NULL.
     */
    virtual GrRenderTarget* accessRenderTarget() { return NULL; }


    /**
     *  Return the device's origin: its offset in device coordinates from
     *  the default origin in its canvas' matrix/clip
     */
    const SkIPoint& getOrigin() const { return fOrigin; }

    /**
     * onAttachToCanvas is invoked whenever a device is installed in a canvas
     * (i.e., setDevice, saveLayer (for the new device created by the save),
     * and SkCanvas' SkBaseDevice & SkBitmap -taking ctors). It allows the
     * devices to prepare for drawing (e.g., locking their pixels, etc.)
     */
    virtual void onAttachToCanvas(SkCanvas*) {
        SkASSERT(!fAttachedToCanvas);
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
#ifdef SK_DEBUG
        fAttachedToCanvas = false;
#endif
    };

protected:
    enum TileUsage {
        kPossible_TileUsage,    //!< the created device may be drawn tiled
        kNever_TileUsage,       //!< the created device will never be drawn tiled
    };

    struct TextFlags {
        uint32_t    fFlags;     // SkPaint::getFlags()
    };

    /**
     * Returns the text-related flags, possibly modified based on the state of the
     * device (e.g. support for LCD).
     */
    uint32_t filterTextFlags(const SkPaint&) const;

    virtual bool onShouldDisableLCD(const SkPaint&) const { return false; }

    /**
     *
     *  DEPRECATED: This will be removed in a future change. Device subclasses
     *  should use the matrix and clip from the SkDraw passed to draw functions.
     *
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
                                const SkClipStack&) {};

    /** These are called inside the per-device-layer loop for each draw call.
     When these are called, we have already applied any saveLayer operations,
     and are handling any looping from the paint, and any effects from the
     DrawFilter.
     */
    virtual void drawPaint(const SkDraw&, const SkPaint& paint) = 0;
    virtual void drawPoints(const SkDraw&, SkCanvas::PointMode mode, size_t count,
                            const SkPoint[], const SkPaint& paint) = 0;
    virtual void drawRect(const SkDraw&, const SkRect& r,
                          const SkPaint& paint) = 0;
    virtual void drawOval(const SkDraw&, const SkRect& oval,
                          const SkPaint& paint) = 0;
    virtual void drawRRect(const SkDraw&, const SkRRect& rr,
                           const SkPaint& paint) = 0;

    // Default impl calls drawPath()
    virtual void drawDRRect(const SkDraw&, const SkRRect& outer,
                            const SkRRect& inner, const SkPaint&);

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
                          bool pathIsMutable = false) = 0;
    virtual void drawBitmap(const SkDraw&, const SkBitmap& bitmap,
                            const SkMatrix& matrix, const SkPaint& paint) = 0;
    virtual void drawSprite(const SkDraw&, const SkBitmap& bitmap,
                            int x, int y, const SkPaint& paint) = 0;

    /**
     *  The default impl. will create a bitmap-shader from the bitmap,
     *  and call drawRect with it.
     */
    virtual void drawBitmapRect(const SkDraw&, const SkBitmap&,
                                const SkRect* srcOrNull, const SkRect& dst,
                                const SkPaint& paint,
                                SkCanvas::SrcRectConstraint) = 0;
    virtual void drawBitmapNine(const SkDraw&, const SkBitmap&, const SkIRect& center,
                               const SkRect& dst, const SkPaint&);

    virtual void drawImage(const SkDraw&, const SkImage*, SkScalar x, SkScalar y, const SkPaint&);
    virtual void drawImageRect(const SkDraw&, const SkImage*, const SkRect* src, const SkRect& dst,
                               const SkPaint&, SkCanvas::SrcRectConstraint);
    virtual void drawImageNine(const SkDraw&, const SkImage*, const SkIRect& center,
                               const SkRect& dst, const SkPaint&);

    /**
     *  Does not handle text decoration.
     *  Decorations (underline and stike-thru) will be handled by SkCanvas.
     */
    virtual void drawText(const SkDraw&, const void* text, size_t len,
                          SkScalar x, SkScalar y, const SkPaint& paint) = 0;
    virtual void drawPosText(const SkDraw&, const void* text, size_t len,
                             const SkScalar pos[], int scalarsPerPos,
                             const SkPoint& offset, const SkPaint& paint) = 0;
    virtual void drawVertices(const SkDraw&, SkCanvas::VertexMode, int vertexCount,
                              const SkPoint verts[], const SkPoint texs[],
                              const SkColor colors[], SkXfermode* xmode,
                              const uint16_t indices[], int indexCount,
                              const SkPaint& paint) = 0;
    // default implementation unrolls the blob runs.
    virtual void drawTextBlob(const SkDraw&, const SkTextBlob*, SkScalar x, SkScalar y,
                              const SkPaint& paint, SkDrawFilter* drawFilter);
    // default implementation calls drawVertices
    virtual void drawPatch(const SkDraw&, const SkPoint cubics[12], const SkColor colors[4],
                           const SkPoint texCoords[4], SkXfermode* xmode, const SkPaint& paint);

    // default implementation calls drawPath
    virtual void drawAtlas(const SkDraw&, const SkImage* atlas, const SkRSXform[], const SkRect[],
                           const SkColor[], int count, SkXfermode::Mode, const SkPaint&);

    virtual void drawAnnotation(const SkDraw&, const SkRect&, const char[], SkData*) {}

    /** The SkDevice passed will be an SkDevice which was returned by a call to
        onCreateDevice on this device with kNeverTile_TileExpectation.
     */
    virtual void drawDevice(const SkDraw&, SkBaseDevice*, int x, int y,
                            const SkPaint&) = 0;

    virtual void drawTextOnPath(const SkDraw&, const void* text, size_t len, const SkPath&,
                                const SkMatrix*, const SkPaint&);

    bool readPixels(const SkImageInfo&, void* dst, size_t rowBytes, int x, int y);

    ///////////////////////////////////////////////////////////////////////////

    /** Update as needed the pixel value in the bitmap, so that the caller can
        access the pixels directly.
        @return The device contents as a bitmap
    */
    virtual const SkBitmap& onAccessBitmap() = 0;

    /**
     *  Override and return true for filters that the device can handle
     *  intrinsically. Doing so means that SkCanvas will pass-through this
     *  filter to drawSprite and drawDevice (and potentially filterImage).
     *  Returning false means the SkCanvas will have apply the filter itself,
     *  and just pass the resulting image to the device.
     */
    virtual bool canHandleImageFilter(const SkImageFilter*) { return false; }

    /**
     *  Related (but not required) to canHandleImageFilter, this method returns
     *  true if the device could apply the filter to the src bitmap and return
     *  the result (and updates offset as needed).
     *  If the device does not recognize or support this filter,
     *  it just returns false and leaves result and offset unchanged.
     */
    virtual bool filterImage(const SkImageFilter*, const SkBitmap&,
                             const SkImageFilter::Context&,
                             SkBitmap* /*result*/, SkIPoint* /*offset*/) {
        return false;
    }

protected:
    virtual sk_sp<SkSurface> makeSurface(const SkImageInfo&, const SkSurfaceProps&);
    virtual bool onPeekPixels(SkPixmap*) { return false; }

    /**
     *  The caller is responsible for "pre-clipping" the dst. The impl can assume that the dst
     *  image at the specified x,y offset will fit within the device's bounds.
     *
     *  This is explicitly asserted in readPixels(), the public way to call this.
     */
    virtual bool onReadPixels(const SkImageInfo&, void*, size_t, int x, int y);

    /**
     *  The caller is responsible for "pre-clipping" the src. The impl can assume that the src
     *  image at the specified x,y offset will fit within the device's bounds.
     *
     *  This is explicitly asserted in writePixelsDirect(), the public way to call this.
     */
    virtual bool onWritePixels(const SkImageInfo&, const void*, size_t, int x, int y);

    virtual bool onAccessPixels(SkPixmap*) { return false; }

    /**
     *  PRIVATE / EXPERIMENTAL -- do not call
     *  This entry point gives the backend an opportunity to take over the rendering
     *  of 'picture'. If optimization data is available (due to an earlier
     *  'optimize' call) this entry point should make use of it and return true
     *  if all rendering has been done. If false is returned, SkCanvas will
     *  perform its own rendering pass. It is acceptable for the backend
     *  to perform some device-specific warm up tasks and then let SkCanvas
     *  perform the main rendering loop (by return false from here).
     */
    virtual bool EXPERIMENTAL_drawPicture(SkCanvas*, const SkPicture*, const SkMatrix*,
                                          const SkPaint*);

    struct CreateInfo {
        static SkPixelGeometry AdjustGeometry(const SkImageInfo&, TileUsage, SkPixelGeometry,
                                              bool preserveLCDText);

        // The constructor may change the pixel geometry based on other parameters.
        CreateInfo(const SkImageInfo& info,
                   TileUsage tileUsage,
                   SkPixelGeometry geo)
            : fInfo(info)
            , fTileUsage(tileUsage)
            , fPixelGeometry(AdjustGeometry(info, tileUsage, geo, false))
            , fForImageFilter(false) {}

        CreateInfo(const SkImageInfo& info,
                   TileUsage tileUsage,
                   SkPixelGeometry geo,
                   bool preserveLCDText,
                   bool forImageFilter)
            : fInfo(info)
            , fTileUsage(tileUsage)
            , fPixelGeometry(AdjustGeometry(info, tileUsage, geo, preserveLCDText))
            , fForImageFilter(forImageFilter) {}

        const SkImageInfo       fInfo;
        const TileUsage         fTileUsage;
        const SkPixelGeometry   fPixelGeometry;
        const bool              fForImageFilter;
    };

    /**
     *  Create a new device based on CreateInfo. If the paint is not null, then it represents a
     *  preview of how the new device will be composed with its creator device (this).
     *
     *  The subclass may be handed this device in drawDevice(), so it must always return
     *  a device that it knows how to draw, and that it knows how to identify if it is not of the
     *  same subclass (since drawDevice is passed a SkBaseDevice*). If the subclass cannot fulfill
     *  that contract (e.g. PDF cannot support some settings on the paint) it should return NULL,
     *  and the caller may then decide to explicitly create a bitmapdevice, knowing that later
     *  it could not call drawDevice with it (but it could call drawSprite or drawBitmap).
     */
    virtual SkBaseDevice* onCreateDevice(const CreateInfo&, const SkPaint*) {
        return NULL;
    }

    /**
     *  Calls through to drawSprite, processing the imagefilter.
     */
    virtual void drawSpriteWithFilter(const SkDraw&, const SkBitmap&,
                                      int x, int y, const SkPaint&);

private:
    friend class SkCanvas;
    friend struct DeviceCM; //for setMatrixClip
    friend class SkDraw;
    friend class SkDrawIter;
    friend class SkDeviceFilteredPaint;
    friend class SkImageFilter::DeviceProxy;
    friend class SkNoPixelsBitmapDevice;
    friend class SkSurface_Raster;

    // used to change the backend's pixels (and possibly config/rowbytes)
    // but cannot change the width/height, so there should be no change to
    // any clip information.
    // TODO: move to SkBitmapDevice
    virtual void replaceBitmapBackendForRasterSurface(const SkBitmap&) {}

    virtual bool forceConservativeRasterClip() const { return false; }

    // just called by SkCanvas when built as a layer
    void setOrigin(int x, int y) { fOrigin.set(x, y); }

    /** Causes any deferred drawing to the device to be completed.
     */
    virtual void flush() {}

    virtual SkImageFilter::Cache* getImageFilterCache() { return NULL; }

    SkIPoint    fOrigin;
    SkMetaData* fMetaData;
    SkSurfaceProps fSurfaceProps;

#ifdef SK_DEBUG
    bool        fAttachedToCanvas;
#endif

    typedef SkRefCnt INHERITED;
};

#endif
