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

/**
 * This class takes a clip stack and produces a reduced set of elements that are equivalent to
 * applying that full stack within a specified query rectangle.
 */
class SK_API GrReducedClip {
public:
    GrReducedClip(const SkClipStack& stack, const SkRect& queryBounds);

    /**
     * Uniquely identifies this reduced clip.
     */
    int32_t genID() const { return fGenID; }

    /**
     * Bounding box within which the reduced clip is valid. The caller must not draw any pixels
     * outside this box.
     */
    const SkIRect& iBounds() const { return fIBounds; }
    int left() const { return this->iBounds().left(); }
    int top() const { return this->iBounds().top(); }
    int width() const { return this->iBounds().width(); }
    int height() const { return this->iBounds().height(); }

    typedef SkTLList<SkClipStack::Element, 16> ElementList;

    /**
     * Populated with a minimal list of elements that implement the clip.
     */
    const ElementList& elements() const { return fElements; }

    /**
     * Indicates whether antialiasing is required to process any of the clip elements.
     */
    bool requiresAA() const { return fRequiresAA; }

    enum class InitialState : bool {
        kAllIn,
        kAllOut
    };

    /**
     * The initial state of the clip within iBounds().
     */
    InitialState initialState() const { return fInitialState; }

private:
    int32_t        fGenID;
    SkIRect        fIBounds;
    ElementList    fElements;
    bool           fRequiresAA;
    InitialState   fInitialState;
};

#endif
