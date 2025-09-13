/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPDFDevice_DEFINED
#define SkPDFDevice_DEFINED

#include "include/core/SkCPURecorder.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "src/core/SkClipStack.h"
#include "src/core/SkClipStackDevice.h"
#include "src/core/SkTHash.h"
#include "src/pdf/SkKeyedImage.h"
#include "src/pdf/SkPDFGraphicStackState.h"
#include "src/pdf/SkPDFTag.h"
#include "src/pdf/SkPDFTypes.h"

#include <memory>

class SkBitmap;
class SkBlender;
class SkData;
class SkDevice;
class SkImage;
class SkMesh;
class SkPaint;
class SkPath;
class SkPDFDocument;
class SkRecorder;
class SkRRect;
class SkSpecialImage;
class SkSurface;
class SkSurfaceProps;
class SkVertices;
enum class SkBlendMode;
struct SkIRect;
struct SkISize;
struct SkImageInfo;
struct SkPoint;
struct SkRect;

namespace sktext {
class GlyphRun;
class GlyphRunList;
}

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
    void drawPoints(SkCanvas::PointMode, SkSpan<const SkPoint>, const SkPaint&) override;
    void drawRect(const SkRect& r, const SkPaint& paint) override;
    void drawOval(const SkRect& oval, const SkPaint& paint) override;
    void drawRRect(const SkRRect& rr, const SkPaint& paint) override;
    void drawPath(const SkPath& origpath, const SkPaint& paint, bool pathIsMutable) override;

    void drawImageRect(const SkImage*,
                       const SkRect* src,
                       const SkRect& dst,
                       const SkSamplingOptions&,
                       const SkPaint&,
                       SkCanvas::SrcRectConstraint) override;

    void drawVertices(const SkVertices*, sk_sp<SkBlender>, const SkPaint&, bool) override;
    void drawMesh(const SkMesh&, sk_sp<SkBlender>, const SkPaint&) override;

    void drawAnnotation(const SkRect&, const char key[], SkData* value) override;

    void drawDevice(SkDevice*, const SkSamplingOptions&, const SkPaint&) override;
    void drawSpecial(SkSpecialImage*, const SkMatrix&, const SkSamplingOptions&,
                     const SkPaint&, SkCanvas::SrcRectConstraint) override;

    sk_sp<SkSurface> makeSurface(const SkImageInfo&, const SkSurfaceProps&) override;
    sk_sp<SkDevice> createDevice(const CreateInfo&, const SkPaint*) override;

    // PDF specific methods.
    void drawSprite(const SkBitmap& bitmap, int x, int y,
                    const SkPaint& paint);

    /** Create the resource dictionary for this device. Destructive. */
    std::unique_ptr<SkPDFDict> makeResourceDict();

    /** Returns a SkStream with the page contents.
     */
    std::unique_ptr<SkStreamAsset> content();

    const SkMatrix& initialTransform() const { return fInitialTransform; }

    SkRecorder* baseRecorder() const override {
        // TODO(kjlubick) the creation of this should likely involve a CPU context.
        return skcpu::Recorder::TODO();
    }

private:
    // TODO(vandebo): push most of SkPDFDevice's state into a core object in
    // order to get the right access levels without using friend.
    friend class ScopedContentEntry;

    SkMatrix fInitialTransform;

    skia_private::THashSet<SkPDFIndirectReference> fGraphicStateResources;
    skia_private::THashSet<SkPDFIndirectReference> fXObjectResources;
    skia_private::THashSet<SkPDFIndirectReference> fShaderResources;
    skia_private::THashSet<SkPDFIndirectReference> fFontResources;

    class MarkedContentManager {
    public:
        MarkedContentManager(SkPDFDocument* document, SkDynamicMemoryWStream* out);
        ~MarkedContentManager();

        // Sets the current element identifier. Associate future draws with the structure element
        // with the given element identifier. Element identifier 0 is reserved to mean no structure
        // element.
        void setNextMarksElemId(int nextMarksElemId);

        // The current element identifier.
        int elemId() const;

        // Starts a marked-content sequence for a content item for the structure element with the
        // current element identifier. If there is an active marked-content sequence associated with
        // a different element identifier the active marked-content sequence will first be closed.
        // If there is no structure element with the current element identifier then the
        // marked-content sequence will not be started.
        void beginMark();

        // Tests if there is an active marked-content sequence.
        bool hasActiveMark() const;

        // Accumulates an upper left location for the active mark. The point is in PDF page space
        // and so is y-up. Only use if this.hasActiveMark()
        void accumulate(const SkPoint& p);

        // Tests if this marked content manager made any marks.
        bool madeMarks() const { return fMadeMarks; }

    private:
        SkPDFDocument* fDoc;
        SkDynamicMemoryWStream* fOut;
        SkPDFStructTree::Mark fCurrentlyActiveMark;
        int fNextMarksElemId;
        int fCurrentMarksElemId;
        bool fMadeMarks;
    } fMarkManager;

    SkDynamicMemoryWStream fContent;
    SkDynamicMemoryWStream fContentBuffer;
    bool fNeedsExtraSave = false;
    SkPDFGraphicStackState fActiveStackState;
    SkPDFDocument* fDocument;

    ////////////////////////////////////////////////////////////////////////////

    void onDrawGlyphRunList(SkCanvas*, const sktext::GlyphRunList&, const SkPaint& paint) override;

    // Set alpha to true if making a transparency group form x-objects.
    SkPDFIndirectReference makeFormXObjectFromDevice(bool alpha = false);
    SkPDFIndirectReference makeFormXObjectFromDevice(SkIRect bbox, bool alpha = false);

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

    void internalDrawGlyphRun(
            const sktext::GlyphRun& glyphRun, SkPoint offset, const SkPaint& runPaint);
    void drawGlyphRunAsPath(
            const sktext::GlyphRun& glyphRun, SkPoint offset, const SkPaint& runPaint);

    void internalDrawImageRect(SkKeyedImage,
                               const SkRect* src,
                               const SkRect& dst,
                               const SkSamplingOptions&,
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

    void clearMaskOnGraphicState(SkDynamicMemoryWStream*);
    void setGraphicState(SkPDFIndirectReference gs, SkDynamicMemoryWStream*);
    void drawFormXObject(SkPDFIndirectReference xObject, SkDynamicMemoryWStream*, SkPath* shape);

    bool hasEmptyClip() const { return this->cs().isEmpty(this->bounds()); }

    void reset();
};

#endif
