/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPDFDevice_DEFINED
#define SkPDFDevice_DEFINED

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkClipStack.h"
#include "SkDevice.h"
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
class SkPDFShader;
class SkPDFStream;
class SkRRect;

// Private classes.
struct ContentEntry;
struct GraphicStateEntry;
struct NamedDestination;
struct RectWithData;

/** \class SkPDFDevice

    The drawing context for the PDF backend.
*/
class SkPDFDevice final : public SkBaseDevice {
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
        return new SkPDFDevice(pageSize, rasterDpi, canon, true);
    }

    /** Create a PDF drawing context without fipping the y-axis. */
    static SkPDFDevice* CreateUnflipped(SkISize pageSize,
                                        SkScalar rasterDpi,
                                        SkPDFCanon* canon) {
        return new SkPDFDevice(pageSize, rasterDpi, canon, false);
    }

    virtual ~SkPDFDevice();

    /** These are called inside the per-device-layer loop for each draw call.
     When these are called, we have already applied any saveLayer operations,
     and are handling any looping from the paint, and any effects from the
     DrawFilter.
     */
    void drawPaint(const SkDraw&, const SkPaint& paint) override;
    void drawPoints(const SkDraw&, SkCanvas::PointMode mode,
                    size_t count, const SkPoint[],
                    const SkPaint& paint) override;
    void drawRect(const SkDraw&, const SkRect& r, const SkPaint& paint) override;
    void drawOval(const SkDraw&, const SkRect& oval, const SkPaint& paint) override;
    void drawRRect(const SkDraw&, const SkRRect& rr, const SkPaint& paint) override;
    void drawPath(const SkDraw&, const SkPath& origpath,
                  const SkPaint& paint, const SkMatrix* prePathMatrix,
                  bool pathIsMutable) override;
    void drawBitmapRect(const SkDraw& draw, const SkBitmap& bitmap, const SkRect* src,
                        const SkRect& dst, const SkPaint&, SkCanvas::SrcRectConstraint) override;
    void drawBitmap(const SkDraw&, const SkBitmap& bitmap,
                    const SkMatrix& matrix, const SkPaint&) override;
    void drawSprite(const SkDraw&, const SkBitmap& bitmap, int x, int y,
                    const SkPaint& paint) override;
    void drawImage(const SkDraw&,
                   const SkImage*,
                   SkScalar x,
                   SkScalar y,
                   const SkPaint&) override;
    void drawImageRect(const SkDraw&,
                       const SkImage*,
                       const SkRect* src,
                       const SkRect& dst,
                       const SkPaint&,
                       SkCanvas::SrcRectConstraint) override;
    void drawText(const SkDraw&, const void* text, size_t len,
                  SkScalar x, SkScalar y, const SkPaint&) override;
    void drawPosText(const SkDraw&, const void* text, size_t len,
                     const SkScalar pos[], int scalarsPerPos,
                     const SkPoint& offset, const SkPaint&) override;
    void drawVertices(const SkDraw&, SkCanvas::VertexMode,
                      int vertexCount, const SkPoint verts[],
                      const SkPoint texs[], const SkColor colors[],
                      SkXfermode* xmode, const uint16_t indices[],
                      int indexCount, const SkPaint& paint) override;
    void drawDevice(const SkDraw&, SkBaseDevice*, int x, int y,
                    const SkPaint&) override;

    void onAttachToCanvas(SkCanvas* canvas) override;
    void onDetachFromCanvas() override;
    SkImageInfo imageInfo() const override;

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

    /** Create the resource dictionary for this device.
     */
    SkPDFDict* createResourceDict() const;

    /** Get the fonts used on this device.
     */
    const SkTDArray<SkPDFFont*>& getFontResources() const;

    /** Add our annotations (link to urls and destinations) to the supplied
     *  array.
     *  @param array Array to add annotations to.
     */
    void appendAnnotations(SkPDFArray* array) const;

    /** Add our named destinations to the supplied dictionary.
     *  @param dict  Dictionary to add destinations to.
     *  @param page  The PDF object representing the page for this device.
     */
    void appendDestinations(SkPDFDict* dict, SkPDFObject* page) const;

    /** Returns a copy of the media box for this device. The caller is required
     *  to unref() this when it is finished.
     */
    SkPDFArray* copyMediaBox() const;

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

    SkPDFCanon* getCanon() const { return fCanon; }

protected:
    const SkBitmap& onAccessBitmap() override {
        return fLegacyBitmap;
    }

    SkSurface* newSurface(const SkImageInfo&, const SkSurfaceProps&) override;

private:
    // TODO(vandebo): push most of SkPDFDevice's state into a core object in
    // order to get the right access levels without using friend.
    friend class ScopedContentEntry;

    SkISize fPageSize;
    SkISize fContentSize;
    SkMatrix fInitialTransform;
    SkClipStack fExistingClipStack;
    SkRegion fExistingClipRegion;

    SkTDArray<RectWithData*> fLinkToURLs;
    SkTDArray<RectWithData*> fLinkToDestinations;
    SkTDArray<NamedDestination*> fNamedDestinations;

    SkTDArray<SkPDFObject*> fGraphicStateResources;
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

    SkBaseDevice* onCreateDevice(const CreateInfo&, const SkPaint*) override;

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
    // returns nullptr and does not create a content entry.
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
    int addGraphicStateResource(SkPDFObject* gs);
    int addXObjectResource(SkPDFObject* xObject);

    void updateFont(const SkPaint& paint, uint16_t glyphID, ContentEntry* contentEntry);
    int getFontResourceIndex(SkTypeface* typeface, uint16_t glyphID);

    void internalDrawPaint(const SkPaint& paint, ContentEntry* contentEntry);
    void internalDrawImage(const SkMatrix& matrix,
                           const SkClipStack* clipStack,
                           const SkRegion& clipRegion,
                           const SkImage* image,
                           const SkIRect* srcRect,
                           const SkPaint& paint);

    /** Helper method for copyContentToData. It is responsible for copying the
     *  list of content entries |entry| to |data|.
     */
    void copyContentEntriesToData(ContentEntry* entry, SkWStream* data) const;

    bool handleInversePath(const SkDraw& d, const SkPath& origPath,
                           const SkPaint& paint, bool pathIsMutable,
                           const SkMatrix* prePathMatrix = nullptr);
    bool handlePointAnnotation(const SkPoint* points, size_t count,
                               const SkMatrix& matrix, SkAnnotation* annot);
    bool handlePathAnnotation(const SkPath& path, const SkDraw& d,
                              SkAnnotation* annot);

    typedef SkBaseDevice INHERITED;

    // TODO(edisonn): Only SkDocument_PDF and SkPDFImageShader should be able to create
    // an SkPDFDevice
    //friend class SkDocument_PDF;
    //friend class SkPDFImageShader;
};

#endif
