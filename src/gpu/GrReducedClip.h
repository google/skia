
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

namespace GrReducedClip {

typedef SkTLList<SkClipStack::Element> ElementList;

enum InitialState {
    kAllIn_InitialState,
    kAllOut_InitialState,
};

/**
 * This function takes a clip stack and a query rectangle and it produces a reduced set of
 * SkClipStack::Elements that are equivalent to applying the full stack to the rectangle. The clip
 * stack generation id that represents the list of elements is returned in resultGenID. The
 * initial state of the query rectangle before the first clip element is applied is returned via
 * initialState. Optionally, the caller can request a tighter bounds on the clip be returned via
 * tighterBounds. If not NULL, tighterBounds will always be contained by queryBounds after return.
 * If tighterBounds is specified then it is assumed that the caller will implicitly clip against it.
 * If the caller specifies non-NULL for requiresAA then it will indicate whether anti-aliasing is
 * required to process any of the elements in the result.
 *
 * This may become a member function of SkClipStack when its interface is determined to be stable.
 * Marked SK_API so that SkLua can call this in a shared library build.
 */
SK_API void ReduceClipStack(const SkClipStack& stack,
                            const SkIRect& queryBounds,
                            ElementList* result,
                            int32_t* resultGenID,
                            InitialState* initialState,
                            SkIRect* tighterBounds = NULL,
                            bool* requiresAA = NULL);

} // namespace GrReducedClip

#endif
