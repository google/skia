/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef Intersections_DEFINE
#define Intersections_DEFINE

#include <algorithm> // for std::min

class Intersections {
public:
    Intersections()
        : fUsed(0)
        , fUsed2(0)
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

    void addCoincident(double s1, double e1, double s2, double e2) {
        assert((fCoincidentUsed & 1) != 1);
        for (int index = 0; index < fCoincidentUsed; index += 2) {
            double cs1 = fCoincidentT[fSwap][index];
            double ce1 = fCoincidentT[fSwap][index + 1];
            bool s1in = approximately_between(cs1, s1, ce1);
            bool e1in = approximately_between(cs1, e1, ce1);
            double cs2 = fCoincidentT[fSwap ^ 1][index];
            double ce2 = fCoincidentT[fSwap ^ 1][index + 1];
            bool s2in = approximately_between(cs2, s2, ce2);
            bool e2in = approximately_between(cs2, e2, ce2);
            if ((s1in | e1in) & (s2in | e2in)) {
                double lesser1 = std::min(cs1, ce1);
                index += cs1 > ce1;
                if (s1in < lesser1) {
                    fCoincidentT[fSwap][index] = s1in;
                } else if (e1in < lesser1) {
                    fCoincidentT[fSwap][index] = e1in;
                }
                index ^= 1;
                double greater1 = fCoincidentT[fSwap][index];
                if (s1in > greater1) {
                    fCoincidentT[fSwap][index] = s1in;
                } else if (e1in > greater1) {
                    fCoincidentT[fSwap][index] = e1in;
                }
                index &= ~1;
                double lesser2 = std::min(cs2, ce2);
                index += cs2 > ce2;
                if (s2in < lesser2) {
                    fCoincidentT[fSwap ^ 1][index] = s2in;
                } else if (e2in < lesser2) {
                    fCoincidentT[fSwap ^ 1][index] = e2in;
                }
                index ^= 1;
                double greater2 = fCoincidentT[fSwap ^ 1][index];
                if (s2in > greater2) {
                    fCoincidentT[fSwap ^ 1][index] = s2in;
                } else if (e2in > greater2) {
                    fCoincidentT[fSwap ^ 1][index] = e2in;
                }
                return;
            }
        }
        assert(fCoincidentUsed < 9);
        fCoincidentT[fSwap][fCoincidentUsed] = s1;
        fCoincidentT[fSwap ^ 1][fCoincidentUsed] = s2;
        ++fCoincidentUsed;
        fCoincidentT[fSwap][fCoincidentUsed] = e1;
        fCoincidentT[fSwap ^ 1][fCoincidentUsed] = e2;
        ++fCoincidentUsed;
    }

    // FIXME: this is necessary because curve/curve intersections are noisy
    // remove once curve/curve intersections are improved
    void cleanUp();

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
    
    void insert(double one, double two) {
        assert(fUsed <= 1 || fT[0][0] < fT[0][1]);
        int index;
        for (index = 0; index < fUsed; ++index) {
            if (approximately_equal(fT[0][index], one)
                    && approximately_equal(fT[1][index], two)) {
                return;
            }
            if (fT[0][index] > one) {
                break;
            }
        }
        assert(fUsed < 9);
        int remaining = fUsed - index;
        if (remaining > 0) {
            memmove(&fT[0][index + 1], &fT[0][index], sizeof(fT[0][0]) * remaining);
            memmove(&fT[1][index + 1], &fT[1][index], sizeof(fT[1][0]) * remaining);
        }
        fT[0][index] = one;
        fT[1][index] = two;
        ++fUsed;
    }
    
    void insertOne(double t, int side) {
        int used = side ? fUsed2 : fUsed;
        assert(used <= 1 || fT[side][0] < fT[side][1]);
        int index;
        for (index = 0; index < used; ++index) {
            if (approximately_equal(fT[side][index], t)) {
                return;
            }
            if (fT[side][index] > t) {
                break;
            }
        }
        assert(used < 9);
        int remaining = used - index;
        if (remaining > 0) {
            memmove(&fT[side][index + 1], &fT[side][index], sizeof(fT[side][0]) * remaining);
        }
        fT[side][index] = t;
        side ? ++fUsed2 : ++fUsed;
    }

    bool intersected() const {
        return fUsed > 0;
    }
    
    bool insertBalanced() const {
        return fUsed == fUsed2;
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
    int fUsed2;
    int fCoincidentUsed;
private:
    int fSwap;
};

#endif

