
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
#include "SkScalarCompare.h"
#include "SkXfermode.h"

class SkBounder;
class SkDevice;
class SkDraw;
class SkDrawFilter;
class SkMetaData;
class SkPicture;
class SkRRect;
class SkSurface_Base;

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

    SkCanvas();

    /** Construct a canvas with the specified device to draw into.

        @param device   Specifies a device for the canvas to draw into.
    */
    explicit SkCanvas(SkDevice* device);

    /** Deprecated - Construct a canvas with the specified bitmap to draw into.
        @param bitmap   Specifies a bitmap for the canvas to draw into. Its
                        structure are copied to the canvas.
    */
    explicit SkCanvas(const SkBitmap& bitmap);
    virtual ~SkCanvas();

    SkMetaData& getMetaData();

    ///////////////////////////////////////////////////////////////////////////

    /**
     *  Trigger the immediate execution of all pending draw operations.
     */
    void flush();

    /**
     *  Return the width/height of the underlying device. The current drawable
     *  area may be small (due to clipping or saveLayer). For a canvas with
     *  no device, 0,0 will be returned.
     */
    SkISize getDeviceSize() const;

    /** Return the canvas' device object, which may be null. The device holds
        the bitmap of the pixels that the canvas draws into. The reference count
        of the returned device is not changed by this call.
    */
    SkDevice* getDevice() const;

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
    SkDevice* getTopDevice(bool updateMatrixClip = false) const;

    /**
     *  Shortcut for getDevice()->createCompatibleDevice(...).
     *  If getDevice() == NULL, this method does nothing, and returns NULL.
     */
    SkDevice* createCompatibleDevice(SkBitmap::Config config,
                                    int width, int height,
                                    bool isOpaque);

    ///////////////////////////////////////////////////////////////////////////

    /**
     * This enum can be used with read/writePixels to perform a pixel ops to or
     * from an 8888 config other than Skia's native config (SkPMColor). There
     * are three byte orders supported: native, BGRA, and RGBA. Each has a
     * premultiplied and unpremultiplied variant.
     *
     * Components of a 8888 pixel can be packed/unpacked from a 32bit word using
     * either byte offsets or shift values. Byte offsets are endian-invariant
     * while shifts are not. BGRA and RGBA configs are defined by byte
     * orderings. The native config is defined by shift values (SK_A32_SHIFT,
     * ..., SK_B32_SHIFT).
     */
    enum Config8888 {
        /**
         * Skia's native order specified by:
         *      SK_A32_SHIFT, SK_R32_SHIFT, SK_G32_SHIFT, and SK_B32_SHIFT
         *
         * kNative_Premul_Config8888 is equivalent to SkPMColor
         * kNative_Unpremul_Config8888 has the same component order as SkPMColor
         * but is not premultiplied.
         */
        kNative_Premul_Config8888,
        kNative_Unpremul_Config8888,
        /**
         * low byte to high byte: B, G, R, A.
         */
        kBGRA_Premul_Config8888,
        kBGRA_Unpremul_Config8888,
        /**
         * low byte to high byte: R, G, B, A.
         */
        kRGBA_Premul_Config8888,
        kRGBA_Unpremul_Config8888
    };

    /**
     *  On success (returns true), copy the canvas pixels into the bitmap.
     *  On failure, the bitmap parameter is left unchanged and false is
     *  returned.
     *
     *  The canvas' pixels are converted to the bitmap's config. The only
     *  supported config is kARGB_8888_Config, though this is likely to be
     *  relaxed in  the future. The meaning of config kARGB_8888_Config is
     *  modified by the enum param config8888. The default value interprets
     *  kARGB_8888_Config as SkPMColor
     *
     *  If the bitmap has pixels already allocated, the canvas pixels will be
     *  written there. If not, bitmap->allocPixels() will be called
     *  automatically. If the bitmap is backed by a texture readPixels will
     *  fail.
     *
     *  The actual pixels written is the intersection of the canvas' bounds, and
     *  the rectangle formed by the bitmap's width,height and the specified x,y.
     *  If bitmap pixels extend outside of that intersection, they will not be
     *  modified.
     *
     *  Other failure conditions:
     *    * If the canvas is backed by a non-raster device (e.g. PDF) then
     *       readPixels will fail.
     *    * If bitmap is texture-backed then readPixels will fail. (This may be
     *       relaxed in the future.)
     *
     *  Example that reads the entire canvas into a bitmap using the native
     *  SkPMColor:
     *    SkISize size = canvas->getDeviceSize();
     *    bitmap->setConfig(SkBitmap::kARGB_8888_Config, size.fWidth,
     *                                                   size.fHeight);
     *    if (canvas->readPixels(bitmap, 0, 0)) {
     *       // use the pixels
     *    }
     */
    bool readPixels(SkBitmap* bitmap,
                    int x, int y,
                    Config8888 config8888 = kNative_Premul_Config8888);

    /**
     * DEPRECATED: This will be removed as soon as webkit is no longer relying
     * on it. The bitmap is resized to the intersection of srcRect and the
     * canvas bounds. New pixels are always allocated on success. Bitmap is
     * unmodified on failure.
     */
    bool readPixels(const SkIRect& srcRect, SkBitmap* bitmap);

    /**
     *  Similar to draw sprite, this method will copy the pixels in bitmap onto
     *  the canvas, with the top/left corner specified by (x, y). The canvas'
     *  pixel values are completely replaced: there is no blending.
     *
     *  Currently if bitmap is backed by a texture this is a no-op. This may be
     *  relaxed in the future.
     *
     *  If the bitmap has config kARGB_8888_Config then the config8888 param
     *  will determines how the pixel valuess are intepreted. If the bitmap is
     *  not kARGB_8888_Config then this parameter is ignored.
     *
     *  Note: If you are recording drawing commands on this canvas to
     *  SkPicture, writePixels() is ignored!
     */
    void writePixels(const SkBitmap& bitmap,
                     int x, int y,
                     Config8888 config8888 = kNative_Premul_Config8888);

    ///////////////////////////////////////////////////////////////////////////

    enum SaveFlags {
        /** save the matrix state, restoring it on restore() */
        kMatrix_SaveFlag            = 0x01,
        /** save the clip state, restoring it on restore() */
        kClip_SaveFlag              = 0x02,
        /** the layer needs to support per-pixel alpha */
        kHasAlphaLayer_SaveFlag     = 0x04,
        /** the layer needs to support 8-bits per color component */
        kFullColorLayer_SaveFlag    = 0x08,
        /** the layer should clip against the bounds argument */
        kClipToLayer_SaveFlag       = 0x10,

        // helper masks for common choices
        kMatrixClip_SaveFlag        = 0x03,
        kARGB_NoClipLayer_SaveFlag  = 0x0F,
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
    virtual int save(SaveFlags flags = kMatrixClip_SaveFlag);

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
        @param flags  LayerFlags
        @return The value to pass to restoreToCount() to balance this save()
    */
    virtual int saveLayer(const SkRect* bounds, const SkPaint* paint,
                          SaveFlags flags = kARGB_ClipLayer_SaveFlag);

    /** This behaves the same as save(), but in addition it allocates an
        offscreen bitmap. All drawing calls are directed there, and only when
        the balancing call to restore() is made is that offscreen transfered to
        the canvas (or the previous layer).
        @param bounds (may be null) This rect, if non-null, is used as a hint to
                      limit the size of the offscreen, and thus drawing may be
                      clipped to it, though that clipping is not guaranteed to
                      happen. If exact clipping is desired, use clipRect().
        @param alpha  This is applied to the offscreen when restore() is called.
        @param flags  LayerFlags
        @return The value to pass to restoreToCount() to balance this save()
    */
    int saveLayerAlpha(const SkRect* bounds, U8CPU alpha,
                       SaveFlags flags = kARGB_ClipLayer_SaveFlag);

    /** This call balances a previous call to save(), and is used to remove all
        modifications to the matrix/clip/drawFilter state since the last save
        call.
        It is an error to call restore() more times than save() was called.
    */
    virtual void restore();

    /** Returns the number of matrix/clip states on the SkCanvas' private stack.
        This will equal # save() calls - # restore() calls.
    */
    int getSaveCount() const;

    /** Efficient way to pop any calls to save() that happened after the save
        count reached saveCount. It is an error for saveCount to be less than
        getSaveCount()
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
        returns true if the operation succeeded (e.g. did not overflow)
    */
    virtual bool translate(SkScalar dx, SkScalar dy);

    /** Preconcat the current matrix with the specified scale.
        @param sx   The amount to scale in X
        @param sy   The amount to scale in Y
        returns true if the operation succeeded (e.g. did not overflow)
    */
    virtual bool scale(SkScalar sx, SkScalar sy);

    /** Preconcat the current matrix with the specified rotation.
        @param degrees  The amount to rotate, in degrees
        returns true if the operation succeeded (e.g. did not overflow)
    */
    virtual bool rotate(SkScalar degrees);

    /** Preconcat the current matrix with the specified skew.
        @param sx   The amount to skew in X
        @param sy   The amount to skew in Y
        returns true if the operation succeeded (e.g. did not overflow)
    */
    virtual bool skew(SkScalar sx, SkScalar sy);

    /** Preconcat the current matrix with the specified matrix.
        @param matrix   The matrix to preconcatenate with the current matrix
        @return true if the operation succeeded (e.g. did not overflow)
    */
    virtual bool concat(const SkMatrix& matrix);

    /** Replace the current matrix with a copy of the specified matrix.
        @param matrix The matrix that will be copied into the current matrix.
    */
    virtual void setMatrix(const SkMatrix& matrix);

    /** Helper for setMatrix(identity). Sets the current matrix to identity.
    */
    void resetMatrix();

    /**
     *  Modify the current clip with the specified rectangle.
     *  @param rect The rect to combine with the current clip
     *  @param op The region op to apply to the current clip
     *  @param doAntiAlias true if the clip should be antialiased
     *  @return true if the canvas' clip is non-empty
     */
    virtual bool clipRect(const SkRect& rect,
                          SkRegion::Op op = SkRegion::kIntersect_Op,
                          bool doAntiAlias = false);

    /**
     *  Modify the current clip with the specified SkRRect.
     *  @param rrect The rrect to combine with the current clip
     *  @param op The region op to apply to the current clip
     *  @param doAntiAlias true if the clip should be antialiased
     *  @return true if the canvas' clip is non-empty
     */
    virtual bool clipRRect(const SkRRect& rrect,
                           SkRegion::Op op = SkRegion::kIntersect_Op,
                           bool doAntiAlias = false);

    /**
     *  Modify the current clip with the specified path.
     *  @param path The path to combine with the current clip
     *  @param op The region op to apply to the current clip
     *  @param doAntiAlias true if the clip should be antialiased
     *  @return true if the canvas' new clip is non-empty
     */
    virtual bool clipPath(const SkPath& path,
                          SkRegion::Op op = SkRegion::kIntersect_Op,
                          bool doAntiAlias = false);

    /** EXPERIMENTAL -- only used for testing
        Set to false to force clips to be hard, even if doAntiAlias=true is
        passed to clipRect or clipPath.
     */
    void setAllowSoftClip(bool allow) {
        fAllowSoftClip = allow;
    }

    /** Modify the current clip with the specified region. Note that unlike
        clipRect() and clipPath() which transform their arguments by the current
        matrix, clipRegion() assumes its argument is already in device
        coordinates, and so no transformation is performed.
        @param deviceRgn    The region to apply to the current clip
        @param op The region op to apply to the current clip
        @return true if the canvas' new clip is non-empty
    */
    virtual bool clipRegion(const SkRegion& deviceRgn,
                            SkRegion::Op op = SkRegion::kIntersect_Op);

    /** Helper for clipRegion(rgn, kReplace_Op). Sets the current clip to the
        specified region. This does not intersect or in any other way account
        for the existing clip region.
        @param deviceRgn The region to copy into the current clip.
        @return true if the new clip region is non-empty
    */
    bool setClipRegion(const SkRegion& deviceRgn) {
        return this->clipRegion(deviceRgn, SkRegion::kReplace_Op);
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
        SkASSERT(SkScalarToCompareType(top) <= SkScalarToCompareType(bottom));
        const SkRectCompareType& clipR = this->getLocalClipBoundsCompareType();
        // In the case where the clip is empty and we are provided with a
        // negative top and positive bottom parameter then this test will return
        // false even though it will be clipped. We have chosen to exclude that
        // check as it is rare and would result double the comparisons.
        return SkScalarToCompareType(top) >= clipR.fBottom
            || SkScalarToCompareType(bottom) <= clipR.fTop;
    }

    /** Return the bounds of the current clip (in local coordinates) in the
        bounds parameter, and return true if it is non-empty. This can be useful
        in a way similar to quickReject, in that it tells you that drawing
        outside of these bounds will be clipped out.
    */
    bool getClipBounds(SkRect* bounds) const;

    /** Return the bounds of the current clip, in device coordinates; returns
        true if non-empty. Maybe faster than getting the clip explicitly and
        then taking its bounds.
    */
    bool getClipDeviceBounds(SkIRect* bounds) const;


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
    void drawIRect(const SkIRect& rect, const SkPaint& paint)
    {
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
        @param bitmap   The bitmap to be drawn
        @param left     The position of the left side of the bitmap being drawn
        @param top      The position of the top side of the bitmap being drawn
        @param paint    The paint used to draw the bitmap, or NULL
    */
    virtual void drawBitmap(const SkBitmap& bitmap, SkScalar left, SkScalar top,
                            const SkPaint* paint = NULL);

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
                                      const SkPaint* paint);

    void drawBitmapRect(const SkBitmap& bitmap, const SkRect& dst,
                        const SkPaint* paint) {
        this->drawBitmapRectToRect(bitmap, NULL, dst, paint);
    }

    void drawBitmapRect(const SkBitmap& bitmap, const SkIRect* isrc,
                        const SkRect& dst, const SkPaint* paint = NULL) {
        SkRect realSrcStorage;
        SkRect* realSrcPtr = NULL;
        if (isrc) {
            realSrcStorage.set(*isrc);
            realSrcPtr = &realSrcStorage;
        }
        this->drawBitmapRectToRect(bitmap, realSrcPtr, dst, paint);
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
     *  - The 4 corners are not stretch at all.
     *  - The sides are stretch in only one axis.
     *  - The center is stretch in both axes.
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
    virtual void drawText(const void* text, size_t byteLength, SkScalar x,
                          SkScalar y, const SkPaint& paint);

    /** Draw the text, with each character/glyph origin specified by the pos[]
        array. The origin is interpreted by the Align setting in the paint.
        @param text The text to be drawn
        @param byteLength   The number of bytes to read from the text parameter
        @param pos      Array of positions, used to position each character
        @param paint    The paint used for the text (e.g. color, size, style)
        */
    virtual void drawPosText(const void* text, size_t byteLength,
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
    virtual void drawPosTextH(const void* text, size_t byteLength,
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
    virtual void drawTextOnPath(const void* text, size_t byteLength,
                                const SkPath& path, const SkMatrix* matrix,
                                const SkPaint& paint);

#ifdef SK_BUILD_FOR_ANDROID
    /** Draw the text on path, with each character/glyph origin specified by the pos[]
        array. The origin is interpreted by the Align setting in the paint.
        @param text The text to be drawn
        @param byteLength   The number of bytes to read from the text parameter
        @param pos      Array of positions, used to position each character
        @param paint    The paint used for the text (e.g. color, size, style)
        @param path The path to draw on
        @param matrix The canvas matrix
        */
    void drawPosTextOnPath(const void* text, size_t byteLength,
                           const SkPoint pos[], const SkPaint& paint,
                           const SkPath& path, const SkMatrix* matrix);
#endif

    /** Draw the picture into this canvas. This method effective brackets the
        playback of the picture's draw calls with save/restore, so the state
        of this canvas will be unchanged after this call.
        @param picture The recorded drawing commands to playback into this
                       canvas.
    */
    virtual void drawPicture(SkPicture& picture);

    enum VertexMode {
        kTriangles_VertexMode,
        kTriangleStrip_VertexMode,
        kTriangleFan_VertexMode
    };

    /** Draw the array of vertices, interpreted as triangles (based on mode).
        @param vmode How to interpret the array of vertices
        @param vertexCount The number of points in the vertices array (and
                    corresponding texs and colors arrays if non-null)
        @param vertices Array of vertices for the mesh
        @param texs May be null. If not null, specifies the coordinate
                             in texture space for each vertex.
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

    /** Send a blob of data to the canvas.
        For canvases that draw, this call is effectively a no-op, as the data
        is not parsed, but just ignored. However, this call exists for
        subclasses like SkPicture's recording canvas, that can store the data
        and then play it back later (via another call to drawData).
     */
    virtual void drawData(const void* data, size_t length);

    //////////////////////////////////////////////////////////////////////////

    /** Get the current bounder object.
        The bounder's reference count is unchaged.
        @return the canva's bounder (or NULL).
    */
    SkBounder*  getBounder() const { return fBounder; }

    /** Set a new bounder (or NULL).
        Pass NULL to clear any previous bounder.
        As a convenience, the parameter passed is also returned.
        If a previous bounder exists, its reference count is decremented.
        If bounder is not NULL, its reference count is incremented.
        @param bounder the new bounder (or NULL) to be installed in the canvas
        @return the set bounder object
    */
    virtual SkBounder* setBounder(SkBounder* bounder);

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

    /** Return the current matrix on the canvas.
        This does not account for the translate in any of the devices.
        @return The current matrix on the canvas.
    */
    const SkMatrix& getTotalMatrix() const;

    enum ClipType {
        kEmpty_ClipType = 0,
        kRect_ClipType,
        kComplex_ClipType
    };

    /** Returns a description of the total clip; may be cheaper than
        getting the clip and querying it directly.
    */
    ClipType getClipType() const;

    /** Return the current device clip (concatenation of all clip calls).
     *  This does not account for the translate in any of the devices.
     *  @return the current device clip (concatenation of all clip calls).
     *
     *  DEPRECATED -- call getClipDeviceBounds() instead.
     */
    const SkRegion& getTotalClip() const;

    /** Return the clip stack. The clip stack stores all the individual
     *  clips organized by the save/restore frame in which they were
     *  added.
     *  @return the current clip stack ("list" of individual clip elements)
     */
    const SkClipStack* getClipStack() const {
        return &fClipStack;
    }

    class ClipVisitor {
    public:
        virtual ~ClipVisitor();
        virtual void clipRect(const SkRect&, SkRegion::Op, bool antialias) = 0;
        virtual void clipPath(const SkPath&, SkRegion::Op, bool antialias) = 0;
    };

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

        SkDevice*       device() const;
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

protected:
    // Returns the canvas to be used by DrawIter. Default implementation
    // returns this. Subclasses that encapsulate an indirect canvas may
    // need to overload this method. The impl must keep track of this, as it
    // is not released or deleted by the caller.
    virtual SkCanvas* canvasForDrawIter();

    // all of the drawBitmap variants call this guy
    void commonDrawBitmap(const SkBitmap&, const SkIRect*, const SkMatrix&,
                          const SkPaint& paint);

    // Clip rectangle bounds. Called internally by saveLayer.
    // returns false if the entire rectangle is entirely clipped out
    bool clipRectBounds(const SkRect* bounds, SaveFlags flags,
                        SkIRect* intersection);

    // notify our surface (if we have one) that we are about to draw, so it
    // can perform copy-on-write or invalidate any cached images
    void predrawNotify();

    /** DEPRECATED -- use constructor(device)

     Marked as 'protected' to avoid new clients using this before we can
     completely remove it.

     Specify a device for this canvas to draw into. If it is not null, its
     reference count is incremented. If the canvas was already holding a
     device, its reference count is decremented. The new device is returned.
     */
    virtual SkDevice* setDevice(SkDevice* device);

private:
    class MCRec;

    SkClipStack fClipStack;
    SkDeque     fMCStack;
    // points to top of stack
    MCRec*      fMCRec;
    // the first N recs that can fit here mean we won't call malloc
    uint32_t    fMCRecStorage[32];

    SkBounder*  fBounder;
    int         fSaveLayerCount;    // number of successful saveLayer calls

    SkMetaData* fMetaData;

    SkSurface_Base*  fSurfaceBase;
    SkSurface_Base* getSurfaceBase() const { return fSurfaceBase; }
    void setSurfaceBase(SkSurface_Base* sb) {
        fSurfaceBase = sb;
    }
    friend class SkSurface_Base;

    bool fDeviceCMDirty;            // cleared by updateDeviceCMCache()
    void updateDeviceCMCache();

    friend class SkDrawIter;    // needs setupDrawForLayerDevice()
    friend class AutoDrawLooper;

    SkDevice* createLayerDevice(SkBitmap::Config, int width, int height,
                                bool isOpaque);

    SkDevice* init(SkDevice*);

    // internal methods are not virtual, so they can safely be called by other
    // canvas apis, without confusing subclasses (like SkPictureRecording)
    void internalDrawBitmap(const SkBitmap&, const SkIRect*, const SkMatrix& m,
                                  const SkPaint* paint);
    void internalDrawBitmapRect(const SkBitmap& bitmap, const SkRect* src,
                                const SkRect& dst, const SkPaint* paint);
    void internalDrawBitmapNine(const SkBitmap& bitmap, const SkIRect& center,
                                const SkRect& dst, const SkPaint* paint);
    void internalDrawPaint(const SkPaint& paint);
    int internalSaveLayer(const SkRect* bounds, const SkPaint* paint,
                          SaveFlags, bool justForImageFilter);
    void internalDrawDevice(SkDevice*, int x, int y, const SkPaint*);

    // shared by save() and saveLayer()
    int internalSave(SaveFlags flags);
    void internalRestore();
    static void DrawRect(const SkDraw& draw, const SkPaint& paint,
                         const SkRect& r, SkScalar textSize);
    static void DrawTextDecorations(const SkDraw& draw, const SkPaint& paint,
                                    const char text[], size_t byteLength,
                                    SkScalar x, SkScalar y);

    /*  These maintain a cache of the clip bounds in local coordinates,
        (converted to 2s-compliment if floats are slow).
     */
    mutable SkRectCompareType fLocalBoundsCompareType;
    mutable bool              fLocalBoundsCompareTypeDirty;
    bool fAllowSoftClip;

    const SkRectCompareType& getLocalClipBoundsCompareType() const {
        if (fLocalBoundsCompareTypeDirty) {
            this->computeLocalClipBoundsCompareType();
            fLocalBoundsCompareTypeDirty = false;
        }
        return fLocalBoundsCompareType;
    }
    void computeLocalClipBoundsCompareType() const;

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
    SkAutoCanvasRestore(SkCanvas* canvas, bool doSave) : fCanvas(canvas) {
        SkASSERT(canvas);
        fSaveCount = canvas->getSaveCount();
        if (doSave) {
            canvas->save();
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

#endif
