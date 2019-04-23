/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrReducedClip_DEFINED
#define GrReducedClip_DEFINED

#include "src/core/SkClipStack.h"
#include "src/core/SkTLList.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrWindowRectangles.h"

class GrCoverageCountingPathRenderer;
class GrRecordingContext;
class GrRenderTargetContext;

/**
 * This class takes a clip stack and produces a reduced set of elements that are equivalent to
 * applying that full stack within a specified query rectangle.
 */
class SK_API GrReducedClip {
public:
    using Element = SkClipStack::Element;
    using ElementList = SkTLList<SkClipStack::Element, 16>;

    GrReducedClip(const SkClipStack&, const SkRect& queryBounds, const GrCaps* caps,
                  int maxWindowRectangles = 0, int maxAnalyticFPs = 0, int maxCCPRClipPaths = 0);

    enum class InitialState : bool {
        kAllIn,
        kAllOut
    };

    InitialState initialState() const { return fInitialState; }

    /**
     * If hasScissor() is true, the clip mask is not valid outside this rect and the caller must
     * enforce this scissor during draw.
     */
    const SkIRect& scissor() const { SkASSERT(fHasScissor); return fScissor; }
    int left() const { return this->scissor().left(); }
    int top() const { return this->scissor().top(); }
    int width() const { return this->scissor().width(); }
    int height() const { return this->scissor().height(); }

    /**
     * Indicates whether scissor() is defined. It will always be defined if the maskElements() are
     * nonempty.
     */
    bool hasScissor() const { return fHasScissor; }

    /**
     * If nonempty, the clip mask is not valid inside these windows and the caller must clip them
     * out using the window rectangles GPU extension.
     */
    const GrWindowRectangles& windowRectangles() const { return fWindowRects; }

    /**
     * An ordered list of clip elements that could not be skipped or implemented by other means. If
     * nonempty, the caller must create an alpha and/or stencil mask for these elements and apply it
     * during draw.
     */
    const ElementList& maskElements() const { return fMaskElements; }

    /**
     * If maskElements() are nonempty, uniquely identifies the region of the clip mask that falls
     * inside of scissor().
     *
     * NOTE: since clip elements might fall outside the query bounds, different regions of the same
     * clip stack might have more or less restrictive IDs.
     *
     * FIXME: this prevents us from reusing a sub-rect of a perfectly good mask when that rect has
     * been assigned a less restrictive ID.
     */
    uint32_t maskGenID() const { SkASSERT(!fMaskElements.isEmpty()); return fMaskGenID; }

    /**
     * Indicates whether antialiasing is required to process any of the mask elements.
     */
    bool maskRequiresAA() const { SkASSERT(!fMaskElements.isEmpty()); return fMaskRequiresAA; }

    bool drawAlphaClipMask(GrRenderTargetContext*) const;
    bool drawStencilClipMask(GrRecordingContext*, GrRenderTargetContext*) const;

    int numAnalyticFPs() const { return fAnalyticFPs.count() + fCCPRClipPaths.count(); }

    /**
     * Called once the client knows the ID of the opList that the clip FPs will operate in. This
     * method finishes any outstanding work that was waiting for the opList ID, then detaches and
     * returns this class's list of FPs that complete the clip.
     *
     * NOTE: this must be called AFTER producing the clip mask (if any) because draw calls on
     * the render target context, surface allocations, and even switching render targets (pre MDB)
     * may cause flushes or otherwise change which opList the actual draw is going into.
     */
    std::unique_ptr<GrFragmentProcessor> finishAndDetachAnalyticFPs(GrCoverageCountingPathRenderer*,
                                                                    uint32_t opListID, int rtWidth,
                                                                    int rtHeight);

private:
    void walkStack(const SkClipStack&, const SkRect& queryBounds);

    enum class ClipResult {
        kNotClipped,
        kClipped,
        kMadeEmpty
    };

    // Intersects the clip with the element's interior, regardless of inverse fill type.
    // NOTE: do not call for elements followed by ops that can grow the clip.
    ClipResult clipInsideElement(const Element*);

    // Intersects the clip with the element's exterior, regardless of inverse fill type.
    // NOTE: do not call for elements followed by ops that can grow the clip.
    ClipResult clipOutsideElement(const Element*);

    void addWindowRectangle(const SkRect& elementInteriorRect, bool elementIsAA);

    enum class Invert : bool {
        kNo = false,
        kYes = true
    };

    static GrClipEdgeType GetClipEdgeType(Invert, GrAA);
    ClipResult addAnalyticFP(const SkRect& deviceSpaceRect, Invert, GrAA);
    ClipResult addAnalyticFP(const SkRRect& deviceSpaceRRect, Invert, GrAA);
    ClipResult addAnalyticFP(const SkPath& deviceSpacePath, Invert, GrAA);

    void makeEmpty();

    const GrCaps* fCaps;
    const int fMaxWindowRectangles;
    const int fMaxAnalyticFPs;
    const int fMaxCCPRClipPaths;

    InitialState fInitialState;
    SkIRect fScissor;
    bool fHasScissor;
    SkRect fAAClipRect;
    uint32_t fAAClipRectGenID; // GenID the mask will have if includes the AA clip rect.
    GrWindowRectangles fWindowRects;
    ElementList fMaskElements;
    uint32_t fMaskGenID;
    bool fMaskRequiresAA;
    SkSTArray<4, std::unique_ptr<GrFragmentProcessor>> fAnalyticFPs;
    SkSTArray<4, SkPath> fCCPRClipPaths; // Will convert to FPs once we have an opList ID for CCPR.
};

#endif
