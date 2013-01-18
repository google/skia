/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef Intersections_DEFINE
#define Intersections_DEFINE

#include <algorithm> // for std::min -- Skia doesn't have a SkMinDouble
#include "SkTypes.h"

class Intersections {
public:
    Intersections()
        : fUsed(0)
        , fUsed2(0)
        , fCoincidentUsed(0)
        , fFlip(false)
        , fUnsortable(false)
        , fSwap(0)
    {
        // OPTIMIZE: don't need to be initialized in release
        bzero(fT, sizeof(fT));
        bzero(fCoincidentT, sizeof(fCoincidentT));
    }

    void add(double one, double two) {
        for (int index = 0; index < fUsed; ++index) {
            if (approximately_equal(fT[fSwap][index], one)
                    && approximately_equal(fT[fSwap ^ 1][index], two)) {
                return;
            }
        }
        assert(fUsed < 9);
        fT[fSwap][fUsed] = one;
        fT[fSwap ^ 1][fUsed] = two;
        ++fUsed;
    }

    // start if index == 0 : end if index == 1
    void addCoincident(double one, double two) {
        for (int index = 0; index < fCoincidentUsed; ++index) {
            if (approximately_equal(fCoincidentT[fSwap][index], one)
                    && approximately_equal(fCoincidentT[fSwap ^ 1][index], two)) {
                return;
            }
        }
        assert(fCoincidentUsed < 9);
        fCoincidentT[fSwap][fCoincidentUsed] = one;
        fCoincidentT[fSwap ^ 1][fCoincidentUsed] = two;
        ++fCoincidentUsed;
    }

    void addCoincident(double s1, double e1, double s2, double e2);

    // FIXME: this is necessary because curve/curve intersections are noisy
    // remove once curve/curve intersections are improved
    void cleanUp();

    int coincidentUsed() const{
        return fCoincidentUsed;
    }

    void offset(int base, double start, double end) {
        for (int index = base; index < fUsed; ++index) {
            double val = fT[fSwap][index];
            val *= end - start;
            val += start;
            fT[fSwap][index] = val;
        }
    }

    void insert(double one, double two);
    void insertOne(double t, int side);

    bool intersected() const {
        return fUsed > 0;
    }

    bool insertBalanced() const {
        return fUsed == fUsed2;
    }

    void swap() {
        fSwap ^= true;
    }

    void swapPts() {
        int index;
        for (index = 0; index < fUsed; ++index) {
            SkTSwap(fT[0][index], fT[1][index]);
        }
    }

    bool swapped() const {
        return fSwap;
    }

    bool unsortable() const {
        return fUnsortable;
    }

    int used() const {
        return fUsed;
    }

    double fT[2][9];
    double fCoincidentT[2][9];
    int fUsed;
    int fUsed2;
    int fCoincidentUsed;
    bool fFlip;
    bool fUnsortable;
private:
    int fSwap;
};

#endif
