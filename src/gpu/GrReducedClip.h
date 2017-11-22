/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrReducedClip_DEFINED
#define GrReducedClip_DEFINED

#include "GrFragmentProcessor.h"
#include "GrWindowRectangles.h"
#include "SkClipStack.h"
#include "SkTLList.h"

class GrAppliedClip;
class GrContext;
class GrRenderTargetContext;

/**
 * This class takes a clip stack and produces a reduced set of elements that are equivalent to
 * applying that full stack within a specified query rectangle.
 */
class SK_API GrReducedClip {
public:
    using Element = SkClipStack::Element;
    using ElementList = SkTLList<SkClipStack::Element, 16>;

    GrReducedClip(const SkClipStack& stack, const SkRect& queryBounds, GrAppliedClip* out = nullptr)
            : GrReducedClip(stack, queryBounds, 0, 0, out) {}

    GrReducedClip(const SkClipStack&, const SkRect& queryBounds, int maxWindowRectangles,
                  int maxAnalyticFPs, GrAppliedClip* out = nullptr);

   /**
     * Indicates whether ibounds() is defined. It will always be defined if the maskElements() are
     * nonempty.
     */
    bool hasIBounds() const { return fHasIBounds; }

    /**
     * If hasIBonds() is true, this is the rect within which the caller must create and apply the
     * clip mask. It will also be the GrAppliedClip's scissor if the queryBounds are not completely
     * contained within.
     */
    const SkIRect& ibounds() const { SkASSERT(fHasIBounds); return fIBounds; }
    int left() const { return this->ibounds().left(); }
    int top() const { return this->ibounds().top(); }
    int width() const { return this->ibounds().width(); }
    int height() const { return this->ibounds().height(); }

    /**
     * Returns the number of analytic clip fragment processors that were added to the GrAppliedClip.
     */
    int numAnalyticFPs() const { return fNumAnalyticFPs; }

    /**
     * An ordered list of clip elements that could not be skipped or implemented by other means. If
     * nonempty, the caller is responsible to create an alpha and/or stencil mask for these elements
     * and add it to the GrAppliedClip.
     */
    const ElementList& maskElements() const { return fMaskElements; }

    /**
     * If maskElements() are nonempty, uniquely identifies the region of the clip mask that falls
     * inside of ibounds().
     * NOTE: since clip elements might fall outside the query bounds, different regions of the same
     * clip stack might have more or less restrictive IDs.
     * FIXME: this prevents us from reusing a sub-rect of a perfectly good mask when that rect has
     * been assigned a less restrictive ID.
     */
    uint32_t maskGenID() const { SkASSERT(!fMaskElements.isEmpty()); return fMaskGenID; }

    /**
     * Indicates whether antialiasing is required to process any of the mask elements.
     */
    bool maskRequiresAA() const { SkASSERT(!fMaskElements.isEmpty()); return fMaskRequiresAA; }

    enum class InitialState : bool {
        kAllIn,
        kAllOut
    };

    InitialState initialState() const { return fInitialState; }

    bool drawAlphaClipMask(GrRenderTargetContext*) const;
    bool drawStencilClipMask(GrContext*, GrRenderTargetContext*) const;

private:
    void init(const SkClipStack&, const SkRect& queryBounds);
    void walkStack(const SkClipStack&, const SkRect& queryBounds);

    enum class ClipResult {
        kNotClipped,
        kClipped,
        kMadeEmpty
    };

    // Clips the the given element's interior out of the final clip.
    // NOTE: do not call for elements followed by ops that can grow the clip.
    ClipResult clipInsideElement(const Element*);

    // Clips the the given element's exterior out of the final clip.
    // NOTE: do not call for elements followed by ops that can grow the clip.
    ClipResult clipOutsideElement(const Element*);

    void addWindowRectangle(const SkRect& elementInteriorRect, bool elementIsAA);

    enum class Invert : bool {
        kNo,
        kYes
    };

    template<typename T> ClipResult addAnalyticFP(const T& deviceSpaceShape, Invert, bool aa);

    void makeEmpty();

    GrAppliedClip* const fAppliedClip;
    const int fMaxWindowRectangles;
    const int fMaxAnalyticFPs;
    SkIRect fIBounds;
    bool fHasIBounds;
    SkRect fAAClipRect;
    uint32_t fAAClipRectGenID; // GenID the mask will have if includes the AA clip rect.
    GrWindowRectangles fWindowRects;
    int fNumAnalyticFPs;
    ElementList fMaskElements;
    uint32_t fMaskGenID;
    bool fMaskRequiresAA;
    InitialState fInitialState;
};

#endif
