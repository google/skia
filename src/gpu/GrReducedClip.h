/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrReducedClip_DEFINED
#define GrReducedClip_DEFINED

#include "SkClipStack.h"
#include "SkTLList.h"

class GrContext;
class GrDrawContext;

/**
 * This class takes a clip stack and produces a reduced set of elements that are equivalent to
 * applying that full stack within a specified query rectangle.
 */
class SK_API GrReducedClip {
public:
    GrReducedClip(const SkClipStack& stack, const SkRect& queryBounds);

    /**
     * If hasIBounds() is true, this is the bounding box within which the reduced clip is valid, and
     * the caller must not modify any pixels outside this box. Undefined if hasIBounds() is false.
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

    typedef SkTLList<SkClipStack::Element, 16> ElementList;

    /**
     * Populated with a minimal list of elements that implement the clip.
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

    bool drawAlphaClipMask(GrDrawContext*) const;
    bool drawStencilClipMask(GrContext*, GrDrawContext*, const SkIPoint& clipOrigin) const;

private:
    void walkStack(const SkClipStack&, const SkRect& queryBounds);
    bool intersectIBounds(const SkIRect&);

    SkIRect        fIBounds;
    bool           fHasIBounds;
    ElementList    fElements;
    int32_t        fElementsGenID;
    bool           fRequiresAA;
    InitialState   fInitialState;
};

#endif
