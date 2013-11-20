/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTDStackNester_DEFINED
#define SkTDStackNester_DEFINED

#include "SkTypes.h"

// Adobe limits it to 28, so 256 should be more than enough
#define MAX_NESTING 256

/** \class SkTDStackNester
 *
 * The difference between SkTDStackNester and SkTDStack is that:
 *   - SkTDStackNester uses new/delete to manage initializations
 *   - Supports nest/unnest which simulates a stack of stack. unnest will pop all the
 *     objects pushed since the last nest
 */

template <typename T> class SkTDStackNester : SkNoncopyable {
public:
    SkTDStackNester() : fCount(0), fTotalCount(0), fLocalCount(0) {
        fInitialRec.fNext = NULL;
        fRec = &fInitialRec;

    //  fCount = kSlotCount;
    }

    ~SkTDStackNester() {
        Rec* rec = fRec;
        while (rec != &fInitialRec) {
            Rec* next = rec->fNext;
            delete rec;
            rec = next;
        }
    }

    int count() const { return fLocalCount; }
    bool empty() const { return fLocalCount == 0; }

    int nests() {
        return fNestingLevel;
    }

    void nest() {
        // We are are past max nesting levels, we will still continue to work, but we might fail
        // to properly ignore errors. Ideally it should only mean poor rendering in exceptional
        // cases
        if (fNestingLevel >= 0 && fNestingLevel < MAX_NESTING) {
            fNestings[fNestingLevel] = fLocalCount;
            fLocalCount = 0;
        }
        fNestingLevel++;
    }

    void unnest() {
        SkASSERT(fNestingLevel > 0);
        fNestingLevel--;
        if (fNestingLevel >= 0 && fNestingLevel < MAX_NESTING) {
            // TODO(edisonn): warn if fLocal > 0
            while (fLocalCount > 0) {
                pop();
            }
            fLocalCount = fNestings[fNestingLevel];
        }
    }

    T* push() {
        SkASSERT(fCount <= kSlotCount);
        if (fCount == kSlotCount) {
            Rec* rec = new Rec();
            rec->fNext = fRec;
            fRec = rec;
            fCount = 0;
        }
        ++fTotalCount;
        ++fLocalCount;
        return &fRec->fSlots[fCount++];
    }

    void push(const T& elem) { *this->push() = elem; }

    const T& index(int idx) const {
        SkASSERT(fRec && fCount > idx);
        return fRec->fSlots[fCount - idx - 1];
    }

    T& index(int idx) {
        SkASSERT(fRec && fCount > idx);
        return fRec->fSlots[fCount - idx - 1];
    }

    const T& top() const {
        SkASSERT(fRec && fCount > 0);
        return fRec->fSlots[fCount - 1];
    }

    T& top() {
        SkASSERT(fRec && fCount > 0);
        return fRec->fSlots[fCount - 1];
    }

    void pop(T* elem) {
        if (elem) {
            *elem = fRec->fSlots[fCount - 1];
        }
        this->pop();
    }

    void pop() {
        SkASSERT(fCount > 0 && fRec);
        --fLocalCount;
        --fTotalCount;
        if (--fCount == 0) {
            if (fRec != &fInitialRec) {
                Rec* rec = fRec->fNext;
                delete fRec;
                fCount = kSlotCount;
                fRec = rec;
            } else {
                SkASSERT(fTotalCount == 0);
            }
        }
    }

private:
    enum {
        kSlotCount  = 64
    };

    struct Rec;
    friend struct Rec;

    struct Rec {
        Rec* fNext;
        T    fSlots[kSlotCount];
    };
    Rec     fInitialRec;
    Rec*    fRec;
    int     fCount, fTotalCount, fLocalCount;
    int     fNestings[MAX_NESTING];
    int     fNestingLevel;
};
#endif // SkTDStackNester_DEFINED
