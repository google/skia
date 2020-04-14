/*
* Copyright 2020 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/core/SkMarkerStack.h"

void SkMarkerStack::setMarker(uint32_t id, const SkM44& mx, void* boundary) {
    // Look if we've already seen id in this save-frame.
    // If so, replace, else append
    for (int i = fStack.size() - 1; i >= 0; --i) {
        auto& m = fStack[i];
        if (m.fBoundary != boundary) {   // we've gone past the save-frame
            break;                      // fall out so we append
        }
        if (m.fID == id) {    // in current frame, so replace
            m.fMatrix = mx;
            return;
        }
    }
    // if we get here, we should append a new marker
    fStack.push_back({boundary, mx, id});
}

bool SkMarkerStack::findMarker(uint32_t id, SkM44* mx) const {
    // search from top to bottom, so we find the most recent id
    for (int i = fStack.size() - 1; i >= 0; --i) {
        if (fStack[i].fID == id) {
            if (mx) {
                *mx = fStack[i].fMatrix;
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
