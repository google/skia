/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrReducedClip_DEFINED
#define GrReducedClip_DEFINED

#include "SkClipStack.h"
#include "SkTLList.h"

class SK_API GrReducedClip {
public:
    typedef SkTLList<SkClipStack::Element, 16> ElementList;

    enum InitialState {
        kAllIn_InitialState,
        kAllOut_InitialState,
    };

    /**
     * This function produces a reduced set of SkClipStack::Elements that are equivalent to applying
     * a full clip stack within a specified query rectangle.
     *
     * @param stack          the clip stack to reduce.
     * @param queryBounds    bounding box of geometry the stack will clip.
     * @param result         populated with a minimal list of elements that implement the clip
     *                       within the provided query bounds.
     * @param resultGenID    uniquely identifies the resulting reduced clip.
     * @param clipIBounds    bounding box within which the reduced clip is valid. The caller must
     *                       not draw any pixels outside this box. NOTE: this box may be undefined
     *                       if no pixels are valid (e.g. empty result, "all out" initial state.)
     * @param requiresAA     indicates whether anti-aliasing is required to process any of the
     *                       elements in the element list result. Undefined if the result is empty.
     * @return the initial clip state within clipIBounds ("all in" or "all out").
     */
    static InitialState ReduceClipStack(const SkClipStack& stack,
                                        const SkRect& queryBounds,
                                        ElementList* result,
                                        int32_t* resultGenID,
                                        SkIRect* clipIBounds,
                                        bool* requiresAA);
};

#endif
