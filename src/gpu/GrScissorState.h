/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrScissorState_DEFINED
#define GrScissorState_DEFINED

#include "include/core/SkRect.h"

class GrScissorState {
public:
    GrScissorState() : fEnabled(false) {}
    GrScissorState(const SkIRect& rect) : fEnabled(true), fRect(rect) {}
    void setDisabled() { fEnabled = false; }
    void set(const SkIRect& rect) { fRect = rect; fEnabled = true; }
    bool SK_WARN_UNUSED_RESULT intersect(const SkIRect& rect) {
        if (!fEnabled) {
            this->set(rect);
            return true;
        }
        return fRect.intersect(rect);
    }
    bool operator==(const GrScissorState& other) const {
        return fEnabled == other.fEnabled &&
                (false == fEnabled || fRect == other.fRect);
    }
    bool operator!=(const GrScissorState& other) const { return !(*this == other); }

    bool enabled() const { return fEnabled; }
    const SkIRect& rect() const { return fRect; }

private:
    bool    fEnabled;
    SkIRect fRect;
};

#endif
