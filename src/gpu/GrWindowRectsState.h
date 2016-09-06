/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrWindowRectsState_DEFINED
#define GrWindowRectsState_DEFINED

#include "GrWindowRectangles.h"

class GrWindowRectsState {
public:
    enum class Mode : bool {
        kExclusive,
        kInclusive
    };

    GrWindowRectsState() : fMode(Mode::kExclusive) {}
    GrWindowRectsState(const GrWindowRectangles& windows, const SkIPoint& origin, Mode mode)
        : fMode(mode)
        , fOrigin(origin)
        , fWindows(windows) {
    }

    bool enabled() const { return Mode::kInclusive == fMode || !fWindows.empty(); }
    Mode mode() const { return fMode; }
    const SkIPoint& origin() const { return fOrigin; }
    const GrWindowRectangles& windows() const { return fWindows; }
    int numWindows() const { return fWindows.count(); }

    void setDisabled() {
        fMode = Mode::kExclusive;
        fWindows.reset();
    }

    void set(const GrWindowRectangles& windows, const SkIPoint& origin, Mode mode) {
        fMode = mode;
        fOrigin = origin;
        fWindows = windows;
    }

    bool cheapEqualTo(const GrWindowRectsState& that) const {
        if (fMode != that.fMode) {
            return false;
        }
        if (!fWindows.empty() && fOrigin != that.fOrigin) {
            return false;
        }
        return fWindows == that.fWindows;
    }

private:
    Mode                 fMode;
    SkIPoint             fOrigin;
    GrWindowRectangles   fWindows;
};

#endif
