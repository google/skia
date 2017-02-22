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
#include "SkClipStackDevice.h"
#include "SkData.h"
#include "SkPaint.h"
#include "SkRect.h"
#include "SkRefCnt.h"
#include "SkSinglyLinkedList.h"
#include "SkStream.h"
#include "SkTDArray.h"
#include "SkTextBlob.h"

class SkImageSubset;
class SkPath;
class SkPDFArray;
class SkPDFCanon;
class SkPDFDevice;
class SkPDFDocument;
class SkPDFDict;
class SkPDFFont;
class SkPDFObject;
class SkPDFStream;
class SkRRect;

/** \class SkPDFDevice

    The drawing context for the PDF backend.
*/
class SkPDFDevice final : public SkClipStackDevice {
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
     *  @param SkPDFDocument.  A non-null pointer back to the
     *         document.  The document is repsonsible for
     *         de-duplicating across pages (via the SkPDFCanon) and
     *         for early serializing of large immutable objects, such
     *         as images (via SkPDFDocument::serialize()).
     */
    static SkPDFDevice* Create(SkISize pageSize,
                               SkScalar rasterDpi,
                               SkPDFDocument* doc) {
        return new SkPDFDevice(pageSize, rasterDpi, doc, true);
    }

    /** Create a PDF drawing context without fipping the y-axis. */
    static SkPDFDevice* CreateUnflipped(SkISize pageSize,
                                        SkScalar rasterDpi,
                                        SkPDFDocument* doc) {
        return new SkPDFDevice(pageSize, rasterDpi, doc, false);
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
    void drawTextBlob(const SkDraw&, const SkTextBlob*, SkScalar x, SkScalar y,
                      const SkPaint &, SkDrawFilter*) override;
    void drawVertices(const SkDraw&, SkCanvas::VertexMode,
                      int vertexCount, const SkPoint verts[],
                      const SkPoint texs[], const SkColor colors[],
                      SkBlendMode, const uint16_t indices[],
                      int indexCount, const SkPaint& paint) override;
    void drawDevice(const SkDraw&, SkBaseDevice*, int x, int y,
                    const SkPaint&) override;

    // PDF specific methods.

    /** Create the resource dictionary for this device. */
    sk_sp<SkPDFDict> makeResourceDict() const;

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

    /** Returns a copy of the media box for this device. */
    sk_sp<SkPDFArray> copyMediaBox() const;

    /** Returns a SkStream with the page contents.
     */
    std::unique_ptr<SkStreamAsset> content() const;

    SkPDFCanon* getCanon() const;

    // It is important to not confuse GraphicStateEntry with SkPDFGraphicState, the
    // later being our representation of an object in the PDF file.
    struct GraphicStateEntry {
        GraphicStateEntry();

        // Compare the fields we care about when setting up a new content entry.
        bool compareInitialState(const GraphicStateEntry& b);

        SkMatrix fMatrix;
        // We can't do set operations on Paths, though PDF natively supports
        // intersect.  If the clip stack does anything other than intersect,
        // we have to fall back to the region.  Treat fClipStack as authoritative.
        // See https://bugs.skia.org/221
        SkClipStack fClipStack;
        SkRegion fClipRegion;

        // When emitting the content entry, we will ensure the graphic state
        // is set to these values first.
        SkColor fColor;
        SkScalar fTextScaleX;  // Zero means we don't care what the value is.
        SkPaint::Style fTextFill;  // Only if TextScaleX is non-zero.
        int fShaderIndex;
        int fGraphicStateIndex;
    };

protected:
    sk_sp<SkSurface> makeSurface(const SkImageInfo&, const SkSurfaceProps&) override;

    void drawAnnotation(const SkDraw&, const SkRect&, const char key[], SkData* value) override;

    void drawSpecial(const SkDraw&, SkSpecialImage*, int x, int y, const SkPaint&) override;
    sk_sp<SkSpecialImage> makeSpecial(const SkBitmap&) override;
    sk_sp<SkSpecialImage> makeSpecial(const SkImage*) override;
    sk_sp<SkSpecialImage> snapSpecial() override;
    SkImageFilterCache* getImageFilterCache() override;

private:
    struct RectWithData {
        SkRect rect;
        sk_sp<SkData> data;
        RectWithData(const SkRect& rect, SkData* data)
            : rect(rect), data(SkRef(data)) {}
        RectWithData(RectWithData&&) = default;
        RectWithData& operator=(RectWithData&& other) = default;
    };

    struct NamedDestination {
        sk_sp<SkData> nameData;
        SkPoint point;
        NamedDestination(SkData* nameData, const SkPoint& point)
            : nameData(SkRef(nameData)), point(point) {}
        NamedDestination(NamedDestination&&) = default;
        NamedDestination& operator=(NamedDestination&&) = default;
    };

    // TODO(vandebo): push most of SkPDFDevice's state into a core object in
    // order to get the right access levels without using friend.
    friend class ScopedContentEntry;

    SkISize fPageSize;
    SkMatrix fInitialTransform;
    SkClipStack fExistingClipStack;
    SkRegion fExistingClipRegion;

    SkTArray<RectWithData> fLinkToURLs;
    SkTArray<RectWithData> fLinkToDestinations;
    SkTArray<NamedDestination> fNamedDestinations;

    SkTDArray<SkPDFObject*> fGraphicStateResources;
    SkTDArray<SkPDFObject*> fXObjectResources;
    SkTDArray<SkPDFFont*> fFontResources;
    SkTDArray<SkPDFObject*> fShaderResources;

    struct ContentEntry {
        GraphicStateEntry fState;
        SkDynamicMemoryWStream fContent;
    };
    SkSinglyLinkedList<ContentEntry> fContentEntries;

    SkScalar fRasterDpi;

    SkPDFDocument* fDocument;
    ////////////////////////////////////////////////////////////////////////////

    SkPDFDevice(SkISize pageSize,
                SkScalar rasterDpi,
                SkPDFDocument* doc,
                bool flip);

    SkBaseDevice* onCreateDevice(const CreateInfo&, const SkPaint*) override;

    void init();
    void cleanUp();
    sk_sp<SkPDFObject> makeFormXObjectFromDevice();

    void drawFormXObjectWithMask(int xObjectIndex,
                                 sk_sp<SkPDFObject> mask,
                                 const SkClipStack* clipStack,
                                 const SkRegion& clipRegion,
                                 SkBlendMode,
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
                                    sk_sp<SkPDFObject>* dst);
    void finishContentEntry(SkBlendMode, sk_sp<SkPDFObject> dst, SkPath* shape);
    bool isContentEmpty();

    void populateGraphicStateEntryFromPaint(const SkMatrix& matrix,
                                            const SkClipStack& clipStack,
                                            const SkRegion& clipRegion,
                                            const SkPaint& paint,
                                            bool hasText,
                                            GraphicStateEntry* entry);
    int addGraphicStateResource(SkPDFObject* gs);
    int addXObjectResource(SkPDFObject* xObject);

    int getFontResourceIndex(SkTypeface* typeface, uint16_t glyphID);


    void internalDrawText(const SkDraw&, const void*, size_t, const SkScalar pos[],
                          SkTextBlob::GlyphPositioning, SkPoint, const SkPaint&,
                          const uint32_t*, uint32_t, const char*);

    void internalDrawPaint(const SkPaint& paint, ContentEntry* contentEntry);

    void internalDrawImage(const SkMatrix& origMatrix,
                           const SkClipStack* clipStack,
                           const SkRegion& origClipRegion,
                           SkImageSubset imageSubset,
                           const SkPaint& paint);

    bool handleInversePath(const SkDraw& d, const SkPath& origPath,
                           const SkPaint& paint, bool pathIsMutable,
                           const SkMatrix* prePathMatrix = nullptr);
    void handlePointAnnotation(const SkPoint&, const SkMatrix&, const char key[], SkData* value);
    void handleRectAnnotation(const SkRect&, const SkDraw& d, const char key[], SkData* value);

    typedef SkClipStackDevice INHERITED;

    // TODO(edisonn): Only SkDocument_PDF and SkPDFImageShader should be able to create
    // an SkPDFDevice
    //friend class SkDocument_PDF;
    //friend class SkPDFImageShader;
};

#endif
