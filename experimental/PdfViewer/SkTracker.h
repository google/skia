/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTracker_DEFINED
#define SkTracker_DEFINED

#include "SkBitmap.h"
#include "SkPoint.h"

// TODO(edisonn): draw plan from point! - list of draw ops of a point, like a tree!
// TODO(edisonn): Minimal PDF to draw some points - remove everything that it is not needed,
//                save pdf uncompressed

#define MAX_TRACKING_POINTS 100

class SkTracker {
public:
    SkTracker() : fEnabled(false)
                , fBreakOnAny(false)
                , fCntExpectedTouched(0)
                , fCntExpectedUntouched(0)
                , fHits(0) {}

    virtual ~SkTracker() {}

    void clearPoints() {
        fCntExpectedTouched = 0;
        fCntExpectedUntouched = 0;
    }

    void enableTracking(bool b) {
        fEnabled = b;
    }

    bool trackingEnabled() {
        return fEnabled;
    }

    void any() {
        fBreakOnAny = true;
    }

    void all() {
        fBreakOnAny = false;
    }

    bool requireAllExpectedTouched() {
        return !fBreakOnAny;
    }

    int cntExpectedTouched() {
        return fCntExpectedTouched;
    }

    const SkIPoint* expectedTouched() {
        return fExpectedTouched;
    }

    int cntExpectedUntouched() {
        return fCntExpectedUntouched;
    }

    const SkIPoint* expectedUntouched() {
        return fExpectedUntouched;
    }

    bool addExpectTouch(int x, int y) {
        if (fCntExpectedTouched >= MAX_TRACKING_POINTS) {
            return false;
        }
        if (found(x, y)) {
            return false;
        }
        fExpectedTouched[fCntExpectedTouched] = SkIPoint::Make(x, y);
        fCntExpectedTouched++;
        return true;
    }

    bool addExpectUntouch(int x, int y) {
        if (fCntExpectedUntouched >= MAX_TRACKING_POINTS) {
            return false;
        }
        if (found(x, y)) {
            return false;
        }
        fExpectedUntouched[fCntExpectedUntouched] = SkIPoint::Make(x, y);
        fCntExpectedUntouched++;
        return true;
    }

    void newFrame() {
        fHits = 0;
    }

    int hits() {
        return fHits;
    }

    void before(const SkBitmap& bitmap) {
        if (fCntExpectedTouched == 0) {
            return;
        }

        for (int i = 0 ; i < fCntExpectedTouched; i++) {
            fBeforeTouched[i] = pickColor(bitmap, fExpectedTouched[i].x(), fExpectedTouched[i].y());
        }
        for (int i = 0 ; i < fCntExpectedUntouched; i++) {
            fBeforeUntouched[i] = pickColor(bitmap, fExpectedUntouched[i].x(),
                                            fExpectedUntouched[i].y());
        }
    }

    // any/all of the expected touched has to be changed, and all expected untouched must be intact
    void after(const SkBitmap& bitmap) {
        if (fCntExpectedTouched == 0) {
            return;
        }

        bool doBreak;
        if (fBreakOnAny) {
            doBreak = false;
            for (int i = 0 ; i < fCntExpectedTouched; i++) {
                doBreak = doBreak || fBeforeTouched[i] != pickColor(bitmap, fExpectedTouched[i].x(),
                                                                    fExpectedTouched[i].y());
            }
        } else {
            doBreak = true;
            for (int i = 0 ; i < fCntExpectedTouched; i++) {
                doBreak = doBreak && fBeforeTouched[i] != pickColor(bitmap, fExpectedTouched[i].x(),
                                                                    fExpectedTouched[i].y());
            }
        }

        for (int i = 0 ; i < fCntExpectedUntouched; i++) {
            doBreak = doBreak && fBeforeUntouched[i] == pickColor(bitmap, fExpectedUntouched[i].x(),
                                                                  fExpectedUntouched[i].y());
        }

        if (doBreak) {
            fHits++;
            if (fEnabled) {
                breakExecution();
            }
        }
    }

private:
    inline SkColor pickColor(const SkBitmap& bitmap, int x, int y) {
        return bitmap.getColor(x, y);
    }

    void breakExecution() {
        printf("break;\n");
    }

    inline bool found(int x, int y) {
        for (int i = 0 ; i < fCntExpectedTouched; i++) {
            if (x == fExpectedTouched[i].x() && y == fExpectedTouched[i].y()) {
                return true;
            }
        }
        for (int i = 0 ; i < fCntExpectedUntouched; i++) {
            if (x == fExpectedUntouched[i].x() && y == fExpectedUntouched[i].y()) {
                return true;
            }
        }
        return false;
    }


    bool fEnabled;
    // break on any change on expected touched or all.
    bool fBreakOnAny;
    SkIPoint fExpectedTouched[MAX_TRACKING_POINTS];
    SkColor fBeforeTouched[MAX_TRACKING_POINTS];
    int fCntExpectedTouched;

    SkIPoint fExpectedUntouched[MAX_TRACKING_POINTS];
    SkColor fBeforeUntouched[MAX_TRACKING_POINTS];
    int fCntExpectedUntouched;

    int fHits;
};

#endif  // SkTracker_DEFINED
