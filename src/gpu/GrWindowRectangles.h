/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrWindowRectangles_DEFINED
#define GrWindowRectangles_DEFINED

#include "include/core/SkRect.h"
#include "include/private/SkTArray.h"

class GrWindowRectangles {
public:
    // This limit is now  totally irrelevant to GrWindowRectangle's impl.
    static constexpr int kMaxWindows = 8;

    GrWindowRectangles() {}

    GrWindowRectangles makeOffset(int dx, int dy) const {
        GrWindowRectangles result;
        for (const SkIRect& w : fWindows) {
            result.addWindow(w.makeOffset(dx,dy));
        }
        return result;
    }

    bool           empty() const { return fWindows.empty(); }
    int            count() const { return fWindows.count(); }
    const SkIRect* data () const { return fWindows.data (); }

    void reset() { fWindows.reset(); }

    SkIRect& addWindow(const SkIRect& window) { return (this->addWindow() = window); }
    SkIRect& addWindow() {
        SkASSERT(this->count() < kMaxWindows);  // Do we care?
        return fWindows.push_back();
    }

    bool operator==(const GrWindowRectangles& that) const { return fWindows == that.fWindows; }
    bool operator!=(const GrWindowRectangles& that) const { return fWindows != that.fWindows; }

private:
    SkSTArray<1, SkIRect> fWindows;
};

#endif
