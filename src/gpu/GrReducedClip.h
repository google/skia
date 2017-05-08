/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrReducedClip_DEFINED
#define GrReducedClip_DEFINED

#include "GrWindowRectangles.h"
#include "SkClipStack.h"
#include "SkTLList.h"

class GrContext;
class GrRenderTargetContext;

/**
 * This class takes a clip stack and produces a reduced set of elements that are equivalent to
 * applying that full stack within a specified query rectangle.
 */
class SK_API GrReducedClip {
public:
    GrReducedClip(const SkClipStack&, const SkRect& queryBounds, int maxWindowRectangles = 0);

    /**
     * If hasIBounds() is true, this is the bounding box within which the clip elements are valid.
     * The caller must not modify any pixels outside this box. Undefined if hasIBounds() is false.
     */
    const SkIRect& ibounds() const { SkASSERT(fHasIBounds); return fIBounds; }
    int left() const { return this->ibounds().left(); }
    int top() const { return this->ibounds().top(); }
    int width() const { return this->ibounds().width(); }
    int height() const { return this->ibounds().height(); }

    /**
     * Indicates whether ibounds() are defined. They will always be defined if the elements() are
     * nonempty.
     */
    bool hasIBounds() const { return fHasIBounds; }

    /**
     * If nonempty, this is a set of "exclusive" windows within which the clip elements are NOT
     * valid. The caller must not modify any pixels inside these windows.
     */
    const GrWindowRectangles& windowRectangles() const { return fWindowRects; }

    typedef SkTLList<SkClipStack::Element, 16> ElementList;

    /**
     * Populated with a minimal list of elements required to fully implement the clip.
     */
    const ElementList& elements() const { return fElements; }

    /**
     * If elements() are nonempty, uniquely identifies the list of elements within ibounds().
     * Otherwise undefined.
     */
    int32_t elementsGenID() const { SkASSERT(!fElements.isEmpty()); return fElementsGenID; }

    /**
     * Indicates whether antialiasing is required to process any of the clip elements.
     */
    bool requiresAA() const { return fRequiresAA; }

    enum class InitialState : bool {
        kAllIn,
        kAllOut
    };

    InitialState initialState() const { return fInitialState; }

    bool drawAlphaClipMask(GrRenderTargetContext*) const;
    bool drawStencilClipMask(GrContext*, GrRenderTargetContext*) const;

private:
    void walkStack(const SkClipStack&, const SkRect& queryBounds, int maxWindowRectangles);
    void addInteriorWindowRectangles(int maxWindowRectangles);
    void addWindowRectangle(const SkRect& elementInteriorRect, bool elementIsAA);
    bool intersectIBounds(const SkIRect&);

    SkIRect              fIBounds;
    bool                 fHasIBounds;
    GrWindowRectangles   fWindowRects;
    ElementList          fElements;
    int32_t              fElementsGenID;
    bool                 fRequiresAA;
    InitialState         fInitialState;
};

#endif
