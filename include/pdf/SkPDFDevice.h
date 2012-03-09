
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPDFDevice_DEFINED
#define SkPDFDevice_DEFINED

#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkRefCnt.h"
#include "SkStream.h"
#include "SkTScopedPtr.h"

class SkPDFArray;
class SkPDFDevice;
class SkPDFDict;
class SkPDFFont;
class SkPDFFormXObject;
class SkPDFGlyphSetMap;
class SkPDFGraphicState;
class SkPDFObject;
class SkPDFShader;
class SkPDFStream;

// Private classes.
struct ContentEntry;
struct GraphicStateEntry;

/** \class SkPDFDevice

    The drawing context for the PDF backend.
*/
class SkPDFDevice : public SkDevice {
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
    // TODO(vandebo): The sizes should be SkSize and not SkISize.
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
    virtual void drawPath(const SkDraw&, const SkPath& origpath,
                          const SkPaint& paint, const SkMatrix* prePathMatrix,
                          bool pathIsMutable) SK_OVERRIDE;
    virtual void drawBitmap(const SkDraw&, const SkBitmap& bitmap,
                            const SkIRect* srcRectOrNull,
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
    virtual void drawDevice(const SkDraw&, SkDevice*, int x, int y,
                            const SkPaint&) SK_OVERRIDE;

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

    // PDF specific methods.

    /** Returns the resource dictionary for this device.
     */
    SK_API SkPDFDict* getResourceDict();

    /** Get the list of resources (PDF objects) used on this page.
     *  @param resourceList A list to append the resources to.
     *  @param recursive    If recursive is true, get the resources of the
     *                      device's resources recursively. (Useful for adding
     *                      objects to the catalog.)
     */
    SK_API void getResources(SkTDArray<SkPDFObject*>* resourceList,
                             bool recursive) const;

    /** Get the fonts used on this device.
     */
    SK_API const SkTDArray<SkPDFFont*>& getFontResources() const;

    /** Returns the media box for this device.
     */
    SK_API SkRefPtr<SkPDFArray> getMediaBox() const;

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
    SkRefPtr<SkPDFDict> fResourceDict;

    SkTDArray<SkPDFGraphicState*> fGraphicStateResources;
    SkTDArray<SkPDFObject*> fXObjectResources;
    SkTDArray<SkPDFFont*> fFontResources;
    SkTDArray<SkPDFObject*> fShaderResources;

    SkTScopedPtr<ContentEntry> fContentEntries;
    ContentEntry* fLastContentEntry;
    SkTScopedPtr<ContentEntry> fMarginContentEntries;
    ContentEntry* fLastMarginContentEntry;
    DrawingArea fDrawingArea;

    // Accessor and setter functions based on the current DrawingArea.
    SkTScopedPtr<ContentEntry>* getContentEntries();
    ContentEntry* getLastContentEntry();
    void setLastContentEntry(ContentEntry* contentEntry);

    // Glyph ids used for each font on this device.
    SkTScopedPtr<SkPDFGlyphSetMap> fFontGlyphUsage;

    SkPDFDevice(const SkISize& layerSize, const SkClipStack& existingClipStack,
                const SkRegion& existingClipRegion);

    // override from SkDevice
    virtual SkDevice* onCreateCompatibleDevice(SkBitmap::Config config,
                                               int width, int height,
                                               bool isOpaque,
                                               Usage usage) SK_OVERRIDE;

    void init();
    void cleanUp(bool clearFontUsage);
    void createFormXObjectFromDevice(SkRefPtr<SkPDFFormXObject>* xobject);

    // Clear the passed clip from all existing content entries.
    void clearClipFromContent(const SkClipStack* clipStack,
                              const SkRegion& clipRegion);
    void drawFormXObjectWithClip(SkPDFFormXObject* form,
                                 const SkClipStack* clipStack,
                                 const SkRegion& clipRegion,
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
                                    SkRefPtr<SkPDFFormXObject>* dst);
    void finishContentEntry(SkXfermode::Mode xfermode,
                            SkPDFFormXObject* dst);
    bool isContentEmpty();

    void populateGraphicStateEntryFromPaint(const SkMatrix& matrix,
                                            const SkClipStack& clipStack,
                                            const SkRegion& clipRegion,
                                            const SkPaint& paint,
                                            bool hasText,
                                            GraphicStateEntry* entry);
    int addGraphicStateResource(SkPDFGraphicState* gs);

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

    // Disable the default copy and assign implementation.
    SkPDFDevice(const SkPDFDevice&);
    void operator=(const SkPDFDevice&);
};

#endif
