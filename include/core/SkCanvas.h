/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCanvas_DEFINED
#define SkCanvas_DEFINED

#include "SkTypes.h"
#include "SkBitmap.h"
#include "SkDeque.h"
#include "SkClipStack.h"
#include "SkPaint.h"
#include "SkRefCnt.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkXfermode.h"

#ifdef SK_SUPPORT_LEGACY_DRAWTEXT_VIRTUAL
    #define SK_LEGACY_DRAWTEXT_VIRTUAL  virtual
#else
    #define SK_LEGACY_DRAWTEXT_VIRTUAL
#endif

class SkCanvasClipVisitor;
class SkBaseDevice;
class SkDraw;
class SkDrawFilter;
class SkMetaData;
class SkPicture;
class SkRRect;
class SkSurface;
class SkSurface_Base;
class SkTextBlob;
class GrContext;
class GrRenderTarget;

/** \class SkCanvas

    A Canvas encapsulates all of the state about drawing into a device (bitmap).
    This includes a reference to the device itself, and a stack of matrix/clip
    values. For any given draw call (e.g. drawRect), the geometry of the object
    being drawn is transformed by the concatenation of all the matrices in the
    stack. The transformed geometry is clipped by the intersection of all of
    the clips in the stack.

    While the Canvas holds the state of the drawing device, the state (style)
    of the object being drawn is held by the Paint, which is provided as a
    parameter to each of the draw() methods. The Paint holds attributes such as
    color, typeface, textSize, strokeWidth, shader (e.g. gradients, patterns),
    etc.
*/
class SK_API SkCanvas : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(SkCanvas)

    /**
     *  Attempt to allocate an offscreen raster canvas, matching the ImageInfo.
     *  On success, return a new canvas that will draw into that offscreen.
     *
     *  The caller can access the pixels after drawing into this canvas by
     *  calling readPixels() or peekPixels().
     *
     *  If the requested ImageInfo is opaque (either the colortype is
     *  intrinsically opaque like RGB_565, or the info's alphatype is kOpaque)
     *  then the pixel memory may be uninitialized. Otherwise, the pixel memory
     *  will be initialized to 0, which is interpreted as transparent.
     *
     *  On failure, return NULL. This can fail for several reasons:
     *  1. the memory allocation failed (e.g. request is too large)
     *  2. invalid ImageInfo (e.g. negative dimensions)
     *  3. unsupported ImageInfo for a canvas
     *      - kUnknown_SkColorType, kIndex_8_SkColorType
     *      - kIgnore_SkAlphaType
     *      - this list is not complete, so others may also be unsupported
     *
     *  Note: it is valid to request a supported ImageInfo, but with zero
     *  dimensions.
     */
    static SkCanvas* NewRaster(const SkImageInfo&);

    static SkCanvas* NewRasterN32(int width, int height) {
        return NewRaster(SkImageInfo::MakeN32Premul(width, height));
    }

    /**
     *  Attempt to allocate raster canvas, matching the ImageInfo, that will draw directly into the
     *  specified pixels. To access the pixels after drawing to them, the caller should call
     *  flush() or call peekPixels(...).
     *
     *  On failure, return NULL. This can fail for several reasons:
     *  1. invalid ImageInfo (e.g. negative dimensions)
     *  2. unsupported ImageInfo for a canvas
     *      - kUnknown_SkColorType, kIndex_8_SkColorType
     *      - kIgnore_SkAlphaType
     *      - this list is not complete, so others may also be unsupported
     *
     *  Note: it is valid to request a supported ImageInfo, but with zero
     *  dimensions.
     */
    static SkCanvas* NewRasterDirect(const SkImageInfo&, void*, size_t);

    static SkCanvas* NewRasterDirectN32(int width, int height, SkPMColor* pixels, size_t rowBytes) {
        return NewRasterDirect(SkImageInfo::MakeN32Premul(width, height), pixels, rowBytes);
    }

    /**
     *  Creates an empty canvas with no backing device/pixels, and zero
     *  dimensions.
     */
    SkCanvas();

    /**
     *  Creates a canvas of the specified dimensions, but explicitly not backed
     *  by any device/pixels. Typically this use used by subclasses who handle
     *  the draw calls in some other way.
     */
    SkCanvas(int width, int height);

    /** Construct a canvas with the specified device to draw into.

        @param device   Specifies a device for the canvas to draw into.
    */
    explicit SkCanvas(SkBaseDevice* device);

    /** Construct a canvas with the specified bitmap to draw into.
        @param bitmap   Specifies a bitmap for the canvas to draw into. Its
                        structure are copied to the canvas.
    */
    explicit SkCanvas(const SkBitmap& bitmap);
    virtual ~SkCanvas();

    SkMetaData& getMetaData();

    /**
     *  Return ImageInfo for this canvas. If the canvas is not backed by pixels
     *  (cpu or gpu), then the info's ColorType will be kUnknown_SkColorType.
     */
    SkImageInfo imageInfo() const;

    ///////////////////////////////////////////////////////////////////////////

    /**
     *  Trigger the immediate execution of all pending draw operations.
     */
    void flush();

    /**
     * Gets the size of the base or root layer in global canvas coordinates. The
     * origin of the base layer is always (0,0). The current drawable area may be
     * smaller (due to clipping or saveLayer).
     */
    SkISize getBaseLayerSize() const;

    /**
     *  DEPRECATED: call getBaseLayerSize
     */
    SkISize getDeviceSize() const { return this->getBaseLayerSize(); }

    /**
     *  DEPRECATED.
     *  Return the canvas' device object, which may be null. The device holds
     *  the bitmap of the pixels that the canvas draws into. The reference count
     *  of the returned device is not changed by this call.
     */
#ifndef SK_SUPPORT_LEGACY_GETDEVICE
protected:  // Can we make this private?
#endif
    SkBaseDevice* getDevice() const;
public:

    /**
     *  saveLayer() can create another device (which is later drawn onto
     *  the previous device). getTopDevice() returns the top-most device current
     *  installed. Note that this can change on other calls like save/restore,
     *  so do not access this device after subsequent canvas calls.
     *  The reference count of the device is not changed.
     *
     * @param updateMatrixClip If this is true, then before the device is
     *        returned, we ensure that its has been notified about the current
     *        matrix and clip. Note: this happens automatically when the device
     *        is drawn to, but is optional here, as there is a small perf hit
     *        sometimes.
     */
#ifndef SK_SUPPORT_LEGACY_GETTOPDEVICE
private:
#endif
    SkBaseDevice* getTopDevice(bool updateMatrixClip = false) const;
public:

    /**
     *  Create a new surface matching the specified info, one that attempts to
     *  be maximally compatible when used with this canvas. If there is no matching Surface type,
     *  NULL is returned.
     */
    SkSurface* newSurface(const SkImageInfo&);

    /**
     * Return the GPU context of the device that is associated with the canvas.
     * For a canvas with non-GPU device, NULL is returned.
     */
    GrContext* getGrContext();

    ///////////////////////////////////////////////////////////////////////////

    /**
     *  If the canvas has writable pixels in its top layer (and is not recording to a picture
     *  or other non-raster target) and has direct access to its pixels (i.e. they are in
     *  local RAM) return the address of those pixels, and if not null,
     *  return the ImageInfo, rowBytes and origin. The returned address is only valid
     *  while the canvas object is in scope and unchanged. Any API calls made on
     *  canvas (or its parent surface if any) will invalidate the
     *  returned address (and associated information).
     *
     *  On failure, returns NULL and the info, rowBytes, and origin parameters are ignored.
     */
    void* accessTopLayerPixels(SkImageInfo* info, size_t* rowBytes, SkIPoint* origin = NULL);

    /**
     *  If the canvas has readable pixels in its base layer (and is not recording to a picture
     *  or other non-raster target) and has direct access to its pixels (i.e. they are in
     *  local RAM) return the const-address of those pixels, and if not null,
     *  return the ImageInfo and rowBytes. The returned address is only valid
     *  while the canvas object is in scope and unchanged. Any API calls made on
     *  canvas (or its parent surface if any) will invalidate the
     *  returned address (and associated information).
     *
     *  On failure, returns NULL and the info and rowBytes parameters are
     *  ignored.
     */
    const void* peekPixels(SkImageInfo* info, size_t* rowBytes);

    /**
     *  Copy the pixels from the base-layer into the specified buffer (pixels + rowBytes),
     *  converting them into the requested format (SkImageInfo). The base-layer pixels are read
     *  starting at the specified (srcX,srcY) location in the coordinate system of the base-layer.
     *
     *  The specified ImageInfo and (srcX,srcY) offset specifies a source rectangle
     *
     *      srcR.setXYWH(srcX, srcY, dstInfo.width(), dstInfo.height());
     *
     *  srcR is intersected with the bounds of the base-layer. If this intersection is not empty,
     *  then we have two sets of pixels (of equal size). Replace the dst pixels with the
     *  corresponding src pixels, performing any colortype/alphatype transformations needed
     *  (in the case where the src and dst have different colortypes or alphatypes).
     *
     *  This call can fail, returning false, for several reasons:
     *  - If srcR does not intersect the base-layer bounds.
     *  - If the requested colortype/alphatype cannot be converted from the base-layer's types.
     *  - If this canvas is not backed by pixels (e.g. picture or PDF)
     */
    bool readPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes,
                    int srcX, int srcY);

    /**
     *  Helper for calling readPixels(info, ...). This call will check if bitmap has been allocated.
     *  If not, it will attempt to call allocPixels(). If this fails, it will return false. If not,
     *  it calls through to readPixels(info, ...) and returns its result.
     */
    bool readPixels(SkBitmap* bitmap, int srcX, int srcY);

    /**
     *  Helper for allocating pixels and then calling readPixels(info, ...). The bitmap is resized
     *  to the intersection of srcRect and the base-layer bounds. On success, pixels will be
     *  allocated in bitmap and true returned. On failure, false is returned and bitmap will be
     *  set to empty.
     */
    bool readPixels(const SkIRect& srcRect, SkBitmap* bitmap);

    /**
     *  This method affects the pixels in the base-layer, and operates in pixel coordinates,
     *  ignoring the matrix and clip.
     *
     *  The specified ImageInfo and (x,y) offset specifies a rectangle: target.
     *
     *      target.setXYWH(x, y, info.width(), info.height());
     *
     *  Target is intersected with the bounds of the base-layer. If this intersection is not empty,
     *  then we have two sets of pixels (of equal size), the "src" specified by info+pixels+rowBytes
     *  and the "dst" by the canvas' backend. Replace the dst pixels with the corresponding src
     *  pixels, performing any colortype/alphatype transformations needed (in the case where the
     *  src and dst have different colortypes or alphatypes).
     *
     *  This call can fail, returning false, for several reasons:
     *  - If the src colortype/alphatype cannot be converted to the canvas' types
     *  - If this canvas is not backed by pixels (e.g. picture or PDF)
     */
    bool writePixels(const SkImageInfo&, const void* pixels, size_t rowBytes, int x, int y);

    /**
     *  Helper for calling writePixels(info, ...) by passing its pixels and rowbytes. If the bitmap
     *  is just wrapping a texture, returns false and does nothing.
     */
    bool writePixels(const SkBitmap& bitmap, int x, int y);

    ///////////////////////////////////////////////////////////////////////////

    enum SaveFlags {
        /** save the matrix state, restoring it on restore() */
        // [deprecated] kMatrix_SaveFlag            = 0x01,
        kMatrix_SaveFlag            = 0x01,
        /** save the clip state, restoring it on restore() */
        // [deprecated] kClip_SaveFlag              = 0x02,
        kClip_SaveFlag              = 0x02,
        /** the layer needs to support per-pixel alpha */
        kHasAlphaLayer_SaveFlag     = 0x04,
        /** the layer needs to support 8-bits per color component */
        kFullColorLayer_SaveFlag    = 0x08,
        /**
         *  the layer should clip against the bounds argument
         *
         *  if SK_SUPPORT_LEGACY_CLIPTOLAYERFLAG is undefined, this is treated as always on.
         */
        kClipToLayer_SaveFlag       = 0x10,

        // helper masks for common choices
        // [deprecated] kMatrixClip_SaveFlag        = 0x03,
        kMatrixClip_SaveFlag        = 0x03,
#ifdef SK_SUPPORT_LEGACY_CLIPTOLAYERFLAG
        kARGB_NoClipLayer_SaveFlag  = 0x0F,
#endif
        kARGB_ClipLayer_SaveFlag    = 0x1F
    };

    /** This call saves the current matrix, clip, and drawFilter, and pushes a
        copy onto a private stack. Subsequent calls to translate, scale,
        rotate, skew, concat or clipRect, clipPath, and setDrawFilter all
        operate on this copy.
        When the balancing call to restore() is made, the previous matrix, clip,
        and drawFilter are restored.

        @return The value to pass to restoreToCount() to balance this save()
    */
    int save();

    /** This behaves the same as save(), but in addition it allocates an
        offscreen bitmap. All drawing calls are directed there, and only when
        the balancing call to restore() is made is that offscreen transfered to
        the canvas (or the previous layer).
        @param bounds (may be null) This rect, if non-null, is used as a hint to
                      limit the size of the offscreen, and thus drawing may be
                      clipped to it, though that clipping is not guaranteed to
                      happen. If exact clipping is desired, use clipRect().
        @param paint (may be null) This is copied, and is applied to the
                     offscreen when restore() is called
        @return The value to pass to restoreToCount() to balance this save()
    */
    int saveLayer(const SkRect* bounds, const SkPaint* paint);

    /** DEPRECATED - use saveLayer(const SkRect*, const SkPaint*) instead.

        This behaves the same as saveLayer(const SkRect*, const SkPaint*),
        but it allows fine-grained control of which state bits to be saved
        (and subsequently restored).

        @param bounds (may be null) This rect, if non-null, is used as a hint to
                      limit the size of the offscreen, and thus drawing may be
                      clipped to it, though that clipping is not guaranteed to
                      happen. If exact clipping is desired, use clipRect().
        @param paint (may be null) This is copied, and is applied to the
                     offscreen when restore() is called
        @param flags  LayerFlags
        @return The value to pass to restoreToCount() to balance this save()
    */
    SK_ATTR_EXTERNALLY_DEPRECATED("SaveFlags use is deprecated")
    int saveLayer(const SkRect* bounds, const SkPaint* paint, SaveFlags flags);

    /** This behaves the same as save(), but in addition it allocates an
        offscreen bitmap. All drawing calls are directed there, and only when
        the balancing call to restore() is made is that offscreen transfered to
        the canvas (or the previous layer).
        @param bounds (may be null) This rect, if non-null, is used as a hint to
                      limit the size of the offscreen, and thus drawing may be
                      clipped to it, though that clipping is not guaranteed to
                      happen. If exact clipping is desired, use clipRect().
        @param alpha  This is applied to the offscreen when restore() is called.
        @return The value to pass to restoreToCount() to balance this save()
    */
    int saveLayerAlpha(const SkRect* bounds, U8CPU alpha);

    /** DEPRECATED - use saveLayerAlpha(const SkRect*, U8CPU) instead.

        This behaves the same as saveLayerAlpha(const SkRect*, U8CPU),
        but it allows fine-grained control of which state bits to be saved
        (and subsequently restored).

        @param bounds (may be null) This rect, if non-null, is used as a hint to
                      limit the size of the offscreen, and thus drawing may be
                      clipped to it, though that clipping is not guaranteed to
                      happen. If exact clipping is desired, use clipRect().
        @param alpha  This is applied to the offscreen when restore() is called.
        @param flags  LayerFlags
        @return The value to pass to restoreToCount() to balance this save()
    */
    SK_ATTR_EXTERNALLY_DEPRECATED("SaveFlags use is deprecated")
    int saveLayerAlpha(const SkRect* bounds, U8CPU alpha, SaveFlags flags);

    /** This call balances a previous call to save(), and is used to remove all
        modifications to the matrix/clip/drawFilter state since the last save
        call.
        It is an error to call restore() more times than save() was called.
    */
    void restore();

    /** Returns the number of matrix/clip states on the SkCanvas' private stack.
        This will equal # save() calls - # restore() calls + 1. The save count on
        a new canvas is 1.
    */
    int getSaveCount() const;

    /** Efficient way to pop any calls to save() that happened after the save
        count reached saveCount. It is an error for saveCount to be greater than
        getSaveCount(). To pop all the way back to the initial matrix/clip context
        pass saveCount == 1.
        @param saveCount    The number of save() levels to restore from
    */
    void restoreToCount(int saveCount);

    /** Returns true if drawing is currently going to a layer (from saveLayer)
     *  rather than to the root device.
     */
    virtual bool isDrawingToLayer() const;

    /** Preconcat the current matrix with the specified translation
        @param dx   The distance to translate in X
        @param dy   The distance to translate in Y
    */
    void translate(SkScalar dx, SkScalar dy);

    /** Preconcat the current matrix with the specified scale.
        @param sx   The amount to scale in X
        @param sy   The amount to scale in Y
    */
    void scale(SkScalar sx, SkScalar sy);

    /** Preconcat the current matrix with the specified rotation.
        @param degrees  The amount to rotate, in degrees
    */
    void rotate(SkScalar degrees);

    /** Preconcat the current matrix with the specified skew.
        @param sx   The amount to skew in X
        @param sy   The amount to skew in Y
    */
    void skew(SkScalar sx, SkScalar sy);

    /** Preconcat the current matrix with the specified matrix.
        @param matrix   The matrix to preconcatenate with the current matrix
    */
    void concat(const SkMatrix& matrix);

    /** Replace the current matrix with a copy of the specified matrix.
        @param matrix The matrix that will be copied into the current matrix.
    */
    void setMatrix(const SkMatrix& matrix);

    /** Helper for setMatrix(identity). Sets the current matrix to identity.
    */
    void resetMatrix();

    /**
     *  Modify the current clip with the specified rectangle.
     *  @param rect The rect to combine with the current clip
     *  @param op The region op to apply to the current clip
     *  @param doAntiAlias true if the clip should be antialiased
     */
    void clipRect(const SkRect& rect,
                  SkRegion::Op op = SkRegion::kIntersect_Op,
                  bool doAntiAlias = false);

    /**
     *  Modify the current clip with the specified SkRRect.
     *  @param rrect The rrect to combine with the current clip
     *  @param op The region op to apply to the current clip
     *  @param doAntiAlias true if the clip should be antialiased
     */
    void clipRRect(const SkRRect& rrect,
                   SkRegion::Op op = SkRegion::kIntersect_Op,
                   bool doAntiAlias = false);

    /**
     *  Modify the current clip with the specified path.
     *  @param path The path to combine with the current clip
     *  @param op The region op to apply to the current clip
     *  @param doAntiAlias true if the clip should be antialiased
     */
    void clipPath(const SkPath& path,
                  SkRegion::Op op = SkRegion::kIntersect_Op,
                  bool doAntiAlias = false);

    /** EXPERIMENTAL -- only used for testing
        Set to false to force clips to be hard, even if doAntiAlias=true is
        passed to clipRect or clipPath.
     */
    void setAllowSoftClip(bool allow) {
        fAllowSoftClip = allow;
    }

    /** EXPERIMENTAL -- only used for testing
        Set to simplify clip stack using path ops.
     */
    void setAllowSimplifyClip(bool allow) {
        fAllowSimplifyClip = allow;
    }

    /** Modify the current clip with the specified region. Note that unlike
        clipRect() and clipPath() which transform their arguments by the current
        matrix, clipRegion() assumes its argument is already in device
        coordinates, and so no transformation is performed.
        @param deviceRgn    The region to apply to the current clip
        @param op The region op to apply to the current clip
    */
    void clipRegion(const SkRegion& deviceRgn,
                    SkRegion::Op op = SkRegion::kIntersect_Op);

    /** Helper for clipRegion(rgn, kReplace_Op). Sets the current clip to the
        specified region. This does not intersect or in any other way account
        for the existing clip region.
        @param deviceRgn The region to copy into the current clip.
    */
    void setClipRegion(const SkRegion& deviceRgn) {
        this->clipRegion(deviceRgn, SkRegion::kReplace_Op);
    }

    /** Return true if the specified rectangle, after being transformed by the
        current matrix, would lie completely outside of the current clip. Call
        this to check if an area you intend to draw into is clipped out (and
        therefore you can skip making the draw calls).
        @param rect the rect to compare with the current clip
        @return true if the rect (transformed by the canvas' matrix) does not
                     intersect with the canvas' clip
    */
    bool quickReject(const SkRect& rect) const;

    /** Return true if the specified path, after being transformed by the
        current matrix, would lie completely outside of the current clip. Call
        this to check if an area you intend to draw into is clipped out (and
        therefore you can skip making the draw calls). Note, for speed it may
        return false even if the path itself might not intersect the clip
        (i.e. the bounds of the path intersects, but the path does not).
        @param path The path to compare with the current clip
        @return true if the path (transformed by the canvas' matrix) does not
                     intersect with the canvas' clip
    */
    bool quickReject(const SkPath& path) const;

    /** Return true if the horizontal band specified by top and bottom is
        completely clipped out. This is a conservative calculation, meaning
        that it is possible that if the method returns false, the band may still
        in fact be clipped out, but the converse is not true. If this method
        returns true, then the band is guaranteed to be clipped out.
        @param top  The top of the horizontal band to compare with the clip
        @param bottom The bottom of the horizontal and to compare with the clip
        @return true if the horizontal band is completely clipped out (i.e. does
                     not intersect the current clip)
    */
    bool quickRejectY(SkScalar top, SkScalar bottom) const {
        SkASSERT(top <= bottom);

#ifndef SK_WILL_NEVER_DRAW_PERSPECTIVE_TEXT
        // TODO: add a hasPerspective method similar to getLocalClipBounds. This
        // would cache the SkMatrix::hasPerspective result. Alternatively, have
        // the MC stack just set a hasPerspective boolean as it is updated.
        if (this->getTotalMatrix().hasPerspective()) {
            // TODO: consider implementing some half-plane test between the
            // two Y planes and the device-bounds (i.e., project the top and
            // bottom Y planes and then determine if the clip bounds is completely
            // outside either one).
            return false;
        }
#endif

        const SkRect& clipR = this->getLocalClipBounds();
        // In the case where the clip is empty and we are provided with a
        // negative top and positive bottom parameter then this test will return
        // false even though it will be clipped. We have chosen to exclude that
        // check as it is rare and would result double the comparisons.
        return top >= clipR.fBottom || bottom <= clipR.fTop;
    }

    /** Return the bounds of the current clip (in local coordinates) in the
        bounds parameter, and return true if it is non-empty. This can be useful
        in a way similar to quickReject, in that it tells you that drawing
        outside of these bounds will be clipped out.
    */
    virtual bool getClipBounds(SkRect* bounds) const;

    /** Return the bounds of the current clip, in device coordinates; returns
        true if non-empty. Maybe faster than getting the clip explicitly and
        then taking its bounds.
    */
    virtual bool getClipDeviceBounds(SkIRect* bounds) const;


    /** Fill the entire canvas' bitmap (restricted to the current clip) with the
        specified ARGB color, using the specified mode.
        @param a    the alpha component (0..255) of the color to fill the canvas
        @param r    the red component (0..255) of the color to fill the canvas
        @param g    the green component (0..255) of the color to fill the canvas
        @param b    the blue component (0..255) of the color to fill the canvas
        @param mode the mode to apply the color in (defaults to SrcOver)
    */
    void drawARGB(U8CPU a, U8CPU r, U8CPU g, U8CPU b,
                  SkXfermode::Mode mode = SkXfermode::kSrcOver_Mode);

    /** Fill the entire canvas' bitmap (restricted to the current clip) with the
        specified color and mode.
        @param color    the color to draw with
        @param mode the mode to apply the color in (defaults to SrcOver)
    */
    void drawColor(SkColor color,
                   SkXfermode::Mode mode = SkXfermode::kSrcOver_Mode);

    /**
     *  This erases the entire drawing surface to the specified color,
     *  irrespective of the clip. It does not blend with the previous pixels,
     *  but always overwrites them.
     *
     *  It is roughly equivalent to the following:
     *      canvas.save();
     *      canvas.clipRect(hugeRect, kReplace_Op);
     *      paint.setColor(color);
     *      paint.setXfermodeMode(kSrc_Mode);
     *      canvas.drawPaint(paint);
     *      canvas.restore();
     *  though it is almost always much more efficient.
     */
    virtual void clear(SkColor);

    /**
     * This makes the contents of the canvas undefined. Subsequent calls that
     * require reading the canvas contents will produce undefined results. Examples
     * include blending and readPixels. The actual implementation is backend-
     * dependent and one legal implementation is to do nothing. Like clear(), this
     * ignores the clip.
     *
     * This function should only be called if the caller intends to subsequently
     * draw to the canvas. The canvas may do real work at discard() time in order
     * to optimize performance on subsequent draws. Thus, if you call this and then
     * never draw to the canvas subsequently you may pay a perfomance penalty.
     */
    void discard() { this->onDiscard(); }

    /**
     *  Fill the entire canvas' bitmap (restricted to the current clip) with the
     *  specified paint.
     *  @param paint    The paint used to fill the canvas
     */
    virtual void drawPaint(const SkPaint& paint);

    enum PointMode {
        /** drawPoints draws each point separately */
        kPoints_PointMode,
        /** drawPoints draws each pair of points as a line segment */
        kLines_PointMode,
        /** drawPoints draws the array of points as a polygon */
        kPolygon_PointMode
    };

    /** Draw a series of points, interpreted based on the PointMode mode. For
        all modes, the count parameter is interpreted as the total number of
        points. For kLine mode, count/2 line segments are drawn.
        For kPoint mode, each point is drawn centered at its coordinate, and its
        size is specified by the paint's stroke-width. It draws as a square,
        unless the paint's cap-type is round, in which the points are drawn as
        circles.
        For kLine mode, each pair of points is drawn as a line segment,
        respecting the paint's settings for cap/join/width.
        For kPolygon mode, the entire array is drawn as a series of connected
        line segments.
        Note that, while similar, kLine and kPolygon modes draw slightly
        differently than the equivalent path built with a series of moveto,
        lineto calls, in that the path will draw all of its contours at once,
        with no interactions if contours intersect each other (think XOR
        xfermode). drawPoints always draws each element one at a time.
        @param mode     PointMode specifying how to draw the array of points.
        @param count    The number of points in the array
        @param pts      Array of points to draw
        @param paint    The paint used to draw the points
    */
    virtual void drawPoints(PointMode mode, size_t count, const SkPoint pts[],
                            const SkPaint& paint);

    /** Helper method for drawing a single point. See drawPoints() for a more
        details.
    */
    void drawPoint(SkScalar x, SkScalar y, const SkPaint& paint);

    /** Draws a single pixel in the specified color.
        @param x        The X coordinate of which pixel to draw
        @param y        The Y coordiante of which pixel to draw
        @param color    The color to draw
    */
    void drawPoint(SkScalar x, SkScalar y, SkColor color);

    /** Draw a line segment with the specified start and stop x,y coordinates,
        using the specified paint. NOTE: since a line is always "framed", the
        paint's Style is ignored.
        @param x0    The x-coordinate of the start point of the line
        @param y0    The y-coordinate of the start point of the line
        @param x1    The x-coordinate of the end point of the line
        @param y1    The y-coordinate of the end point of the line
        @param paint The paint used to draw the line
    */
    void drawLine(SkScalar x0, SkScalar y0, SkScalar x1, SkScalar y1,
                  const SkPaint& paint);

    /** Draw the specified rectangle using the specified paint. The rectangle
        will be filled or stroked based on the Style in the paint.
        @param rect     The rect to be drawn
        @param paint    The paint used to draw the rect
    */
    virtual void drawRect(const SkRect& rect, const SkPaint& paint);

    /** Draw the specified rectangle using the specified paint. The rectangle
        will be filled or framed based on the Style in the paint.
        @param rect     The rect to be drawn
        @param paint    The paint used to draw the rect
    */
    void drawIRect(const SkIRect& rect, const SkPaint& paint) {
        SkRect r;
        r.set(rect);    // promotes the ints to scalars
        this->drawRect(r, paint);
    }

    /** Draw the specified rectangle using the specified paint. The rectangle
        will be filled or framed based on the Style in the paint.
        @param left     The left side of the rectangle to be drawn
        @param top      The top side of the rectangle to be drawn
        @param right    The right side of the rectangle to be drawn
        @param bottom   The bottom side of the rectangle to be drawn
        @param paint    The paint used to draw the rect
    */
    void drawRectCoords(SkScalar left, SkScalar top, SkScalar right,
                        SkScalar bottom, const SkPaint& paint);

    /** Draw the specified oval using the specified paint. The oval will be
        filled or framed based on the Style in the paint.
        @param oval     The rectangle bounds of the oval to be drawn
        @param paint    The paint used to draw the oval
    */
    virtual void drawOval(const SkRect& oval, const SkPaint&);

    /**
     *  Draw the specified RRect using the specified paint The rrect will be filled or stroked
     *  based on the Style in the paint.
     *
     *  @param rrect    The round-rect to draw
     *  @param paint    The paint used to draw the round-rect
     */
    virtual void drawRRect(const SkRRect& rrect, const SkPaint& paint);

    /**
     *  Draw the annulus formed by the outer and inner rrects. The results
     *  are undefined if the outer does not contain the inner.
     */
    void drawDRRect(const SkRRect& outer, const SkRRect& inner, const SkPaint&);

    /** Draw the specified circle using the specified paint. If radius is <= 0,
        then nothing will be drawn. The circle will be filled
        or framed based on the Style in the paint.
        @param cx       The x-coordinate of the center of the cirle to be drawn
        @param cy       The y-coordinate of the center of the cirle to be drawn
        @param radius   The radius of the cirle to be drawn
        @param paint    The paint used to draw the circle
    */
    void drawCircle(SkScalar cx, SkScalar cy, SkScalar radius,
                    const SkPaint& paint);

    /** Draw the specified arc, which will be scaled to fit inside the
        specified oval. If the sweep angle is >= 360, then the oval is drawn
        completely. Note that this differs slightly from SkPath::arcTo, which
        treats the sweep angle mod 360.
        @param oval The bounds of oval used to define the shape of the arc
        @param startAngle Starting angle (in degrees) where the arc begins
        @param sweepAngle Sweep angle (in degrees) measured clockwise
        @param useCenter true means include the center of the oval. For filling
                         this will draw a wedge. False means just use the arc.
        @param paint    The paint used to draw the arc
    */
    void drawArc(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle,
                 bool useCenter, const SkPaint& paint);

    /** Draw the specified round-rect using the specified paint. The round-rect
        will be filled or framed based on the Style in the paint.
        @param rect     The rectangular bounds of the roundRect to be drawn
        @param rx       The x-radius of the oval used to round the corners
        @param ry       The y-radius of the oval used to round the corners
        @param paint    The paint used to draw the roundRect
    */
    void drawRoundRect(const SkRect& rect, SkScalar rx, SkScalar ry,
                       const SkPaint& paint);

    /** Draw the specified path using the specified paint. The path will be
        filled or framed based on the Style in the paint.
        @param path     The path to be drawn
        @param paint    The paint used to draw the path
    */
    virtual void drawPath(const SkPath& path, const SkPaint& paint);

    /** Draw the specified bitmap, with its top/left corner at (x,y), using the
        specified paint, transformed by the current matrix. Note: if the paint
        contains a maskfilter that generates a mask which extends beyond the
        bitmap's original width/height, then the bitmap will be drawn as if it
        were in a Shader with CLAMP mode. Thus the color outside of the original
        width/height will be the edge color replicated.

        If a shader is present on the paint it will be ignored, except in the
        case where the bitmap is kAlpha_8_SkColorType. In that case, the color is
        generated by the shader.

        @param bitmap   The bitmap to be drawn
        @param left     The position of the left side of the bitmap being drawn
        @param top      The position of the top side of the bitmap being drawn
        @param paint    The paint used to draw the bitmap, or NULL
    */
    virtual void drawBitmap(const SkBitmap& bitmap, SkScalar left, SkScalar top,
                            const SkPaint* paint = NULL);

    enum DrawBitmapRectFlags {
        kNone_DrawBitmapRectFlag            = 0x0,
        /**
         *  When filtering is enabled, allow the color samples outside of
         *  the src rect (but still in the src bitmap) to bleed into the
         *  drawn portion
         */
        kBleed_DrawBitmapRectFlag           = 0x1,
    };

    /** Draw the specified bitmap, with the specified matrix applied (before the
        canvas' matrix is applied).
        @param bitmap   The bitmap to be drawn
        @param src      Optional: specify the subset of the bitmap to be drawn
        @param dst      The destination rectangle where the scaled/translated
                        image will be drawn
        @param paint    The paint used to draw the bitmap, or NULL
    */
    virtual void drawBitmapRectToRect(const SkBitmap& bitmap, const SkRect* src,
                                      const SkRect& dst,
                                      const SkPaint* paint = NULL,
                                      DrawBitmapRectFlags flags = kNone_DrawBitmapRectFlag);

    void drawBitmapRect(const SkBitmap& bitmap, const SkRect& dst,
                        const SkPaint* paint = NULL) {
        this->drawBitmapRectToRect(bitmap, NULL, dst, paint, kNone_DrawBitmapRectFlag);
    }

    void drawBitmapRect(const SkBitmap& bitmap, const SkIRect* isrc,
                        const SkRect& dst, const SkPaint* paint = NULL,
                        DrawBitmapRectFlags flags = kNone_DrawBitmapRectFlag) {
        SkRect realSrcStorage;
        SkRect* realSrcPtr = NULL;
        if (isrc) {
            realSrcStorage.set(*isrc);
            realSrcPtr = &realSrcStorage;
        }
        this->drawBitmapRectToRect(bitmap, realSrcPtr, dst, paint, flags);
    }

    virtual void drawBitmapMatrix(const SkBitmap& bitmap, const SkMatrix& m,
                                  const SkPaint* paint = NULL);

    /**
     *  Draw the bitmap stretched differentially to fit into dst.
     *  center is a rect within the bitmap, and logically divides the bitmap
     *  into 9 sections (3x3). For example, if the middle pixel of a [5x5]
     *  bitmap is the "center", then the center-rect should be [2, 2, 3, 3].
     *
     *  If the dst is >= the bitmap size, then...
     *  - The 4 corners are not stretched at all.
     *  - The sides are stretched in only one axis.
     *  - The center is stretched in both axes.
     * Else, for each axis where dst < bitmap,
     *  - The corners shrink proportionally
     *  - The sides (along the shrink axis) and center are not drawn
     */
    virtual void drawBitmapNine(const SkBitmap& bitmap, const SkIRect& center,
                                const SkRect& dst, const SkPaint* paint = NULL);

    /** Draw the specified bitmap, with its top/left corner at (x,y),
        NOT transformed by the current matrix. Note: if the paint
        contains a maskfilter that generates a mask which extends beyond the
        bitmap's original width/height, then the bitmap will be drawn as if it
        were in a Shader with CLAMP mode. Thus the color outside of the original
        width/height will be the edge color replicated.
        @param bitmap   The bitmap to be drawn
        @param left     The position of the left side of the bitmap being drawn
        @param top      The position of the top side of the bitmap being drawn
        @param paint    The paint used to draw the bitmap, or NULL
    */
    virtual void drawSprite(const SkBitmap& bitmap, int left, int top,
                            const SkPaint* paint = NULL);

    /** Draw the text, with origin at (x,y), using the specified paint.
        The origin is interpreted based on the Align setting in the paint.
        @param text The text to be drawn
        @param byteLength   The number of bytes to read from the text parameter
        @param x        The x-coordinate of the origin of the text being drawn
        @param y        The y-coordinate of the origin of the text being drawn
        @param paint    The paint used for the text (e.g. color, size, style)
    */
    SK_LEGACY_DRAWTEXT_VIRTUAL void drawText(const void* text, size_t byteLength, SkScalar x,
                          SkScalar y, const SkPaint& paint);

    /** Draw the text, with each character/glyph origin specified by the pos[]
        array. The origin is interpreted by the Align setting in the paint.
        @param text The text to be drawn
        @param byteLength   The number of bytes to read from the text parameter
        @param pos      Array of positions, used to position each character
        @param paint    The paint used for the text (e.g. color, size, style)
        */
    SK_LEGACY_DRAWTEXT_VIRTUAL void drawPosText(const void* text, size_t byteLength,
                             const SkPoint pos[], const SkPaint& paint);

    /** Draw the text, with each character/glyph origin specified by the x
        coordinate taken from the xpos[] array, and the y from the constY param.
        The origin is interpreted by the Align setting in the paint.
        @param text The text to be drawn
        @param byteLength   The number of bytes to read from the text parameter
        @param xpos     Array of x-positions, used to position each character
        @param constY   The shared Y coordinate for all of the positions
        @param paint    The paint used for the text (e.g. color, size, style)
        */
    SK_LEGACY_DRAWTEXT_VIRTUAL void drawPosTextH(const void* text, size_t byteLength,
                              const SkScalar xpos[], SkScalar constY,
                              const SkPaint& paint);

    /** Draw the text, with origin at (x,y), using the specified paint, along
        the specified path. The paint's Align setting determins where along the
        path to start the text.
        @param text The text to be drawn
        @param byteLength   The number of bytes to read from the text parameter
        @param path         The path the text should follow for its baseline
        @param hOffset      The distance along the path to add to the text's
                            starting position
        @param vOffset      The distance above(-) or below(+) the path to
                            position the text
        @param paint        The paint used for the text
    */
    void drawTextOnPathHV(const void* text, size_t byteLength,
                          const SkPath& path, SkScalar hOffset,
                          SkScalar vOffset, const SkPaint& paint);

    /** Draw the text, with origin at (x,y), using the specified paint, along
        the specified path. The paint's Align setting determins where along the
        path to start the text.
        @param text The text to be drawn
        @param byteLength   The number of bytes to read from the text parameter
        @param path         The path the text should follow for its baseline
        @param matrix       (may be null) Applied to the text before it is
                            mapped onto the path
        @param paint        The paint used for the text
        */
    SK_LEGACY_DRAWTEXT_VIRTUAL void drawTextOnPath(const void* text, size_t byteLength,
                                const SkPath& path, const SkMatrix* matrix,
                                const SkPaint& paint);

    /** Draw the text blob, offset by (x,y), using the specified paint.
        @param blob     The text blob to be drawn
        @param x        The x-offset of the text being drawn
        @param y        The y-offset of the text being drawn
        @param paint    The paint used for the text (e.g. color, size, style)
    */
    void drawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y, const SkPaint& paint);

    /** PRIVATE / EXPERIMENTAL -- do not call
        Perform back-end analysis/optimization of a picture. This may attach
        optimization data to the picture which can be used by a later
        drawPicture call.
        @param picture The recorded drawing commands to analyze/optimize
    */
    void EXPERIMENTAL_optimize(const SkPicture* picture);

    /** Draw the picture into this canvas. This method effective brackets the
        playback of the picture's draw calls with save/restore, so the state
        of this canvas will be unchanged after this call.
        @param picture The recorded drawing commands to playback into this
                       canvas.
    */
    void drawPicture(const SkPicture* picture);

    /**
     *  Draw the picture into this canvas.
     *
     *  If matrix is non-null, apply that matrix to the CTM when drawing this picture. This is
     *  logically equivalent to
     *      save/concat/drawPicture/restore
     *
     *  If paint is non-null, draw the picture into a temporary buffer, and then apply the paint's
     *  alpha/colorfilter/imagefilter/xfermode to that buffer as it is drawn to the canvas.
     *  This is logically equivalent to
     *      saveLayer(paint)/drawPicture/restore
     */
    void drawPicture(const SkPicture*, const SkMatrix* matrix, const SkPaint* paint);

    enum VertexMode {
        kTriangles_VertexMode,
        kTriangleStrip_VertexMode,
        kTriangleFan_VertexMode
    };

    /** Draw the array of vertices, interpreted as triangles (based on mode).

        If both textures and vertex-colors are NULL, it strokes hairlines with
        the paint's color. This behavior is a useful debugging mode to visualize
        the mesh.

        @param vmode How to interpret the array of vertices
        @param vertexCount The number of points in the vertices array (and
                    corresponding texs and colors arrays if non-null)
        @param vertices Array of vertices for the mesh
        @param texs May be null. If not null, specifies the coordinate
                    in _texture_ space (not uv space) for each vertex.
        @param colors May be null. If not null, specifies a color for each
                      vertex, to be interpolated across the triangle.
        @param xmode Used if both texs and colors are present. In this
                    case the colors are combined with the texture using mode,
                    before being drawn using the paint. If mode is null, then
                    kModulate_Mode is used.
        @param indices If not null, array of indices to reference into the
                    vertex (texs, colors) array.
        @param indexCount number of entries in the indices array (if not null)
        @param paint Specifies the shader/texture if present.
    */
    virtual void drawVertices(VertexMode vmode, int vertexCount,
                              const SkPoint vertices[], const SkPoint texs[],
                              const SkColor colors[], SkXfermode* xmode,
                              const uint16_t indices[], int indexCount,
                              const SkPaint& paint);

    /**
     Draw a cubic coons patch

     @param cubic specifies the 4 bounding cubic bezier curves of a patch with clockwise order
                    starting at the top left corner.
     @param colors specifies the colors for the corners which will be bilerp across the patch,
                    their order is clockwise starting at the top left corner.
     @param texCoords specifies the texture coordinates that will be bilerp across the patch,
                    their order is the same as the colors.
     @param xmode specifies how are the colors and the textures combined if both of them are
                    present.
     @param paint Specifies the shader/texture if present.
     */
    void drawPatch(const SkPoint cubics[12], const SkColor colors[4],
                   const SkPoint texCoords[4], SkXfermode* xmode, const SkPaint& paint);

    /** Send a blob of data to the canvas.
        For canvases that draw, this call is effectively a no-op, as the data
        is not parsed, but just ignored. However, this call exists for
        subclasses like SkPicture's recording canvas, that can store the data
        and then play it back later (via another call to drawData).
     */
    virtual void drawData(const void* data, size_t length) {
        // do nothing. Subclasses may do something with the data
    }

    /** Add comments. beginCommentGroup/endCommentGroup open/close a new group.
        Each comment added via addComment is notionally attached to its
        enclosing group. Top-level comments simply belong to no group.
     */
    virtual void beginCommentGroup(const char* description) {
        // do nothing. Subclasses may do something
    }
    virtual void addComment(const char* kywd, const char* value) {
        // do nothing. Subclasses may do something
    }
    virtual void endCommentGroup() {
        // do nothing. Subclasses may do something
    }

    /**
     *  With this call the client asserts that subsequent draw operations (up to the
     *  matching popCull()) are fully contained within the given bounding box. The assertion
     *  is not enforced, but the information might be used to quick-reject command blocks,
     *  so an incorrect bounding box may result in incomplete rendering.
     */
    void pushCull(const SkRect& cullRect);

    /**
     *  Terminates the current culling block, and restores the previous one (if any).
     */
    void popCull();

    //////////////////////////////////////////////////////////////////////////

    /** Get the current filter object. The filter's reference count is not
        affected. The filter is saved/restored, just like the matrix and clip.
        @return the canvas' filter (or NULL).
    */
    SkDrawFilter* getDrawFilter() const;

    /** Set the new filter (or NULL). Pass NULL to clear any existing filter.
        As a convenience, the parameter is returned. If an existing filter
        exists, its refcnt is decrement. If the new filter is not null, its
        refcnt is incremented. The filter is saved/restored, just like the
        matrix and clip.
        @param filter the new filter (or NULL)
        @return the new filter
    */
    virtual SkDrawFilter* setDrawFilter(SkDrawFilter* filter);

    //////////////////////////////////////////////////////////////////////////

    /**
     *  Return true if the current clip is empty (i.e. nothing will draw).
     *  Note: this is not always a free call, so it should not be used
     *  more often than necessary. However, once the canvas has computed this
     *  result, subsequent calls will be cheap (until the clip state changes,
     *  which can happen on any clip..() or restore() call.
     */
    virtual bool isClipEmpty() const;

    /**
     *  Returns true if the current clip is just a (non-empty) rectangle.
     *  Returns false if the clip is empty, or if it is complex.
     */
    virtual bool isClipRect() const;

    /** Return the current matrix on the canvas.
        This does not account for the translate in any of the devices.
        @return The current matrix on the canvas.
    */
    const SkMatrix& getTotalMatrix() const;

    /** Return the clip stack. The clip stack stores all the individual
     *  clips organized by the save/restore frame in which they were
     *  added.
     *  @return the current clip stack ("list" of individual clip elements)
     */
    const SkClipStack* getClipStack() const {
        return &fClipStack;
    }

    typedef SkCanvasClipVisitor ClipVisitor;
    /**
     *  Replays the clip operations, back to front, that have been applied to
     *  the canvas, calling the appropriate method on the visitor for each
     *  clip. All clips have already been transformed into device space.
     */
    void replayClips(ClipVisitor*) const;

    ///////////////////////////////////////////////////////////////////////////

    /** After calling saveLayer(), there can be any number of devices that make
        up the top-most drawing area. LayerIter can be used to iterate through
        those devices. Note that the iterator is only valid until the next API
        call made on the canvas. Ownership of all pointers in the iterator stays
        with the canvas, so none of them should be modified or deleted.
    */
    class SK_API LayerIter /*: SkNoncopyable*/ {
    public:
        /** Initialize iterator with canvas, and set values for 1st device */
        LayerIter(SkCanvas*, bool skipEmptyClips);
        ~LayerIter();

        /** Return true if the iterator is done */
        bool done() const { return fDone; }
        /** Cycle to the next device */
        void next();

        // These reflect the current device in the iterator

        SkBaseDevice*   device() const;
        const SkMatrix& matrix() const;
        const SkRegion& clip() const;
        const SkPaint&  paint() const;
        int             x() const;
        int             y() const;

    private:
        // used to embed the SkDrawIter object directly in our instance, w/o
        // having to expose that class def to the public. There is an assert
        // in our constructor to ensure that fStorage is large enough
        // (though needs to be a compile-time-assert!). We use intptr_t to work
        // safely with 32 and 64 bit machines (to ensure the storage is enough)
        intptr_t          fStorage[32];
        class SkDrawIter* fImpl;    // this points at fStorage
        SkPaint           fDefaultPaint;
        bool              fDone;
    };

    // don't call
    const SkRegion& internal_private_getTotalClip() const;
    // don't call
    void internal_private_getTotalClipAsPath(SkPath*) const;
    // don't call
    GrRenderTarget* internal_private_accessTopLayerRenderTarget();

protected:
    // default impl defers to getDevice()->newSurface(info)
    virtual SkSurface* onNewSurface(const SkImageInfo&);

    // default impl defers to its device
    virtual const void* onPeekPixels(SkImageInfo*, size_t* rowBytes);
    virtual void* onAccessTopLayerPixels(SkImageInfo*, size_t* rowBytes);

    // Subclass save/restore notifiers.
    // Overriders should call the corresponding INHERITED method up the inheritance chain.
    // willSaveLayer()'s return value may suppress full layer allocation.
    enum SaveLayerStrategy {
        kFullLayer_SaveLayerStrategy,
        kNoLayer_SaveLayerStrategy
    };

    virtual void willSave() {}
    virtual SaveLayerStrategy willSaveLayer(const SkRect*, const SkPaint*, SaveFlags) {
        return kFullLayer_SaveLayerStrategy;
    }
    virtual void willRestore() {}
    virtual void didRestore() {}
    virtual void didConcat(const SkMatrix&) {}
    virtual void didSetMatrix(const SkMatrix&) {}

    virtual void onDrawDRRect(const SkRRect&, const SkRRect&, const SkPaint&);

    virtual void onDrawText(const void* text, size_t byteLength, SkScalar x,
                            SkScalar y, const SkPaint& paint);

    virtual void onDrawPosText(const void* text, size_t byteLength,
                               const SkPoint pos[], const SkPaint& paint);

    virtual void onDrawPosTextH(const void* text, size_t byteLength,
                                const SkScalar xpos[], SkScalar constY,
                                const SkPaint& paint);

    virtual void onDrawTextOnPath(const void* text, size_t byteLength,
                                  const SkPath& path, const SkMatrix* matrix,
                                  const SkPaint& paint);

    virtual void onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                                const SkPaint& paint);

    virtual void onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                           const SkPoint texCoords[4], SkXfermode* xmode, const SkPaint& paint);

    enum ClipEdgeStyle {
        kHard_ClipEdgeStyle,
        kSoft_ClipEdgeStyle
    };

    virtual void onClipRect(const SkRect& rect, SkRegion::Op op, ClipEdgeStyle edgeStyle);
    virtual void onClipRRect(const SkRRect& rrect, SkRegion::Op op, ClipEdgeStyle edgeStyle);
    virtual void onClipPath(const SkPath& path, SkRegion::Op op, ClipEdgeStyle edgeStyle);
    virtual void onClipRegion(const SkRegion& deviceRgn, SkRegion::Op op);

    virtual void onDiscard();

    virtual void onDrawPicture(const SkPicture*, const SkMatrix*, const SkPaint*);

    // Returns the canvas to be used by DrawIter. Default implementation
    // returns this. Subclasses that encapsulate an indirect canvas may
    // need to overload this method. The impl must keep track of this, as it
    // is not released or deleted by the caller.
    virtual SkCanvas* canvasForDrawIter();

    // Clip rectangle bounds. Called internally by saveLayer.
    // returns false if the entire rectangle is entirely clipped out
    // If non-NULL, The imageFilter parameter will be used to expand the clip
    // and offscreen bounds for any margin required by the filter DAG.
    bool clipRectBounds(const SkRect* bounds, SaveFlags flags,
                        SkIRect* intersection,
                        const SkImageFilter* imageFilter = NULL);

    // Called by child classes that override clipPath and clipRRect to only
    // track fast conservative clip bounds, rather than exact clips.
    void updateClipConservativelyUsingBounds(const SkRect&, SkRegion::Op,
                                             bool inverseFilled);

    // notify our surface (if we have one) that we are about to draw, so it
    // can perform copy-on-write or invalidate any cached images
    void predrawNotify();

    virtual void onPushCull(const SkRect& cullRect);
    virtual void onPopCull();

private:
    class MCRec;

    SkClipStack fClipStack;
    SkDeque     fMCStack;
    // points to top of stack
    MCRec*      fMCRec;
    // the first N recs that can fit here mean we won't call malloc
    uint32_t    fMCRecStorage[32];

    int         fSaveLayerCount;    // number of successful saveLayer calls
    int         fCullCount;         // number of active culls

    SkMetaData* fMetaData;

    SkSurface_Base*  fSurfaceBase;
    SkSurface_Base* getSurfaceBase() const { return fSurfaceBase; }
    void setSurfaceBase(SkSurface_Base* sb) {
        fSurfaceBase = sb;
    }
    friend class SkSurface_Base;
    friend class SkSurface_Gpu;

    bool fDeviceCMDirty;            // cleared by updateDeviceCMCache()
    void updateDeviceCMCache();

    friend class SkDrawIter;        // needs setupDrawForLayerDevice()
    friend class AutoDrawLooper;
    friend class SkLua;             // needs top layer size and offset
    friend class SkDebugCanvas;     // needs experimental fAllowSimplifyClip
    friend class SkDeferredDevice;  // needs getTopDevice()
    friend class SkSurface_Raster;  // needs getDevice()

    SkBaseDevice* createLayerDevice(const SkImageInfo&);

    SkBaseDevice* init(SkBaseDevice*);

    /**
     *  DEPRECATED
     *
     *  Specify a device for this canvas to draw into. If it is not null, its
     *  reference count is incremented. If the canvas was already holding a
     *  device, its reference count is decremented. The new device is returned.
     */
    SkBaseDevice* setRootDevice(SkBaseDevice* device);

    /**
     * Gets the size/origin of the top level layer in global canvas coordinates. We don't want this
     * to be public because it exposes decisions about layer sizes that are internal to the canvas.
     */
    SkISize getTopLayerSize() const;
    SkIPoint getTopLayerOrigin() const;

    // internal methods are not virtual, so they can safely be called by other
    // canvas apis, without confusing subclasses (like SkPictureRecording)
    void internalDrawBitmap(const SkBitmap&, const SkMatrix& m, const SkPaint* paint);
    void internalDrawBitmapRect(const SkBitmap& bitmap, const SkRect* src,
                                const SkRect& dst, const SkPaint* paint,
                                DrawBitmapRectFlags flags);
    void internalDrawBitmapNine(const SkBitmap& bitmap, const SkIRect& center,
                                const SkRect& dst, const SkPaint* paint);
    void internalDrawPaint(const SkPaint& paint);
    int internalSaveLayer(const SkRect* bounds, const SkPaint* paint,
                          SaveFlags, bool justForImageFilter, SaveLayerStrategy strategy);
    void internalDrawDevice(SkBaseDevice*, int x, int y, const SkPaint*);

    // shared by save() and saveLayer()
    int internalSave();
    void internalRestore();
    static void DrawRect(const SkDraw& draw, const SkPaint& paint,
                         const SkRect& r, SkScalar textSize);
    static void DrawTextDecorations(const SkDraw& draw, const SkPaint& paint,
                                    const char text[], size_t byteLength,
                                    SkScalar x, SkScalar y);

    /*  These maintain a cache of the clip bounds in local coordinates,
        (converted to 2s-compliment if floats are slow).
     */
    mutable SkRect fCachedLocalClipBounds;
    mutable bool   fCachedLocalClipBoundsDirty;
    bool fAllowSoftClip;
    bool fAllowSimplifyClip;

    const SkRect& getLocalClipBounds() const {
        if (fCachedLocalClipBoundsDirty) {
            if (!this->getClipBounds(&fCachedLocalClipBounds)) {
                fCachedLocalClipBounds.setEmpty();
            }
            fCachedLocalClipBoundsDirty = false;
        }
        return fCachedLocalClipBounds;
    }

    class AutoValidateClip : ::SkNoncopyable {
    public:
        explicit AutoValidateClip(SkCanvas* canvas) : fCanvas(canvas) {
            fCanvas->validateClip();
        }
        ~AutoValidateClip() { fCanvas->validateClip(); }

    private:
        const SkCanvas* fCanvas;
    };

#ifdef SK_DEBUG
    // The cull stack rects are in device-space
    SkTDArray<SkIRect> fCullStack;
    void validateCull(const SkIRect&);
    void validateClip() const;
#else
    void validateClip() const {}
#endif

    typedef SkRefCnt INHERITED;
};

/** Stack helper class to automatically call restoreToCount() on the canvas
    when this object goes out of scope. Use this to guarantee that the canvas
    is restored to a known state.
*/
class SkAutoCanvasRestore : SkNoncopyable {
public:
    SkAutoCanvasRestore(SkCanvas* canvas, bool doSave) : fCanvas(canvas), fSaveCount(0) {
        if (fCanvas) {
            fSaveCount = canvas->getSaveCount();
            if (doSave) {
                canvas->save();
            }
        }
    }
    ~SkAutoCanvasRestore() {
        if (fCanvas) {
            fCanvas->restoreToCount(fSaveCount);
        }
    }

    /**
     *  Perform the restore now, instead of waiting for the destructor. Will
     *  only do this once.
     */
    void restore() {
        if (fCanvas) {
            fCanvas->restoreToCount(fSaveCount);
            fCanvas = NULL;
        }
    }

private:
    SkCanvas*   fCanvas;
    int         fSaveCount;
};
#define SkAutoCanvasRestore(...) SK_REQUIRE_LOCAL_VAR(SkAutoCanvasRestore)

/** Stack helper class to automatically open and close a comment block
 */
class SkAutoCommentBlock : SkNoncopyable {
public:
    SkAutoCommentBlock(SkCanvas* canvas, const char* description) {
        fCanvas = canvas;
        if (NULL != fCanvas) {
            fCanvas->beginCommentGroup(description);
        }
    }

    ~SkAutoCommentBlock() {
        if (NULL != fCanvas) {
            fCanvas->endCommentGroup();
        }
    }

private:
    SkCanvas* fCanvas;
};
#define SkAutoCommentBlock(...) SK_REQUIRE_LOCAL_VAR(SkAutoCommentBlock)

/**
 *  If the caller wants read-only access to the pixels in a canvas, it can just
 *  call canvas->peekPixels(), since that is the fastest way to "peek" at the
 *  pixels on a raster-backed canvas.
 *
 *  If the canvas has pixels, but they are not readily available to the CPU
 *  (e.g. gpu-backed), then peekPixels() will fail, but readPixels() will
 *  succeed (though be slower, since it will return a copy of the pixels).
 *
 *  SkAutoROCanvasPixels encapsulates these two techniques, trying first to call
 *  peekPixels() (for performance), but if that fails, calling readPixels() and
 *  storing the copy locally.
 *
 *  The caller must respect the restrictions associated with peekPixels(), since
 *  that may have been called: The returned information is invalidated if...
 *      - any API is called on the canvas (or its parent surface if present)
 *      - the canvas goes out of scope
 */
class SkAutoROCanvasPixels : SkNoncopyable {
public:
    SkAutoROCanvasPixels(SkCanvas* canvas);

    // returns NULL on failure
    const void* addr() const { return fAddr; }

    // undefined if addr() == NULL
    size_t rowBytes() const { return fRowBytes; }

    // undefined if addr() == NULL
    const SkImageInfo& info() const { return fInfo; }

    // helper that, if returns true, installs the pixels into the bitmap. Note
    // that the bitmap may reference the address returned by peekPixels(), so
    // the caller must respect the restrictions associated with peekPixels().
    bool asROBitmap(SkBitmap*) const;

private:
    SkBitmap    fBitmap;    // used if peekPixels() fails
    const void* fAddr;      // NULL on failure
    SkImageInfo fInfo;
    size_t      fRowBytes;
};

static inline SkCanvas::SaveFlags operator|(const SkCanvas::SaveFlags lhs,
                                            const SkCanvas::SaveFlags rhs) {
    return static_cast<SkCanvas::SaveFlags>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

static inline SkCanvas::SaveFlags& operator|=(SkCanvas::SaveFlags& lhs,
                                              const SkCanvas::SaveFlags rhs) {
    lhs = lhs | rhs;
    return lhs;
}

class SkCanvasClipVisitor {
public:
    virtual ~SkCanvasClipVisitor();
    virtual void clipRect(const SkRect&, SkRegion::Op, bool antialias) = 0;
    virtual void clipRRect(const SkRRect&, SkRegion::Op, bool antialias) = 0;
    virtual void clipPath(const SkPath&, SkRegion::Op, bool antialias) = 0;
};

#endif
