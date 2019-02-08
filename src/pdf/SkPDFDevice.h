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
#include "SkKeyedImage.h"
#include "SkPDFTypes.h"
#include "SkPaint.h"
#include "SkRect.h"
#include "SkRefCnt.h"
#include "SkStream.h"
#include "SkTHash.h"
#include "SkTextBlobPriv.h"

#include <vector>

class SkGlyphRunList;
class SkKeyedImage;
class SkPDFArray;
class SkPDFDevice;
class SkPDFDict;
class SkPDFDocument;
class SkPDFFont;
class SkPDFObject;
class SkPath;
class SkRRect;
struct SkPDFIndirectReference;

/**
 *  \class SkPDFDevice
 *
 *  An SkPDFDevice is the drawing context for a page or layer of PDF
 *  content.
 */
class SkPDFDevice final : public SkClipStackDevice {
public:
    /**
     *  @param pageSize Page size in point units.
     *         1 point == 127/360 mm == 1/72 inch
     *  @param document  A non-null pointer back to the
     *         PDFDocument object.  The document is responsible for
     *         de-duplicating across pages (via the SkPDFDocument) and
     *         for early serializing of large immutable objects, such
     *         as images (via SkPDFDocument::serialize()).
     *  @param initialTransform Transform to be applied to the entire page.
     */
    SkPDFDevice(SkISize pageSize, SkPDFDocument* document,
                const SkMatrix& initialTransform = SkMatrix::I());

    sk_sp<SkPDFDevice> makeCongruentDevice() {
        return sk_make_sp<SkPDFDevice>(this->size(), fDocument);
    }

    ~SkPDFDevice() override;

    /**
     *  These are called inside the per-device-layer loop for each draw call.
     *  When these are called, we have already applied any saveLayer
     *  operations, and are handling any looping from the paint.
     */
    void drawPaint(const SkPaint& paint) override;
    void drawPoints(SkCanvas::PointMode mode,
                    size_t count, const SkPoint[],
                    const SkPaint& paint) override;
    void drawRect(const SkRect& r, const SkPaint& paint) override;
    void drawOval(const SkRect& oval, const SkPaint& paint) override;
    void drawRRect(const SkRRect& rr, const SkPaint& paint) override;
    void drawPath(const SkPath& origpath, const SkPaint& paint, bool pathIsMutable) override;
    void drawBitmapRect(const SkBitmap& bitmap, const SkRect* src,
                        const SkRect& dst, const SkPaint&, SkCanvas::SrcRectConstraint) override;
    void drawSprite(const SkBitmap& bitmap, int x, int y,
                    const SkPaint& paint) override;

    void drawImageRect(const SkImage*,
                       const SkRect* src,
                       const SkRect& dst,
                       const SkPaint&,
                       SkCanvas::SrcRectConstraint) override;
    void drawGlyphRunList(const SkGlyphRunList& glyphRunList) override;
    void drawVertices(const SkVertices*, const SkVertices::Bone bones[], int boneCount, SkBlendMode,
                      const SkPaint&) override;
    void drawDevice(SkBaseDevice*, int x, int y,
                    const SkPaint&) override;

    // PDF specific methods.

    /** Create the resource dictionary for this device. Destructive. */
    std::unique_ptr<SkPDFDict> makeResourceDict();

    /** return annotations (link to urls and destinations) or nulltpr */
    std::unique_ptr<SkPDFArray> getAnnotations();

    /** Add our named destinations to the supplied dictionary.
     *  @param dict  Dictionary to add destinations to.
     *  @param page  The PDF object representing the page for this device.
     */
    void appendDestinations(SkPDFDict* dict, SkPDFIndirectReference page) const;

    /** Returns a SkStream with the page contents.
     */
    std::unique_ptr<SkStreamAsset> content();

    SkISize size() const { return this->imageInfo().dimensions(); }
    SkIRect bounds() const { return this->imageInfo().bounds(); }

    // It is important to not confuse GraphicStateEntry with SkPDFGraphicState, the
    // later being our representation of an object in the PDF file.
    struct GraphicStateEntry {
        SkMatrix fMatrix = SkMatrix::I();
        uint32_t fClipStackGenID = SkClipStack::kWideOpenGenID;
        SkColor4f fColor = {0, 0, 0, 1};
        SkScalar fTextScaleX = 1;  // Zero means we don't care what the value is.
        SkPaint::Style fTextFill = SkPaint::kFill_Style;  // Only if TextScaleX is non-zero.
        int fShaderIndex = -1;
        int fGraphicStateIndex = -1;
    };

    void DrawGlyphRunAsPath(SkPDFDevice* dev, const SkGlyphRun& glyphRun, SkPoint offset);

protected:
    sk_sp<SkSurface> makeSurface(const SkImageInfo&, const SkSurfaceProps&) override;

    void drawAnnotation(const SkRect&, const char key[], SkData* value) override;

    void drawSpecial(SkSpecialImage*, int x, int y, const SkPaint&,
                     SkImage*, const SkMatrix&) override;
    sk_sp<SkSpecialImage> makeSpecial(const SkBitmap&) override;
    sk_sp<SkSpecialImage> makeSpecial(const SkImage*) override;
    sk_sp<SkSpecialImage> snapSpecial() override;
    SkImageFilterCache* getImageFilterCache() override;

private:
    struct RectWithData {
        SkRect rect;
        sk_sp<SkData> data;
    };

    struct NamedDestination {
        sk_sp<SkData> nameData;
        SkPoint point;
    };

    // TODO(vandebo): push most of SkPDFDevice's state into a core object in
    // order to get the right access levels without using friend.
    friend class ScopedContentEntry;

    SkMatrix fInitialTransform;

    std::vector<RectWithData> fLinkToURLs;
    std::vector<RectWithData> fLinkToDestinations;
    std::vector<NamedDestination> fNamedDestinations;

    SkTHashSet<SkPDFIndirectReference> fGraphicStateResources;
    SkTHashSet<SkPDFIndirectReference> fXObjectResources;
    SkTHashSet<SkPDFIndirectReference> fShaderResources;
    SkTHashSet<SkPDFIndirectReference> fFontResources;
    int fNodeId;

    SkDynamicMemoryWStream fContent;
    SkDynamicMemoryWStream fContentBuffer;
    bool fNeedsExtraSave = false;
    struct GraphicStackState {
        GraphicStackState(SkDynamicMemoryWStream* s = nullptr);
        void updateClip(const SkClipStack* clipStack, const SkIRect& bounds);
        void updateMatrix(const SkMatrix& matrix);
        void updateDrawingState(const SkPDFDevice::GraphicStateEntry& state);
        void push();
        void pop();
        void drainStack();
        SkPDFDevice::GraphicStateEntry* currentEntry() { return &fEntries[fStackDepth]; }
        // Must use stack for matrix, and for clip, plus one for no matrix or clip.
        static constexpr int kMaxStackDepth = 2;
        SkPDFDevice::GraphicStateEntry fEntries[kMaxStackDepth + 1];
        int fStackDepth = 0;
        SkDynamicMemoryWStream* fContentStream;
    };
    GraphicStackState fActiveStackState;
    SkPDFDocument* fDocument;

    ////////////////////////////////////////////////////////////////////////////

    SkBaseDevice* onCreateDevice(const CreateInfo&, const SkPaint*) override;

    // Set alpha to true if making a transparency group form x-objects.
    SkPDFIndirectReference makeFormXObjectFromDevice(bool alpha = false);

    void drawFormXObjectWithMask(SkPDFIndirectReference xObject,
                                 SkPDFIndirectReference sMask,
                                 SkBlendMode,
                                 bool invertClip);

    // If the paint or clip is such that we shouldn't draw anything, this
    // returns nullptr and does not create a content entry.
    // setUpContentEntry and finishContentEntry can be used directly, but
    // the preferred method is to use the ScopedContentEntry helper class.
    SkDynamicMemoryWStream* setUpContentEntry(const SkClipStack* clipStack,
                                              const SkMatrix& matrix,
                                              const SkPaint& paint,
                                              SkScalar,
                                              SkPDFIndirectReference* dst);
    void finishContentEntry(const SkClipStack*, SkBlendMode, SkPDFIndirectReference, SkPath*);
    bool isContentEmpty();

    void internalDrawGlyphRun(const SkGlyphRun& glyphRun, SkPoint offset, const SkPaint& runPaint);
    void drawGlyphRunAsPath(const SkGlyphRun& glyphRun, SkPoint offset, const SkPaint& runPaint);

    void internalDrawImageRect(SkKeyedImage,
                               const SkRect* src,
                               const SkRect& dst,
                               const SkPaint&,
                               const SkMatrix& canvasTransformationMatrix);

    void internalDrawPath(const SkClipStack&,
                          const SkMatrix&,
                          const SkPath&,
                          const SkPaint&,
                          bool pathIsMutable);

    void internalDrawPathWithFilter(const SkClipStack& clipStack,
                                    const SkMatrix& ctm,
                                    const SkPath& origPath,
                                    const SkPaint& paint);

    bool handleInversePath(const SkPath& origPath, const SkPaint& paint, bool pathIsMutable);

    void addSMaskGraphicState(sk_sp<SkPDFDevice> maskDevice, SkDynamicMemoryWStream*);
    void clearMaskOnGraphicState(SkDynamicMemoryWStream*);
    void setGraphicState(SkPDFIndirectReference gs, SkDynamicMemoryWStream*);
    void drawFormXObject(SkPDFIndirectReference xObject, SkDynamicMemoryWStream*);

    bool hasEmptyClip() const { return this->cs().isEmpty(this->bounds()); }

    void reset();

    typedef SkClipStackDevice INHERITED;
};

#endif
