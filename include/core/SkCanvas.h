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
#include "SkPaint.h"
#include "SkRefCnt.h"
#include "SkRegion.h"
#include "SkSurfaceProps.h"
#include "SkXfermode.h"

class GrContext;
class GrRenderTarget;
class SkBaseDevice;
class SkCanvasClipVisitor;
class SkClipStack;
class SkDraw;
class SkDrawable;
class SkDrawFilter;
class SkImage;
class SkImageFilter;
class SkMetaData;
class SkPath;
class SkPicture;
class SkPixmap;
class SkRRect;
struct SkRSXform;
class SkSurface;
class SkSurface_Base;
class SkTextBlob;

/*
 *  If you want the legacy cliptolayer flag (i.e. android), then you must have the new
 *  legacy saveflags.
 */
#ifdef SK_SUPPORT_LEGACY_CLIPTOLAYERFLAG
#ifndef SK_SUPPORT_LEGACY_SAVEFLAGS
    #define SK_SUPPORT_LEGACY_SAVEFLAGS
#endif
#endif

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
    enum PrivateSaveLayerFlags {
        kDontClipToLayer_PrivateSaveLayerFlag   = 1 << 31,
    };
    
public:
    /**
     *  Attempt to allocate raster canvas, matching the ImageInfo, that will draw directly into the
     *  specified pixels. To access the pixels after drawing to them, the caller should call
     *  flush() or call peekPixels(...).
     *
     *  On failure, return NULL. This can fail for several reasons:
     *  1. invalid ImageInfo (e.g. negative dimensions)
     *  2. unsupported ImageInfo for a canvas
     *      - kUnknown_SkColorType, kIndex_8_SkColorType
     *      - kUnknown_SkAlphaType
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
    SkCanvas(int width, int height, const SkSurfaceProps* = NULL);

    /** Construct a canvas with the specified device to draw into.

        @param device   Specifies a device for the canvas to draw into.
    */
    explicit SkCanvas(SkBaseDevice* device);

    /** Construct a canvas with the specified bitmap to draw into.
        @param bitmap   Specifies a bitmap for the canvas to draw into. Its
                        structure are copied to the canvas.
    */
    explicit SkCanvas(const SkBitmap& bitmap);

    /** Construct a canvas with the specified bitmap to draw into.
        @param bitmap   Specifies a bitmap for the canvas to draw into. Its
                        structure are copied to the canvas.
        @param props    New canvas surface properties.
    */
    SkCanvas(const SkBitmap& bitmap, const SkSurfaceProps& props);

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
    virtual SkISize getBaseLayerSize() const;

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
    SkBaseDevice* getDevice_just_for_deprecated_compatibility_testing() const {
        return this->getDevice();
    }

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
     *
     *  If surfaceprops is specified, those are passed to the new surface, otherwise the new surface
     *  inherits the properties of the surface that owns this canvas. If this canvas has no parent
     *  surface, then the new surface is created with default properties.
     */
    SkSurface* newSurface(const SkImageInfo&, const SkSurfaceProps* = NULL);

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

#ifdef SK_SUPPORT_LEGACY_SAVEFLAGS
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
#endif

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
    int saveLayer(const SkRect& bounds, const SkPaint* paint) {
        return this->saveLayer(&bounds, paint);
    }

    /**
     *  Temporary name.
     *  Will allow any requests for LCD text to be respected, so the caller must be careful to
     *  only draw on top of opaque sections of the layer to get good results.
     */
    int saveLayerPreserveLCDTextRequests(const SkRect* bounds, const SkPaint* paint);

#ifdef SK_SUPPORT_LEGACY_SAVEFLAGS
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
#endif

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

#ifdef SK_SUPPORT_LEGACY_SAVEFLAGS
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
#endif

    enum {
        kIsOpaque_SaveLayerFlag         = 1 << 0,
        kPreserveLCDText_SaveLayerFlag  = 1 << 1,

#ifdef SK_SUPPORT_LEGACY_CLIPTOLAYERFLAG
        kDontClipToLayer_Legacy_SaveLayerFlag = kDontClipToLayer_PrivateSaveLayerFlag,
#endif
    };
    typedef uint32_t SaveLayerFlags;

    struct SaveLayerRec {
        SaveLayerRec()
            : fBounds(nullptr), fPaint(nullptr), fBackdrop(nullptr), fSaveLayerFlags(0)
        {}
        SaveLayerRec(const SkRect* bounds, const SkPaint* paint, SaveLayerFlags saveLayerFlags = 0)
            : fBounds(bounds)
            , fPaint(paint)
            , fBackdrop(nullptr)
            , fSaveLayerFlags(saveLayerFlags)
        {}
        SaveLayerRec(const SkRect* bounds, const SkPaint* paint, const SkImageFilter* backdrop,
                     SaveLayerFlags saveLayerFlags)
            : fBounds(bounds)
            , fPaint(paint)
            , fBackdrop(backdrop)
            , fSaveLayerFlags(saveLayerFlags)
        {}

        const SkRect*           fBounds;    // optional
        const SkPaint*          fPaint;     // optional
        const SkImageFilter*    fBackdrop;  // optional
        SaveLayerFlags          fSaveLayerFlags;
    };

    int saveLayer(const SaveLayerRec&);

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
    void drawColor(SkColor color, SkXfermode::Mode mode = SkXfermode::kSrcOver_Mode);

    /**
     *  Helper method for drawing a color in SRC mode, completely replacing all the pixels
     *  in the current clip with this color.
     */
    void clear(SkColor color) {
        this->drawColor(color, SkXfermode::kSrc_Mode);
    }

    /**
     * This makes the contents of the canvas undefined. Subsequent calls that
     * require reading the canvas contents will produce undefined results. Examples
     * include blending and readPixels. The actual implementation is backend-
     * dependent and one legal implementation is to do nothing. This method
     * ignores the current clip.
     *
     * This function should only be called if the caller intends to subsequently
     * draw to the canvas. The canvas may do real work at discard() time in order
     * to optimize performance on subsequent draws. Thus, if you call this and then
     * never draw to the canvas subsequently you may pay a perfomance penalty.
     */
    void discard() { this->onDiscard(); }

    /**
     *  Fill the entire canvas (restricted to the current clip) with the
     *  specified paint.
     *  @param paint    The paint used to fill the canvas
     */
    void drawPaint(const SkPaint& paint);

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
    void drawPoints(PointMode mode, size_t count, const SkPoint pts[], const SkPaint& paint);

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
    void drawRect(const SkRect& rect, const SkPaint& paint);

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
    void drawOval(const SkRect& oval, const SkPaint&);

    /**
     *  Draw the specified RRect using the specified paint The rrect will be filled or stroked
     *  based on the Style in the paint.
     *
     *  @param rrect    The round-rect to draw
     *  @param paint    The paint used to draw the round-rect
     */
    void drawRRect(const SkRRect& rrect, const SkPaint& paint);

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
    void drawPath(const SkPath& path, const SkPaint& paint);

    /** Draw the specified image, with its top/left corner at (x,y), using the
        specified paint, transformed by the current matrix.

        @param image    The image to be drawn
        @param left     The position of the left side of the image being drawn
        @param top      The position of the top side of the image being drawn
        @param paint    The paint used to draw the image, or NULL
     */
    void drawImage(const SkImage* image, SkScalar left, SkScalar top, const SkPaint* paint = NULL);

    /**
     *  Controls the behavior at the edge of the src-rect, when specified in drawImageRect,
     *  trading off speed for exactness.
     *
     *  When filtering is enabled (in the Paint), skia may need to sample in a neighborhood around
     *  the pixels in the image. If there is a src-rect specified, it is intended to restrict the
     *  pixels that will be read. However, for performance reasons, some implementations may slow
     *  down if they cannot read 1-pixel past the src-rect boundary at times.
     *
     *  This enum allows the caller to specify if such a 1-pixel "slop" will be visually acceptable.
     *  If it is, the caller should pass kFast, and it may result in a faster draw. If the src-rect
     *  must be strictly respected, the caller should pass kStrict.
     */
    enum SrcRectConstraint {
        /**
         *  If kStrict is specified, the implementation must respect the src-rect
         *  (if specified) strictly, and will never sample outside of those bounds during sampling
         *  even when filtering. This may be slower than kFast.
         */
        kStrict_SrcRectConstraint,

        /**
         *  If kFast is specified, the implementation may sample outside of the src-rect
         *  (if specified) by half the width of filter. This allows greater flexibility
         *  to the implementation and can make the draw much faster.
         */
        kFast_SrcRectConstraint,
    };

    /** Draw the specified image, scaling and translating so that it fills the specified
     *  dst rect. If the src rect is non-null, only that subset of the image is transformed
     *  and drawn.
     *
     *  @param image      The image to be drawn
     *  @param src        Optional: specify the subset of the image to be drawn
     *  @param dst        The destination rectangle where the scaled/translated
     *                    image will be drawn
     *  @param paint      The paint used to draw the image, or NULL
     *  @param constraint Control the tradeoff between speed and exactness w.r.t. the src-rect.
     */
    void drawImageRect(const SkImage* image, const SkRect& src, const SkRect& dst,
                       const SkPaint* paint,
                       SrcRectConstraint constraint = kStrict_SrcRectConstraint);
    // variant that takes src SkIRect
    void drawImageRect(const SkImage* image, const SkIRect& isrc, const SkRect& dst,
                       const SkPaint* paint, SrcRectConstraint = kStrict_SrcRectConstraint);
    // variant that assumes src == image-bounds
    void drawImageRect(const SkImage* image, const SkRect& dst, const SkPaint* paint,
                       SrcRectConstraint = kStrict_SrcRectConstraint);

    /**
     *  Draw the image stretched differentially to fit into dst.
     *  center is a rect within the image, and logically divides the image
     *  into 9 sections (3x3). For example, if the middle pixel of a [5x5]
     *  image is the "center", then the center-rect should be [2, 2, 3, 3].
     *
     *  If the dst is >= the image size, then...
     *  - The 4 corners are not stretched at all.
     *  - The sides are stretched in only one axis.
     *  - The center is stretched in both axes.
     * Else, for each axis where dst < image,
     *  - The corners shrink proportionally
     *  - The sides (along the shrink axis) and center are not drawn
     */
    void drawImageNine(const SkImage*, const SkIRect& center, const SkRect& dst,
                        const SkPaint* paint = NULL);

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
    void drawBitmap(const SkBitmap& bitmap, SkScalar left, SkScalar top,
                    const SkPaint* paint = NULL);

    /** Draw the specified bitmap, scaling and translating so that it fills the specified
     *  dst rect. If the src rect is non-null, only that subset of the bitmap is transformed
     *  and drawn.
     *
     *  @param bitmap     The bitmap to be drawn
     *  @param src        Optional: specify the subset of the bitmap to be drawn
     *  @param dst        The destination rectangle where the scaled/translated
     *                    bitmap will be drawn
     *  @param paint      The paint used to draw the bitmap, or NULL
     *  @param constraint Control the tradeoff between speed and exactness w.r.t. the src-rect.
     */
    void drawBitmapRect(const SkBitmap& bitmap, const SkRect& src, const SkRect& dst,
                        const SkPaint* paint, SrcRectConstraint = kStrict_SrcRectConstraint);
    // variant where src is SkIRect
    void drawBitmapRect(const SkBitmap& bitmap, const SkIRect& isrc, const SkRect& dst,
                        const SkPaint* paint, SrcRectConstraint = kStrict_SrcRectConstraint);
    void drawBitmapRect(const SkBitmap& bitmap, const SkRect& dst, const SkPaint* paint,
                        SrcRectConstraint = kStrict_SrcRectConstraint);

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
    void drawBitmapNine(const SkBitmap& bitmap, const SkIRect& center, const SkRect& dst,
                        const SkPaint* paint = NULL);

    /** Draw the text, with origin at (x,y), using the specified paint.
        The origin is interpreted based on the Align setting in the paint.
        @param text The text to be drawn
        @param byteLength   The number of bytes to read from the text parameter
        @param x        The x-coordinate of the origin of the text being drawn
        @param y        The y-coordinate of the origin of the text being drawn
        @param paint    The paint used for the text (e.g. color, size, style)
    */
    void drawText(const void* text, size_t byteLength, SkScalar x, SkScalar y,
                  const SkPaint& paint);

    /** Draw the text, with each character/glyph origin specified by the pos[]
        array. The origin is interpreted by the Align setting in the paint.
        @param text The text to be drawn
        @param byteLength   The number of bytes to read from the text parameter
        @param pos      Array of positions, used to position each character
        @param paint    The paint used for the text (e.g. color, size, style)
        */
    void drawPosText(const void* text, size_t byteLength, const SkPoint pos[],
                     const SkPaint& paint);

    /** Draw the text, with each character/glyph origin specified by the x
        coordinate taken from the xpos[] array, and the y from the constY param.
        The origin is interpreted by the Align setting in the paint.
        @param text The text to be drawn
        @param byteLength   The number of bytes to read from the text parameter
        @param xpos     Array of x-positions, used to position each character
        @param constY   The shared Y coordinate for all of the positions
        @param paint    The paint used for the text (e.g. color, size, style)
        */
    void drawPosTextH(const void* text, size_t byteLength, const SkScalar xpos[], SkScalar constY,
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
    void drawTextOnPathHV(const void* text, size_t byteLength, const SkPath& path, SkScalar hOffset,
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
    void drawTextOnPath(const void* text, size_t byteLength, const SkPath& path,
                        const SkMatrix* matrix, const SkPaint& paint);

    /** Draw the text blob, offset by (x,y), using the specified paint.
        @param blob     The text blob to be drawn
        @param x        The x-offset of the text being drawn
        @param y        The y-offset of the text being drawn
        @param paint    The paint used for the text (e.g. color, size, style)
    */
    void drawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y, const SkPaint& paint);

    /** Draw the picture into this canvas. This method effective brackets the
        playback of the picture's draw calls with save/restore, so the state
        of this canvas will be unchanged after this call.
        @param picture The recorded drawing commands to playback into this
                       canvas.
    */
    void drawPicture(const SkPicture* picture) {
        this->drawPicture(picture, NULL, NULL);
    }

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
    void drawVertices(VertexMode vmode, int vertexCount,
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

    /**
     *  Draw a set of sprites from the atlas. Each is specified by a tex rectangle in the
     *  coordinate space of the atlas, and a corresponding xform which transforms the tex rectangle
     *  into a quad.
     *
     *      xform maps [0, 0, tex.width, tex.height] -> quad
     *
     *  The color array is optional. When specified, each color modulates the pixels in its
     *  corresponding quad (via the specified SkXfermode::Mode).
     *
     *  The cullRect is optional. When specified, it must be a conservative bounds of all of the
     *  resulting transformed quads, allowing the canvas to skip drawing if the cullRect does not
     *  intersect the current clip.
     *
     *  The paint is optional. If specified, its antialiasing, alpha, color-filter, image-filter
     *  and xfermode are used to affect each of the quads.
     */
    void drawAtlas(const SkImage* atlas, const SkRSXform xform[], const SkRect tex[],
                   const SkColor colors[], int count, SkXfermode::Mode, const SkRect* cullRect,
                   const SkPaint* paint);

    void drawAtlas(const SkImage* atlas, const SkRSXform xform[], const SkRect tex[], int count,
                   const SkRect* cullRect, const SkPaint* paint) {
        this->drawAtlas(atlas, xform, tex, NULL, count, SkXfermode::kDst_Mode, cullRect, paint);
    }

    /**
     *  Draw the contents of this drawable into the canvas. If the canvas is async
     *  (e.g. it is recording into a picture) then the drawable will be referenced instead,
     *  to have its draw() method called when the picture is finalized.
     *
     *  If the intent is to force the contents of the drawable into this canvas immediately,
     *  then drawable->draw(canvas) may be called.
     */
    void drawDrawable(SkDrawable* drawable, const SkMatrix* = NULL);
    void drawDrawable(SkDrawable*, SkScalar x, SkScalar y);

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
        return fClipStack;
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
    GrRenderTarget* internal_private_accessTopLayerRenderTarget();

    // don't call
    static void Internal_Private_SetIgnoreSaveLayerBounds(bool);
    static bool Internal_Private_GetIgnoreSaveLayerBounds();
    static void Internal_Private_SetTreatSpriteAsBitmap(bool);
    static bool Internal_Private_GetTreatSpriteAsBitmap();

    // TEMP helpers until we switch virtual over to const& for src-rect
    void legacy_drawImageRect(const SkImage* image, const SkRect* src, const SkRect& dst,
                              const SkPaint* paint,
                              SrcRectConstraint constraint = kStrict_SrcRectConstraint);
    void legacy_drawBitmapRect(const SkBitmap& bitmap, const SkRect* src, const SkRect& dst,
                               const SkPaint* paint,
                               SrcRectConstraint constraint = kStrict_SrcRectConstraint);

protected:
    // default impl defers to getDevice()->newSurface(info)
    virtual SkSurface* onNewSurface(const SkImageInfo&, const SkSurfaceProps&);

    // default impl defers to its device
    virtual bool onPeekPixels(SkPixmap*);
    virtual bool onAccessTopLayerPixels(SkPixmap*);

    // Subclass save/restore notifiers.
    // Overriders should call the corresponding INHERITED method up the inheritance chain.
    // getSaveLayerStrategy()'s return value may suppress full layer allocation.
    enum SaveLayerStrategy {
        kFullLayer_SaveLayerStrategy,
        kNoLayer_SaveLayerStrategy,
    };

    virtual void willSave() {}
    // Overriders should call the corresponding INHERITED method up the inheritance chain.
    virtual SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec&) {
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

    virtual void onDrawDrawable(SkDrawable*, const SkMatrix*);

    virtual void onDrawPaint(const SkPaint&);
    virtual void onDrawRect(const SkRect&, const SkPaint&);
    virtual void onDrawOval(const SkRect&, const SkPaint&);
    virtual void onDrawRRect(const SkRRect&, const SkPaint&);
    virtual void onDrawPoints(PointMode, size_t count, const SkPoint pts[], const SkPaint&);
    virtual void onDrawVertices(VertexMode, int vertexCount, const SkPoint vertices[],
                                const SkPoint texs[], const SkColor colors[], SkXfermode*,
                                const uint16_t indices[], int indexCount, const SkPaint&);

    virtual void onDrawAtlas(const SkImage*, const SkRSXform[], const SkRect[], const SkColor[],
                             int count, SkXfermode::Mode, const SkRect* cull, const SkPaint*);
    virtual void onDrawPath(const SkPath&, const SkPaint&);
    virtual void onDrawImage(const SkImage*, SkScalar dx, SkScalar dy, const SkPaint*);
    virtual void onDrawImageRect(const SkImage*, const SkRect*, const SkRect&, const SkPaint*,
                                 SrcRectConstraint);
    virtual void onDrawImageNine(const SkImage*, const SkIRect& center, const SkRect& dst,
                                 const SkPaint*);

    virtual void onDrawBitmap(const SkBitmap&, SkScalar dx, SkScalar dy, const SkPaint*);
    virtual void onDrawBitmapRect(const SkBitmap&, const SkRect*, const SkRect&, const SkPaint*,
                                  SrcRectConstraint);
    virtual void onDrawBitmapNine(const SkBitmap&, const SkIRect& center, const SkRect& dst,
                                  const SkPaint*);

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
    bool clipRectBounds(const SkRect* bounds, SaveLayerFlags, SkIRect* intersection,
                        const SkImageFilter* imageFilter = NULL);

#ifdef SK_SUPPORT_LEGACY_SAVEFLAGS
    // Needed by SkiaCanvasProxy in Android. Make sure that class is updated
    // before removing this method.
    static uint32_t SaveLayerFlagsToSaveFlags(SaveLayerFlags);
#endif
private:
    static bool BoundsAffectsClip(SaveLayerFlags);
#ifdef SK_SUPPORT_LEGACY_SAVEFLAGS
    static uint32_t SaveFlagsToSaveLayerFlags(SaveFlags);
#endif
    static SaveLayerFlags LegacySaveFlagsToSaveLayerFlags(uint32_t legacySaveFlags);

    enum ShaderOverrideOpacity {
        kNone_ShaderOverrideOpacity,        //!< there is no overriding shader (bitmap or image)
        kOpaque_ShaderOverrideOpacity,      //!< the overriding shader is opaque
        kNotOpaque_ShaderOverrideOpacity,   //!< the overriding shader may not be opaque
    };

    // notify our surface (if we have one) that we are about to draw, so it
    // can perform copy-on-write or invalidate any cached images
    void predrawNotify(bool willOverwritesEntireSurface = false);
    void predrawNotify(const SkRect* rect, const SkPaint* paint, ShaderOverrideOpacity);
    void predrawNotify(const SkRect* rect, const SkPaint* paint, bool shaderOverrideIsOpaque) {
        this->predrawNotify(rect, paint, shaderOverrideIsOpaque ? kOpaque_ShaderOverrideOpacity
                                                                : kNotOpaque_ShaderOverrideOpacity);
    }

    class MCRec;

    SkAutoTUnref<SkClipStack> fClipStack;
    SkDeque     fMCStack;
    // points to top of stack
    MCRec*      fMCRec;
    // the first N recs that can fit here mean we won't call malloc
    enum {
        kMCRecSize      = 128,  // most recent measurement
        kMCRecCount     = 32,   // common depth for save/restores
        kDeviceCMSize   = 136,  // most recent measurement
    };
    intptr_t fMCRecStorage[kMCRecSize * kMCRecCount / sizeof(intptr_t)];
    intptr_t fDeviceCMStorage[kDeviceCMSize / sizeof(intptr_t)];

    const SkSurfaceProps fProps;

    int         fSaveCount;         // value returned by getSaveCount()

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

    void doSave();
    void checkForDeferredSave();

    friend class SkDrawIter;        // needs setupDrawForLayerDevice()
    friend class AutoDrawLooper;
    friend class SkLua;             // needs top layer size and offset
    friend class SkDebugCanvas;     // needs experimental fAllowSimplifyClip
    friend class SkSurface_Raster;  // needs getDevice()
    friend class SkRecorder;        // InitFlags
    friend class SkNoSaveLayerCanvas;   // InitFlags
    friend class SkPictureImageFilter;  // SkCanvas(SkBaseDevice*, SkSurfaceProps*, InitFlags)
    friend class SkPictureRecord;   // predrawNotify (why does it need it? <reed>)
    friend class SkPicturePlayback; // SaveFlagsToSaveLayerFlags

    enum InitFlags {
        kDefault_InitFlags                  = 0,
        kConservativeRasterClip_InitFlag    = 1 << 0,
    };
    SkCanvas(const SkIRect& bounds, InitFlags);
    SkCanvas(SkBaseDevice* device, InitFlags);

    void resetForNextPicture(const SkIRect& bounds);

    // needs gettotalclip()
    friend class SkCanvasStateUtils;

    // call this each time we attach ourselves to a device
    //  - constructor
    //  - internalSaveLayer
    void setupDevice(SkBaseDevice*);

    SkBaseDevice* init(SkBaseDevice*, InitFlags);

    /**
     * Gets the size/origin of the top level layer in global canvas coordinates. We don't want this
     * to be public because it exposes decisions about layer sizes that are internal to the canvas.
     */
    SkISize getTopLayerSize() const;
    SkIPoint getTopLayerOrigin() const;

    void internalDrawBitmapRect(const SkBitmap& bitmap, const SkRect* src,
                                const SkRect& dst, const SkPaint* paint,
                                SrcRectConstraint);
    void internalDrawPaint(const SkPaint& paint);
    void internalSaveLayer(const SaveLayerRec&, SaveLayerStrategy);
    void internalDrawDevice(SkBaseDevice*, int x, int y, const SkPaint*, bool isBitmapDevice);

    // shared by save() and saveLayer()
    void internalSave();
    void internalRestore();
    static void DrawRect(const SkDraw& draw, const SkPaint& paint,
                         const SkRect& r, SkScalar textSize);
    static void DrawTextDecorations(const SkDraw& draw, const SkPaint& paint,
                                    const char text[], size_t byteLength,
                                    SkScalar x, SkScalar y);

    // only for canvasutils
    const SkRegion& internal_private_getTotalClip() const;

    /*
     *  Returns true if drawing the specified rect (or all if it is null) with the specified
     *  paint (or default if null) would overwrite the entire root device of the canvas
     *  (i.e. the canvas' surface if it had one).
     */
    bool wouldOverwriteEntireSurface(const SkRect*, const SkPaint*, ShaderOverrideOpacity) const;

    /**
     *  Returns true if the paint's imagefilter can be invoked directly, without needed a layer.
     */
    bool canDrawBitmapAsSprite(SkScalar x, SkScalar y, int w, int h, const SkPaint&);

    /*  These maintain a cache of the clip bounds in local coordinates,
        (converted to 2s-compliment if floats are slow).
     */
    mutable SkRect fCachedLocalClipBounds;
    mutable bool   fCachedLocalClipBoundsDirty;
    bool fAllowSoftClip;
    bool fAllowSimplifyClip;
    const bool fConservativeRasterClip;

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

#ifdef SK_SUPPORT_LEGACY_SAVEFLAGS
static inline SkCanvas::SaveFlags operator|(const SkCanvas::SaveFlags lhs,
                                            const SkCanvas::SaveFlags rhs) {
    return static_cast<SkCanvas::SaveFlags>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

static inline SkCanvas::SaveFlags& operator|=(SkCanvas::SaveFlags& lhs,
                                              const SkCanvas::SaveFlags rhs) {
    lhs = lhs | rhs;
    return lhs;
}
#endif

class SkCanvasClipVisitor {
public:
    virtual ~SkCanvasClipVisitor();
    virtual void clipRect(const SkRect&, SkRegion::Op, bool antialias) = 0;
    virtual void clipRRect(const SkRRect&, SkRegion::Op, bool antialias) = 0;
    virtual void clipPath(const SkPath&, SkRegion::Op, bool antialias) = 0;
};

#endif
