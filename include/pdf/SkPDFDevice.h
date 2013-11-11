
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPDFDevice_DEFINED
#define SkPDFDevice_DEFINED

#include "SkBitmapDevice.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkPicture.h"
#include "SkRect.h"
#include "SkRefCnt.h"
#include "SkStream.h"
#include "SkTDArray.h"
#include "SkTemplates.h"

class SkPDFArray;
class SkPDFDevice;
class SkPDFDict;
class SkPDFFont;
class SkPDFFormXObject;
class SkPDFGlyphSetMap;
class SkPDFGraphicState;
class SkPDFObject;
class SkPDFResourceDict;
class SkPDFShader;
class SkPDFStream;
class SkRRect;
template <typename T> class SkTSet;

// Private classes.
struct ContentEntry;
struct GraphicStateEntry;
struct NamedDestination;

/** \class SkPDFDevice

    The drawing context for the PDF backend.
*/
class SkPDFDevice : public SkBitmapDevice {
public:
    /** Create a PDF drawing context with the given width and height.
     *  72 points/in means letter paper is 612x792.
     *  @param pageSize Page size in points.
     *  @param contentSize The content size of the page in points. This will be
     *         combined with the initial transform to determine the drawing area
     *         (as reported by the width and height methods). Anything outside
     *         of the drawing area will be clipped.
     *  @param initialTransform The initial transform to apply to the page.
     *         This may be useful to, for example, move the origin in and
     *         over a bit to account for a margin, scale the canvas,
     *         or apply a rotation.  Note1: the SkPDFDevice also applies
     *         a scale+translate transform to move the origin from the
     *         bottom left (PDF default) to the top left.  Note2: drawDevice
     *         (used by layer restore) draws the device after this initial
     *         transform is applied, so the PDF device does an
     *         inverse scale+translate to accommodate the one that SkPDFDevice
     *         always does.
     */
    // Deprecated, please use SkDocument::CreatePdf() instead.
    SK_API SkPDFDevice(const SkISize& pageSize, const SkISize& contentSize,
                       const SkMatrix& initialTransform);
    SK_API virtual ~SkPDFDevice();

    virtual uint32_t getDeviceCapabilities() SK_OVERRIDE;

    virtual void clear(SkColor color) SK_OVERRIDE;

    /** These are called inside the per-device-layer loop for each draw call.
     When these are called, we have already applied any saveLayer operations,
     and are handling any looping from the paint, and any effects from the
     DrawFilter.
     */
    virtual void drawPaint(const SkDraw&, const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPoints(const SkDraw&, SkCanvas::PointMode mode,
                            size_t count, const SkPoint[],
                            const SkPaint& paint) SK_OVERRIDE;
    virtual void drawRect(const SkDraw&, const SkRect& r, const SkPaint& paint);
    virtual void drawRRect(const SkDraw&, const SkRRect& rr,
                           const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPath(const SkDraw&, const SkPath& origpath,
                          const SkPaint& paint, const SkMatrix* prePathMatrix,
                          bool pathIsMutable) SK_OVERRIDE;
    virtual void drawBitmapRect(const SkDraw& draw, const SkBitmap& bitmap,
                                const SkRect* src, const SkRect& dst,
                                const SkPaint& paint,
                                SkCanvas::DrawBitmapRectFlags flags) SK_OVERRIDE;
    virtual void drawBitmap(const SkDraw&, const SkBitmap& bitmap,
                            const SkMatrix& matrix, const SkPaint&) SK_OVERRIDE;
    virtual void drawSprite(const SkDraw&, const SkBitmap& bitmap, int x, int y,
                            const SkPaint& paint) SK_OVERRIDE;
    virtual void drawText(const SkDraw&, const void* text, size_t len,
                          SkScalar x, SkScalar y, const SkPaint&) SK_OVERRIDE;
    virtual void drawPosText(const SkDraw&, const void* text, size_t len,
                             const SkScalar pos[], SkScalar constY,
                             int scalarsPerPos, const SkPaint&) SK_OVERRIDE;
    virtual void drawTextOnPath(const SkDraw&, const void* text, size_t len,
                                const SkPath& path, const SkMatrix* matrix,
                                const SkPaint& paint) SK_OVERRIDE;
    virtual void drawVertices(const SkDraw&, SkCanvas::VertexMode,
                              int vertexCount, const SkPoint verts[],
                              const SkPoint texs[], const SkColor colors[],
                              SkXfermode* xmode, const uint16_t indices[],
                              int indexCount, const SkPaint& paint) SK_OVERRIDE;
    virtual void drawDevice(const SkDraw&, SkBaseDevice*, int x, int y,
                            const SkPaint&) SK_OVERRIDE;

    virtual void onAttachToCanvas(SkCanvas* canvas) SK_OVERRIDE;
    virtual void onDetachFromCanvas() SK_OVERRIDE;

    enum DrawingArea {
        kContent_DrawingArea,  // Drawing area for the page content.
        kMargin_DrawingArea,   // Drawing area for the margin content.
    };

    /** Sets the drawing area for the device. Subsequent draw calls are directed
     *  to the specific drawing area (margin or content). The default drawing
     *  area is the content drawing area.
     *
     *  Currently if margin content is drawn and then a complex (for PDF) xfer
     *  mode is used, like SrcIn, Clear, etc, the margin content will get
     *  clipped. A simple way to avoid the bug is to always draw the margin
     *  content last.
     */
    SK_API void setDrawingArea(DrawingArea drawingArea);

    /** Sets the DCTEncoder for images.
     *  @param encoder The encoder to encode a bitmap as JPEG (DCT).
     *         Result of encodings are cached, if the encoder changes the
     *         behaivor dynamically and an image is added to a second catalog,
     *         we will likely use the result of the first encoding call.
     *         By returning false from the encoder function, the encoder result
     *         is not used.
     *         Callers might not want to encode small images, as the time spent
     *         encoding and decoding might not be worth the space savings,
     *         if any at all.
     */
    void setDCTEncoder(SkPicture::EncodeBitmap encoder) {
        fEncoder = encoder;
    }

    // PDF specific methods.

    /** Returns the resource dictionary for this device.
     */
    SK_API SkPDFResourceDict* getResourceDict();

    /** Get the fonts used on this device.
     */
    SK_API const SkTDArray<SkPDFFont*>& getFontResources() const;

    /** Add our named destinations to the supplied dictionary.
     *  @param dict  Dictionary to add destinations to.
     *  @param page  The PDF object representing the page for this device.
     */
    void appendDestinations(SkPDFDict* dict, SkPDFObject* page);

    /** Returns a copy of the media box for this device. The caller is required
     *  to unref() this when it is finished.
     */
    SK_API SkPDFArray* copyMediaBox() const;

    /** Get the annotations from this page, or NULL if there are none.
     */
    SK_API SkPDFArray* getAnnotations() const { return fAnnotations; }

    /** Returns a SkStream with the page contents.  The caller is responsible
        for a reference to the returned value.
        DEPRECATED: use copyContentToData()
     */
    SK_API SkStream* content() const;

    /** Returns a SkStream with the page contents.  The caller is responsible
     *  for calling data->unref() when it is finished.
     */
    SK_API SkData* copyContentToData() const;

    SK_API const SkMatrix& initialTransform() const {
        return fInitialTransform;
    }

    /** Returns a SkPDFGlyphSetMap which represents glyph usage of every font
     *  that shows on this device.
     */
    const SkPDFGlyphSetMap& getFontGlyphUsage() const {
        return *(fFontGlyphUsage.get());
    }


    /**
     *  rasterDpi - the DPI at which features without native PDF support
     *              will be rasterized (e.g. draw image with perspective,
     *              draw text with perspective, ...)
     *              A larger DPI would create a PDF that reflects the original
     *              intent with better fidelity, but it can make for larger
     *              PDF files too, which would use more memory while rendering,
     *              and it would be slower to be processed or sent online or
     *              to printer.
     */
    void setRasterDpi(SkScalar rasterDpi) {
        fRasterDpi = rasterDpi;
    }

protected:
    virtual bool onReadPixels(const SkBitmap& bitmap, int x, int y,
                              SkCanvas::Config8888) SK_OVERRIDE;

    virtual bool allowImageFilter(SkImageFilter*) SK_OVERRIDE;

private:
    // TODO(vandebo): push most of SkPDFDevice's state into a core object in
    // order to get the right access levels without using friend.
    friend class ScopedContentEntry;

    SkISize fPageSize;
    SkISize fContentSize;
    SkMatrix fInitialTransform;
    SkClipStack fExistingClipStack;
    SkRegion fExistingClipRegion;
    SkPDFArray* fAnnotations;
    SkPDFResourceDict* fResourceDict;
    SkTDArray<NamedDestination*> fNamedDestinations;

    SkTDArray<SkPDFGraphicState*> fGraphicStateResources;
    SkTDArray<SkPDFObject*> fXObjectResources;
    SkTDArray<SkPDFFont*> fFontResources;
    SkTDArray<SkPDFObject*> fShaderResources;

    SkAutoTDelete<ContentEntry> fContentEntries;
    ContentEntry* fLastContentEntry;
    SkAutoTDelete<ContentEntry> fMarginContentEntries;
    ContentEntry* fLastMarginContentEntry;
    DrawingArea fDrawingArea;

    const SkClipStack* fClipStack;

    // Accessor and setter functions based on the current DrawingArea.
    SkAutoTDelete<ContentEntry>* getContentEntries();
    ContentEntry* getLastContentEntry();
    void setLastContentEntry(ContentEntry* contentEntry);

    // Glyph ids used for each font on this device.
    SkAutoTDelete<SkPDFGlyphSetMap> fFontGlyphUsage;

    SkPicture::EncodeBitmap fEncoder;
    SkScalar fRasterDpi;

    SkPDFDevice(const SkISize& layerSize, const SkClipStack& existingClipStack,
                const SkRegion& existingClipRegion);

    // override from SkBaseDevice
    virtual SkBaseDevice* onCreateCompatibleDevice(SkBitmap::Config config,
                                                   int width, int height,
                                                   bool isOpaque,
                                                   Usage usage) SK_OVERRIDE;

    void init();
    void cleanUp(bool clearFontUsage);
    SkPDFFormXObject* createFormXObjectFromDevice();

    void drawFormXObjectWithMask(int xObjectIndex,
                                 SkPDFFormXObject* mask,
                                 const SkClipStack* clipStack,
                                 const SkRegion& clipRegion,
                                 SkXfermode::Mode mode,
                                 bool invertClip);

    // If the paint or clip is such that we shouldn't draw anything, this
    // returns NULL and does not create a content entry.
    // setUpContentEntry and finishContentEntry can be used directly, but
    // the preferred method is to use the ScopedContentEntry helper class.
    ContentEntry* setUpContentEntry(const SkClipStack* clipStack,
                                    const SkRegion& clipRegion,
                                    const SkMatrix& matrix,
                                    const SkPaint& paint,
                                    bool hasText,
                                    SkPDFFormXObject** dst);
    void finishContentEntry(SkXfermode::Mode xfermode,
                            SkPDFFormXObject* dst,
                            SkPath* shape);
    bool isContentEmpty();

    void populateGraphicStateEntryFromPaint(const SkMatrix& matrix,
                                            const SkClipStack& clipStack,
                                            const SkRegion& clipRegion,
                                            const SkPaint& paint,
                                            bool hasText,
                                            GraphicStateEntry* entry);
    int addGraphicStateResource(SkPDFGraphicState* gs);
    int addXObjectResource(SkPDFObject* xObject);

    void updateFont(const SkPaint& paint, uint16_t glyphID,
                    ContentEntry* contentEntry);
    int getFontResourceIndex(SkTypeface* typeface, uint16_t glyphID);

    void internalDrawPaint(const SkPaint& paint, ContentEntry* contentEntry);
    void internalDrawBitmap(const SkMatrix& matrix,
                            const SkClipStack* clipStack,
                            const SkRegion& clipRegion,
                            const SkBitmap& bitmap,
                            const SkIRect* srcRect,
                            const SkPaint& paint);

    /** Helper method for copyContentToData. It is responsible for copying the
     *  list of content entries |entry| to |data|.
     */
    void copyContentEntriesToData(ContentEntry* entry, SkWStream* data) const;

#ifdef SK_PDF_USE_PATHOPS
    bool handleInversePath(const SkDraw& d, const SkPath& origPath,
                           const SkPaint& paint, bool pathIsMutable,
                           const SkMatrix* prePathMatrix = NULL);
#endif
    bool handleRectAnnotation(const SkRect& r, const SkMatrix& matrix,
                              const SkPaint& paint);
    bool handlePointAnnotation(const SkPoint* points, size_t count,
                               const SkMatrix& matrix, const SkPaint& paint);
    SkPDFDict* createLinkAnnotation(const SkRect& r, const SkMatrix& matrix);
    void handleLinkToURL(SkData* urlData, const SkRect& r,
                         const SkMatrix& matrix);
    void handleLinkToNamedDest(SkData* nameData, const SkRect& r,
                               const SkMatrix& matrix);
    void defineNamedDestination(SkData* nameData, const SkPoint& point,
                                const SkMatrix& matrix);

    typedef SkBitmapDevice INHERITED;

    // TODO(edisonn): Only SkDocument_PDF and SkPDFImageShader should be able to create
    // an SkPDFDevice
    //friend class SkDocument_PDF;
    //friend class SkPDFImageShader;
};

#endif
