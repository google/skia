/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "DataTypes.h"
#include "Intersections.h"

void Intersections::insertCoincidentPair(double s1, double e1, double s2, double e2,
        const _Point& startPt, const _Point& endPt) {
    if (fSwap) {
        remove(s2, e2, startPt, endPt);
    } else {
        remove(s1, e1, startPt, endPt);
    }
    SkASSERT(coincidentUsed() == fUsed);
    SkASSERT((coincidentUsed() & 1) != 1);
    int i1 = 0;
    int i2 = 0;
    do {
        while (i1 < fUsed && !(fIsCoincident[fSwap] & (1 << i1))) {
            ++i1;
        }
        if (i1 == fUsed) {
            break;
        }
        SkASSERT(i1 < fUsed);
        int iEnd1 = i1 + 1;
        while (!(fIsCoincident[fSwap] & (1 << iEnd1))) {
            ++iEnd1;
        }
        SkASSERT(iEnd1 < fUsed);
        double cs1 = fT[fSwap][i1];
        double ce1 = fT[fSwap][iEnd1];
        bool s1in = between(cs1, s1, ce1) || startPt.approximatelyEqual(fPt[i1])
                || startPt.approximatelyEqual(fPt[iEnd1]);
        bool e1in = between(cs1, e1, ce1) || endPt.approximatelyEqual(fPt[i1])
                || endPt.approximatelyEqual(fPt[iEnd1]);
        while (i2 < fUsed && !(fIsCoincident[fSwap ^ 1] & (1 << i2))) {
            ++i2;
        }
        int iEnd2 = i2 + 1;
        while (!(fIsCoincident[fSwap ^ 1] & (1 << iEnd2))) {
            ++iEnd2;
        }
        SkASSERT(iEnd2 < fUsed);
        double cs2 = fT[fSwap ^ 1][i2];
        double ce2 = fT[fSwap ^ 1][iEnd2];
        bool s2in = between(cs2, s2, ce2) || startPt.approximatelyEqual(fPt[i2])
                || startPt.approximatelyEqual(fPt[iEnd2]);
        bool e2in = between(cs2, e2, ce2) || endPt.approximatelyEqual(fPt[i2])
                || endPt.approximatelyEqual(fPt[iEnd2]);
        if ((s1in | e1in) & (s2in | e2in)) {
            if (s1 < cs1) {
                fT[fSwap][i1] = s1;
                fPt[i1] = startPt;
            } else if (e1 < cs1) {
                fT[fSwap][i1] = e1;
                fPt[i1] = endPt;
            }
            if (s1 > ce1) {
                fT[fSwap][iEnd1] = s1;
                fPt[iEnd1] = startPt;
            } else if (e1 > ce1) {
                fT[fSwap][iEnd1] = e1;
                fPt[iEnd1] = endPt;
            }
            if (s2 > e2) {
                SkTSwap(cs2, ce2);
                SkTSwap(i2, iEnd2);
            }
            if (s2 < cs2) {
                fT[fSwap ^ 1][i2] = s2;
            } else if (e2 < cs2) {
                fT[fSwap ^ 1][i2] = e2;
            }
            if (s2 > ce2) {
                fT[fSwap ^ 1][iEnd2] = s2;
            } else if (e2 > ce2) {
                fT[fSwap ^ 1][iEnd2] = e2;
            }
            return;
        }
    } while (true);
    SkASSERT(fUsed < 9);
    insertCoincident(s1, s2, startPt);
    insertCoincident(e1, e2, endPt);
}

int Intersections::insert(double one, double two, const _Point& pt) {
    SkASSERT(fUsed <= 1 || fT[0][0] <= fT[0][1]);
    int index;
    for (index = 0; index < fUsed; ++index) {
        double oldOne = fT[0][index];
        double oldTwo = fT[1][index];
        if (roughly_equal(oldOne, one) && roughly_equal(oldTwo, two)) {
            if ((precisely_zero(one) && !precisely_zero(oldOne))
                    || (precisely_equal(one, 1) && !precisely_equal(oldOne, 1))
                    || (precisely_zero(two) && !precisely_zero(oldTwo))
                    || (precisely_equal(two, 1) && !precisely_equal(oldTwo, 1))) {
                fT[0][index] = one;
                fT[1][index] = two;
                fPt[index] = pt;
            }
            return -1;
        }
    #if ONE_OFF_DEBUG
        if (pt.roughlyEqual(fPt[index])) {
            SkDebugf("%s t=%1.9g pts roughly equal\n", __FUNCTION__, one);
        }
    #endif
        if (fT[0][index] > one) {
            break;
        }
    }
    SkASSERT(fUsed < 9);
    int remaining = fUsed - index;
    if (remaining > 0) {
        memmove(&fPt[index + 1], &fPt[index], sizeof(fPt[0]) * remaining);
        memmove(&fT[0][index + 1], &fT[0][index], sizeof(fT[0][0]) * remaining);
        memmove(&fT[1][index + 1], &fT[1][index], sizeof(fT[1][0]) * remaining);
        fIsCoincident[0] += fIsCoincident[0] & ~((1 << index) - 1);
        fIsCoincident[1] += fIsCoincident[1] & ~((1 << index) - 1);
    }
    fPt[index] = pt;
    fT[0][index] = one;
    fT[1][index] = two;
    ++fUsed;
    return index;
}

void Intersections::remove(double one, double two, const _Point& startPt, const _Point& endPt) {
    for (int index = fUsed - 1; index >= 0; --index) {
        if (!(fIsCoincident[0] & (1 << index)) && (between(one, fT[fSwap][index], two)
                || startPt.approximatelyEqual(fPt[index])
                || endPt.approximatelyEqual(fPt[index]))) {
            SkASSERT(fUsed > 0);
            removeOne(index);
        }
    }
}

void Intersections::removeOne(int index) {
    int remaining = --fUsed - index;
    if (remaining <= 0) {
        return;
    }
    memmove(&fPt[index], &fPt[index + 1], sizeof(fPt[0]) * remaining);
    memmove(&fT[0][index], &fT[0][index + 1], sizeof(fT[0][0]) * remaining);
    memmove(&fT[1][index], &fT[1][index + 1], sizeof(fT[1][0]) * remaining);
    SkASSERT(fIsCoincident[0] == 0);
    int coBit = fIsCoincident[0] & (1 << index);
    fIsCoincident[0] -= ((fIsCoincident[0] >> 1) & ~((1 << index) - 1)) + coBit;
    SkASSERT(!(coBit ^ (fIsCoincident[1] & (1 << index))));
    fIsCoincident[1] -= ((fIsCoincident[1] >> 1) & ~((1 << index) - 1)) + coBit;
}
