/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDeferredCanvas_DEFINED
#define SkDeferredCanvas_DEFINED

#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkPicture.h"
#include "SkPixelRef.h"

/** \class SkDeferredCanvas
    Subclass of SkCanvas that encapsulates an SkPicture for deferred drawing.
    The main difference between this class and SkPictureRecord (the canvas
    provided by SkPicture) is that this is a full drop-in replacement for
    SkCanvas, while SkPictureRecord only supports draw operations.
    SkDeferredCanvas will transparently trigger the flushing of deferred
    draw operations when an attempt is made to access the pixel data.
*/
class SK_API SkDeferredCanvas : public SkCanvas {
public:
    class DeviceContext;

    SkDeferredCanvas();

    /** Construct a canvas with the specified device to draw into.
        Equivalent to calling default constructor, then setDevice.
        @param device Specifies a device for the canvas to draw into.
    */
    explicit SkDeferredCanvas(SkDevice* device);

    /** Construct a canvas with the specified device to draw into, and
     *  a device context. Equivalent to calling default constructor, then
     *  setDevice.
     *  @param device Specifies a device for the canvas to draw into.
     *  @param deviceContext interface for the device's the graphics context
     */
    explicit SkDeferredCanvas(SkDevice* device, DeviceContext* deviceContext);

    virtual ~SkDeferredCanvas();

    /**
     *  Specify a device to be used by this canvas. Calling setDevice will
     *  release the previously set device, if any.
     *
     *  @param device The device that the canvas will raw into
     *  @return The device argument, for convenience.
     */
    virtual SkDevice* setDevice(SkDevice* device);

    /**
     *  Specify a deviceContext to be used by this canvas. Calling
     *  setDeviceContext will release the previously set deviceContext, if any.
     *  A deviceContext must be specified if the device uses a graphics context
     *  that requires some form of state initialization prior to drawing
     *  and/or explicit flushing to synchronize the execution of rendering
     *  operations.
     *  Note: Must be called after the device is set with setDevice.
     *
     *  @deviceContext interface for the device's the graphics context
     *  @return The deviceContext argument, for convenience.
     */
    DeviceContext* setDeviceContext(DeviceContext* deviceContext);

    /**
     *  Enable or disable deferred drawing. When deferral is disabled,
     *  pending draw operations are immediately flushed and from then on,
     *  the SkDeferredCanvas behaves just like a regular SkCanvas.
     *  This method must not be called while the save/restore stack is in use.
     *  @param deferred true/false
     */
    void setDeferredDrawing(bool deferred);

    // Overrides of the SkCanvas interface
    virtual int save(SaveFlags flags) SK_OVERRIDE;
    virtual int saveLayer(const SkRect* bounds, const SkPaint* paint,
                          SaveFlags flags) SK_OVERRIDE;
    virtual void restore() SK_OVERRIDE;
    virtual bool isDrawingToLayer() const SK_OVERRIDE;
    virtual bool translate(SkScalar dx, SkScalar dy) SK_OVERRIDE;
    virtual bool scale(SkScalar sx, SkScalar sy) SK_OVERRIDE;
    virtual bool rotate(SkScalar degrees) SK_OVERRIDE;
    virtual bool skew(SkScalar sx, SkScalar sy) SK_OVERRIDE;
    virtual bool concat(const SkMatrix& matrix) SK_OVERRIDE;
    virtual void setMatrix(const SkMatrix& matrix) SK_OVERRIDE;
    virtual bool clipRect(const SkRect& rect, SkRegion::Op op,
                          bool doAntiAlias) SK_OVERRIDE;
    virtual bool clipPath(const SkPath& path, SkRegion::Op op,
                          bool doAntiAlias) SK_OVERRIDE;
    virtual bool clipRegion(const SkRegion& deviceRgn,
                            SkRegion::Op op) SK_OVERRIDE;
    virtual void clear(SkColor) SK_OVERRIDE;
    virtual void drawPaint(const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPoints(PointMode mode, size_t count, const SkPoint pts[],
                            const SkPaint& paint) SK_OVERRIDE;
    virtual void drawRect(const SkRect& rect, const SkPaint& paint)
                          SK_OVERRIDE;
    virtual void drawPath(const SkPath& path, const SkPaint& paint)
                          SK_OVERRIDE;
    virtual void drawBitmap(const SkBitmap& bitmap, SkScalar left,
                            SkScalar top, const SkPaint* paint)
                            SK_OVERRIDE;
    virtual void drawBitmapRect(const SkBitmap& bitmap, const SkIRect* src,
                                const SkRect& dst, const SkPaint* paint)
                                SK_OVERRIDE;

    virtual void drawBitmapMatrix(const SkBitmap& bitmap, const SkMatrix& m,
                                  const SkPaint* paint) SK_OVERRIDE;
    virtual void drawBitmapNine(const SkBitmap& bitmap, const SkIRect& center,
                                const SkRect& dst, const SkPaint* paint)
                                SK_OVERRIDE;
    virtual void drawSprite(const SkBitmap& bitmap, int left, int top,
                            const SkPaint* paint) SK_OVERRIDE;
    virtual void drawText(const void* text, size_t byteLength, SkScalar x,
                          SkScalar y, const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPosText(const void* text, size_t byteLength,
                             const SkPoint pos[], const SkPaint& paint)
                             SK_OVERRIDE;
    virtual void drawPosTextH(const void* text, size_t byteLength,
                              const SkScalar xpos[], SkScalar constY,
                              const SkPaint& paint) SK_OVERRIDE;
    virtual void drawTextOnPath(const void* text, size_t byteLength,
                                const SkPath& path, const SkMatrix* matrix,
                                const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPicture(SkPicture& picture) SK_OVERRIDE;
    virtual void drawVertices(VertexMode vmode, int vertexCount,
                              const SkPoint vertices[], const SkPoint texs[],
                              const SkColor colors[], SkXfermode* xmode,
                              const uint16_t indices[], int indexCount,
                              const SkPaint& paint) SK_OVERRIDE;
    virtual SkBounder* setBounder(SkBounder* bounder) SK_OVERRIDE;
    virtual SkDrawFilter* setDrawFilter(SkDrawFilter* filter) SK_OVERRIDE;

private:
    void flushIfNeeded(const SkBitmap& bitmap);

public:
    class DeviceContext : public SkRefCnt {
    public:
        virtual void prepareForDraw() {}
    };

public:
    class DeferredDevice : public SkDevice {
    public:
        /**
         *  Constructor
         *  @param immediateDevice device to be drawn to when flushing
         *      deferred operations
         *  @param deviceContext callback interface for managing graphics
         *      context state, can be NULL.
         */
        DeferredDevice(SkDevice* immediateDevice,
            DeviceContext* deviceContext = NULL);
        ~DeferredDevice();

        /**
         *  Sets the device context to be use with the device.
         *  @param deviceContext callback interface for managing graphics
         *      context state, can be NULL.
         */
        void setDeviceContext(DeviceContext* deviceContext);

        /**
         *  Returns the recording canvas.
         */
        SkCanvas* recordingCanvas() const {return fRecordingCanvas;}

        /**
         *  Returns the immediate (non deferred) canvas.
         */
        SkCanvas* immediateCanvas() const {return fImmediateCanvas;}

        /**
         *  Returns the immediate (non deferred) device.
         */
        SkDevice* immediateDevice() const {return fImmediateDevice;}

        /**
         *  Returns true if an opaque draw operation covering the entire canvas
         *  was performed since the last call to isFreshFrame().
         */
        bool isFreshFrame();

        void flushPending();
        void contentsCleared();
        void flushIfNeeded(const SkBitmap& bitmap);

        virtual uint32_t getDeviceCapabilities() SK_OVERRIDE;
        virtual int width() const SK_OVERRIDE;
        virtual int height() const SK_OVERRIDE;
        virtual SkGpuRenderTarget* accessRenderTarget() SK_OVERRIDE;

        virtual SkDevice* onCreateCompatibleDevice(SkBitmap::Config config,
                                                   int width, int height,
                                                   bool isOpaque,
                                                   Usage usage) SK_OVERRIDE;

        virtual void writePixels(const SkBitmap& bitmap, int x, int y,
                                 SkCanvas::Config8888 config8888) SK_OVERRIDE;

    protected:
        virtual const SkBitmap& onAccessBitmap(SkBitmap*) SK_OVERRIDE;
        virtual bool onReadPixels(const SkBitmap& bitmap,
                                  int x, int y,
                                  SkCanvas::Config8888 config8888) SK_OVERRIDE;

        // The following methods are no-ops on a deferred device
        virtual bool filterTextFlags(const SkPaint& paint, TextFlags*)
            SK_OVERRIDE
            {return false;}
        virtual void setMatrixClip(const SkMatrix&, const SkRegion&,
                                   const SkClipStack&) SK_OVERRIDE
            {}
        virtual void gainFocus(SkCanvas*, const SkMatrix&, const SkRegion&,
                               const SkClipStack&) SK_OVERRIDE
            {}

        // None of the following drawing methods should ever get called on the
        // deferred device
        virtual void clear(SkColor color)
            {SkASSERT(0);}
        virtual void drawPaint(const SkDraw&, const SkPaint& paint)
            {SkASSERT(0);}
        virtual void drawPoints(const SkDraw&, SkCanvas::PointMode mode,
                                size_t count, const SkPoint[],
                                const SkPaint& paint)
            {SkASSERT(0);}
        virtual void drawRect(const SkDraw&, const SkRect& r,
                              const SkPaint& paint)
            {SkASSERT(0);}
        virtual void drawPath(const SkDraw&, const SkPath& path,
                              const SkPaint& paint,
                              const SkMatrix* prePathMatrix = NULL,
                              bool pathIsMutable = false)
            {SkASSERT(0);}
        virtual void drawBitmap(const SkDraw&, const SkBitmap& bitmap,
                                const SkIRect* srcRectOrNull,
                                const SkMatrix& matrix, const SkPaint& paint)
            {SkASSERT(0);}
        virtual void drawSprite(const SkDraw&, const SkBitmap& bitmap,
                                int x, int y, const SkPaint& paint)
            {SkASSERT(0);}
        virtual void drawText(const SkDraw&, const void* text, size_t len,
                              SkScalar x, SkScalar y, const SkPaint& paint)
            {SkASSERT(0);}
        virtual void drawPosText(const SkDraw&, const void* text, size_t len,
                                 const SkScalar pos[], SkScalar constY,
                                 int scalarsPerPos, const SkPaint& paint)
            {SkASSERT(0);}
        virtual void drawTextOnPath(const SkDraw&, const void* text,
                                    size_t len, const SkPath& path,
                                    const SkMatrix* matrix,
                                    const SkPaint& paint)
            {SkASSERT(0);}
        virtual void drawPosTextOnPath(const SkDraw& draw, const void* text,
                                       size_t len, const SkPoint pos[],
                                       const SkPaint& paint,
                                       const SkPath& path,
                                       const SkMatrix* matrix)
            {SkASSERT(0);}
        virtual void drawVertices(const SkDraw&, SkCanvas::VertexMode,
                                  int vertexCount, const SkPoint verts[],
                                  const SkPoint texs[], const SkColor colors[],
                                  SkXfermode* xmode, const uint16_t indices[],
                                  int indexCount, const SkPaint& paint)
            {SkASSERT(0);}
        virtual void drawDevice(const SkDraw&, SkDevice*, int x, int y,
                                const SkPaint&)
            {SkASSERT(0);}
    private:
        virtual void flush();

        SkPicture fPicture;
        SkDevice* fImmediateDevice;
        SkCanvas* fImmediateCanvas;
        SkCanvas* fRecordingCanvas;
        DeviceContext* fDeviceContext;
        bool fFreshFrame;
    };

    DeferredDevice* getDeferredDevice() const;

protected:
    virtual SkCanvas* canvasForDrawIter();

private:
    SkCanvas* drawingCanvas() const;
    bool isFullFrame(const SkRect*, const SkPaint*) const;
    void validate() const;
    void init();
    bool            fDeferredDrawing;

    typedef SkCanvas INHERITED;
};


#endif
