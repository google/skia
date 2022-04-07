/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrWindowRectsState_DEFINED
#define GrWindowRectsState_DEFINED

#include "src/gpu/ganesh/GrWindowRectangles.h"

class GrWindowRectsState {
public:
    enum class Mode : bool {
        kExclusive,
        kInclusive
    };

    GrWindowRectsState() : fMode(Mode::kExclusive) {}
    GrWindowRectsState(const GrWindowRectangles& windows, Mode mode)
        : fMode(mode)
        , fWindows(windows) {
    }

    bool enabled() const { return Mode::kInclusive == fMode || !fWindows.empty(); }
    Mode mode() const { return fMode; }
    const GrWindowRectangles& windows() const { return fWindows; }
    int numWindows() const { return fWindows.count(); }

    void setDisabled() {
        fMode = Mode::kExclusive;
        fWindows.reset();
    }

    void set(const GrWindowRectangles& windows, Mode mode) {
        fMode = mode;
        fWindows = windows;
    }

    bool operator==(const GrWindowRectsState& that) const {
        if (fMode != that.fMode) {
            return false;
        }
        return fWindows == that.fWindows;
    }
    bool operator!=(const GrWindowRectsState& that) const { return !(*this == that); }

private:
    Mode                 fMode;
    GrWindowRectangles   fWindows;
};

#endif
