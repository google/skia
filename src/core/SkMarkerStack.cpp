/*
* Copyright 2020 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/core/SkMarkerStack.h"

void SkMarkerStack::setMarker(uint32_t id, const SkM44& mx, void* boundary) {
    // We compute and cache the inverse here. Most clients are only interested in that, and we'll
    // be fetching matrices from this far more often than we insert them.
    SkM44 inv;
    SkAssertResult(mx.invert(&inv));

    // Look if we've already seen id in this save-frame.
    // If so, replace, else append
    for (auto it = fStack.rbegin(); it != fStack.rend(); ++it) {
        if (it->fBoundary != boundary) {   // we've gone past the save-frame
            break;                         // fall out so we append
        }
        if (it->fID == id) {    // in current frame, so replace
            it->fMatrix = mx;
            it->fMatrixInverse = inv;
            return;
        }
    }
    // if we get here, we should append a new marker
    fStack.push_back({boundary, mx, inv, id});
}

bool SkMarkerStack::findMarker(uint32_t id, SkM44* mx) const {
    // search from top to bottom, so we find the most recent id
    for (auto it = fStack.rbegin(); it != fStack.rend(); ++it) {
        if (it->fID == id) {
            if (mx) {
                *mx = it->fMatrix;
            }
            return true;
        }
    }
    return false;
}

bool SkMarkerStack::findMarkerInverse(uint32_t id, SkM44* mx) const {
    // search from top to bottom, so we find the most recent id
    for (auto it = fStack.rbegin(); it != fStack.rend(); ++it) {
        if (it->fID == id) {
            if (mx) {
                *mx = it->fMatrixInverse;
            }
            return true;
        }
    }
    return false;
}

void SkMarkerStack::restore(void* boundary) {
    while (!fStack.empty() && fStack.back().fBoundary == boundary) {
        fStack.pop_back();
    }
}
