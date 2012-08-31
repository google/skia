/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef Intersections_DEFINE
#define Intersections_DEFINE

class Intersections {
public:
    Intersections()
        : fUsed(0)
        , fCoincidentUsed(0)
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
        fT[fSwap][fUsed] = one;
        fT[fSwap ^ 1][fUsed] = two;
        ++fUsed;
    }

    // start if index == 0 : end if index == 1
    void addCoincident(double one, double two, bool cancel) {
        for (int index = 0; index < fCoincidentUsed; ++index) {
            if (approximately_equal(fCoincidentT[fSwap][index], one)
                    && approximately_equal(fCoincidentT[fSwap ^ 1][index], two)) {
                if (cancel) {
                    --fCoincidentUsed;
                }
                return;
            }
        }
        fCoincidentT[fSwap][fCoincidentUsed] = one;
        fCoincidentT[fSwap ^ 1][fCoincidentUsed] = two;
        ++fCoincidentUsed;
    }

    int coincidentUsed() {
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

    bool intersected() {
        return fUsed > 0;
    }

    void swap() {
        fSwap ^= 1;
    }

    bool swapped() {
        return fSwap;
    }

    int used() {
        return fUsed;
    }

    double fT[2][9];
    double fCoincidentT[2][9];
    int fUsed;
    int fCoincidentUsed;
private:
    int fSwap;
};

#endif

