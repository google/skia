
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPDFDevice_DEFINED
#define SkPDFDevice_DEFINED

#include "SkDevice.h"
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
class SkPDFCanon;
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
class SkPDFDevice : public SkBaseDevice {
public:
    /** Create a PDF drawing context.  SkPDFDevice applies a
     *  scale-and-translate transform to move the origin from the
     *  bottom left (PDF default) to the top left (Skia default).
     *  @param pageSize Page size in point units.
     *         1 point == 127/360 mm == 1/72 inch
     *  @param rasterDpi the DPI at which features without native PDF
     *         support will be rasterized (e.g. draw image with
     *         perspective, draw text with perspective, ...).  A
     *         larger DPI would create a PDF that reflects the
     *         original intent with better fidelity, but it can make
     *         for larger PDF files too, which would use more memory
     *         while rendering, and it would be slower to be processed
     *         or sent online or to printer.  A good choice is
     *         SK_ScalarDefaultRasterDPI(72.0f).
     *  @param SkPDFCanon.  Should be non-null, and shared by all
     *         devices in a document.
     */
    static SkPDFDevice* Create(SkISize pageSize,
                               SkScalar rasterDpi,
                               SkPDFCanon* canon) {
        return SkNEW_ARGS(SkPDFDevice, (pageSize, rasterDpi, canon, true));
    }

    /** Create a PDF drawing context without fipping the y-axis. */
    static SkPDFDevice* CreateUnflipped(SkISize pageSize,
                                        SkScalar rasterDpi,
                                        SkPDFCanon* canon) {
        return SkNEW_ARGS(SkPDFDevice, (pageSize, rasterDpi, canon, false));
    }

    virtual ~SkPDFDevice();

    /** These are called inside the per-device-layer loop for each draw call.
     When these are called, we have already applied any saveLayer operations,
     and are handling any looping from the paint, and any effects from the
     DrawFilter.
     */
    void drawPaint(const SkDraw&, const SkPaint& paint) SK_OVERRIDE;
    void drawPoints(const SkDraw&, SkCanvas::PointMode mode,
                    size_t count, const SkPoint[],
                    const SkPaint& paint) SK_OVERRIDE;
    void drawRect(const SkDraw&, const SkRect& r, const SkPaint& paint) SK_OVERRIDE;
    void drawOval(const SkDraw&, const SkRect& oval, const SkPaint& paint) SK_OVERRIDE;
    void drawRRect(const SkDraw&, const SkRRect& rr, const SkPaint& paint) SK_OVERRIDE;
    void drawPath(const SkDraw&, const SkPath& origpath,
                  const SkPaint& paint, const SkMatrix* prePathMatrix,
                  bool pathIsMutable) SK_OVERRIDE;
    void drawBitmapRect(const SkDraw& draw, const SkBitmap& bitmap,
                        const SkRect* src, const SkRect& dst,
                        const SkPaint& paint,
                        SkCanvas::DrawBitmapRectFlags flags) SK_OVERRIDE;
    void drawBitmap(const SkDraw&, const SkBitmap& bitmap,
                    const SkMatrix& matrix, const SkPaint&) SK_OVERRIDE;
    void drawSprite(const SkDraw&, const SkBitmap& bitmap, int x, int y,
                    const SkPaint& paint) SK_OVERRIDE;
    void drawText(const SkDraw&, const void* text, size_t len,
                  SkScalar x, SkScalar y, const SkPaint&) SK_OVERRIDE;
    void drawPosText(const SkDraw&, const void* text, size_t len,
                     const SkScalar pos[], int scalarsPerPos,
                     const SkPoint& offset, const SkPaint&) SK_OVERRIDE;
    void drawVertices(const SkDraw&, SkCanvas::VertexMode,
                      int vertexCount, const SkPoint verts[],
                      const SkPoint texs[], const SkColor colors[],
                      SkXfermode* xmode, const uint16_t indices[],
                      int indexCount, const SkPaint& paint) SK_OVERRIDE;
    void drawDevice(const SkDraw&, SkBaseDevice*, int x, int y,
                    const SkPaint&) SK_OVERRIDE;

    void onAttachToCanvas(SkCanvas* canvas) SK_OVERRIDE;
    void onDetachFromCanvas() SK_OVERRIDE;
    SkImageInfo imageInfo() const SK_OVERRIDE;

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
    void setDrawingArea(DrawingArea drawingArea);

    // PDF specific methods.

    /** Returns the resource dictionary for this device.
     */
    SkPDFResourceDict* getResourceDict();

    /** Get the fonts used on this device.
     */
    const SkTDArray<SkPDFFont*>& getFontResources() const;

    /** Add our named destinations to the supplied dictionary.
     *  @param dict  Dictionary to add destinations to.
     *  @param page  The PDF object representing the page for this device.
     */
    void appendDestinations(SkPDFDict* dict, SkPDFObject* page);

    /** Returns a copy of the media box for this device. The caller is required
     *  to unref() this when it is finished.
     */
    SkPDFArray* copyMediaBox() const;

    /** Get the annotations from this page, or NULL if there are none.
     */
    SkPDFArray* getAnnotations() const { return fAnnotations; }

    /** Returns a SkStream with the page contents.  The caller is responsible
     *  for a deleting the returned value.
     */
    SkStreamAsset* content() const;

    /** Writes the page contents to the stream. */
    void writeContent(SkWStream*) const;

    const SkMatrix& initialTransform() const {
        return fInitialTransform;
    }

    /** Returns a SkPDFGlyphSetMap which represents glyph usage of every font
     *  that shows on this device.
     */
    const SkPDFGlyphSetMap& getFontGlyphUsage() const {
        return *(fFontGlyphUsage.get());
    }

protected:
    const SkBitmap& onAccessBitmap() SK_OVERRIDE {
        return fLegacyBitmap;
    }

    SkSurface* newSurface(const SkImageInfo&, const SkSurfaceProps&) SK_OVERRIDE;

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

    // Glyph ids used for each font on this device.
    SkAutoTDelete<SkPDFGlyphSetMap> fFontGlyphUsage;

    SkScalar fRasterDpi;

    SkBitmap fLegacyBitmap;

    SkPDFCanon* fCanon;  // Owned by SkDocument_PDF
    ////////////////////////////////////////////////////////////////////////////

    SkPDFDevice(SkISize pageSize,
                SkScalar rasterDpi,
                SkPDFCanon* canon,
                bool flip);

    ContentEntry* getLastContentEntry();
    void setLastContentEntry(ContentEntry* contentEntry);

    // override from SkBaseDevice
    SkBaseDevice* onCreateCompatibleDevice(const CreateInfo&) SK_OVERRIDE;

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

    typedef SkBaseDevice INHERITED;

    // TODO(edisonn): Only SkDocument_PDF and SkPDFImageShader should be able to create
    // an SkPDFDevice
    //friend class SkDocument_PDF;
    //friend class SkPDFImageShader;
};

#endif
