/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTracker_DEFINED
#define SkTracker_DEFINED

#include <stdio.h>

#include "SkBitmap.h"
#include "SkPoint.h"

// TODO(edisonn): draw plan from point! - list of draw ops of a point, like a tree!
// TODO(edisonn): Minimal PDF to draw some points - remove everything that it is not needed,
//                save pdf uncompressed

#define MAX_TRACKING_POINTS 100

/** \class SkTracker
 *
 *   A Tracker can be attached to a SkTrackDevice and it will store the track pixels.
 *   It can be used with SampleApp to investigate bugs (CL not checked in yet).
 *
 *   The Tracker tracks 2 sets of points
 *     A) one which is expected to issue breackpoints if the pixels are changes
 *     B) one which if changes will disable the breackpoint
 *   For point in A) there are two modes:
 *     A.1) a breackpoint require that any of the points is changed
 *     A.2) a breackpoint require that all of the points is changed
 *   Points in B are allways in "any mode" - chaning the value of any pixel, will disable
 *     the breackpoint
 *
 *   Point in A) are used to show what are the areas of interest, while poit in B are used to
 *     disable breackpoints which would be issued in background change.
 *
 */
class SkTracker {
public:
    SkTracker() : fEnabled(false)
                , fBreakOnAny(false)
                , fCntExpectedTouched(0)
                , fCntExpectedUntouched(0)
                , fHits(0) {}

    virtual ~SkTracker() {}

    // Clears all the points, but preserves the break mode.
    void clearPoints() {
        fCntExpectedTouched = 0;
        fCntExpectedUntouched = 0;
    }

    // Enable the breackpoints.
    void enableTracking(bool b) {
        fEnabled = b;
    }

    // Returns true if breackpoints are enabled.
    bool trackingEnabled() {
        return fEnabled;
    }

    // Puts the tracker in Any mode.
    void any() {
        fBreakOnAny = true;
    }

    // Puts the tracker in Any mode.
    void all() {
        fBreakOnAny = false;
    }

    // returns true in in All mode. False for Any mode.
    bool requireAllExpectedTouched() {
        return !fBreakOnAny;
    }

    // Returns the numbers of points in which if touched, would trigger a breackpoint.
    int cntExpectedTouched() {
        return fCntExpectedTouched;
    }

    // Returns the points which if touched, would trigger a breackpoint.
    // the Tracker owns the array
    const SkIPoint* expectedTouched() {
        return fExpectedTouched;
    }

    // Returns the numbers of points in which if touched, would disable a breackpoint.
    int cntExpectedUntouched() {
        return fCntExpectedUntouched;
    }

    // Returns the points which if touched, would disable a breackpoint.
    // the Tracker owns the array
    const SkIPoint* expectedUntouched() {
        return fExpectedUntouched;
    }

    // Adds a point which if changes in a drawFoo operation, would trigger a breakpoint.
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

    // Adds a point which if changes in a drawFoo operation, would disable a breakpoint.
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

    // Starts a new rendering session - reset the number of hits.
    void newFrame() {
        fHits = 0;
    }

    // returns the number of breackpoints issues in this rendering session.
    int hits() {
        return fHits;
    }

    // Called before drawFoo to store the state of the pixels
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

    // Called after drawFoo to evaluate what pixels have changed, it could issue a breakpoint.
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
