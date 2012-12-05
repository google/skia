
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

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
 * SkClipStack::Elements that are equivalent to applying the full stack to the rectangle. The
 * initial state of the query rectangle before the first clip element is applied is returned via
 * initialState. This function is declared here so that it can be unit-tested. It may become a
 * member function of SkClipStack when its interface is determined to be stable.
 */
void GrReduceClipStack(const SkClipStack& stack,
                       const SkRect& queryBounds,
                       ElementList* result,
                       InitialState* initialState);

} // namespace GrReducedClip
