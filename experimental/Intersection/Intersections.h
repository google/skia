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
        : fFlip(0)
#if SK_DEBUG
        , fDepth(0)
#endif
        , fSwap(0)
    {
#if SK_DEBUG
        bzero(fPt, sizeof(fPt));
        bzero(fT, sizeof(fT));
        bzero(fIsCoincident, sizeof(fIsCoincident));
#endif
        reset();
    }

    int coincidentUsed() const {
        if (!fIsCoincident[0]) {
            SkASSERT(!fIsCoincident[0]);
            return 0;
        }
        int count = 0;
        SkDEBUGCODE(int count2 = 0;)
        for (int index = 0; index < fUsed; ++index) {
            if (fIsCoincident[0] & (1 << index)) {
                ++count;
            }
    #if SK_DEBUG
            if (fIsCoincident[1] & (1 << index)) {
                ++count2;
            }
    #endif
        }
        SkASSERT(count == count2);
        return count;
    }

    void offset(int base, double start, double end) {
        for (int index = base; index < fUsed; ++index) {
            double val = fT[fSwap][index];
            val *= end - start;
            val += start;
            fT[fSwap][index] = val;
        }
    }

    // FIXME : does not respect swap
    int insert(double one, double two, const _Point& pt);

    // start if index == 0 : end if index == 1
    void insertCoincident(double one, double two, const _Point& pt) {
        int index = insertSwap(one, two, pt);
        int bit = 1 << index;
        fIsCoincident[0] |= bit;
        fIsCoincident[1] |= bit;
    }

    void insertCoincidentPair(double s1, double e1, double s2, double e2,
            const _Point& startPt, const _Point& endPt);

    int insertSwap(double one, double two, const _Point& pt) {
        if (fSwap) {
            return insert(two, one, pt);
        } else {
            return insert(one, two, pt);
        }
    }

    bool intersected() const {
        return fUsed > 0;
    }

    void removeOne(int index);

    // leaves flip, swap alone
    void reset() {
        fUsed = 0;
        fUnsortable = false;
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

    void downDepth() {
        SkASSERT(--fDepth >= 0);
    }

    void upDepth() {
        SkASSERT(++fDepth < 16);
    }

#if SK_DEBUG
    int depth() const {
        return fDepth;
    }
#endif

    _Point fPt[9];
    double fT[2][9];
    unsigned short fIsCoincident[2]; // bit arrays, one bit set for each coincident T
    unsigned char fUsed;
    bool fFlip;
    bool fUnsortable;
#if SK_DEBUG
    int fDepth;
#endif
protected:
    // used by addCoincident to remove ordinary intersections in range
    void remove(double one, double two, const _Point& startPt, const _Point& endPt);
private:
    bool fSwap;
};

#endif
