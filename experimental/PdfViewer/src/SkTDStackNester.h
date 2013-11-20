/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTDStackNester_DEFINED
#define SkTDStackNester_DEFINED

#include "SkTypes.h"
#include "SkPdfReporter.h"

// Adobe limits it to 28. Allow deeper nesting in case a file does not quite meet the
// spec. 256 should be more than enough.
#define MAX_NESTING 256

/** \class SkTDStackNester
 *
 * Specialized version of SkTDStack which allows a stack of stacks.
 * FIXME (scroggo): Could this be a subclass of SkTDStack? Could it have-a SkTDStack?
 * The difference between SkTDStackNester and SkTDStack is that:
 *   - SkTDStackNester uses new/delete to manage initializations
 *     FIXME (scroggo): Why use new rather than malloc?
 *   - Supports nest/unnest which simulates a stack of stack. unnest will pop all the
 *     objects pushed since the last nest
 *   - kSlotCount is 64, instead of 8.
 *     FIXME (scroggo): How did we arrive at this number?
 */

template <typename T> class SkTDStackNester : SkNoncopyable {
public:
    SkTDStackNester()
        : fCount(0)
        , fLocalCount(0)
        , fNestingLevel(0) {
        fInitialRec.fNext = NULL;
        fRec = &fInitialRec;
        SkDEBUGCODE(fTotalCount = 0;)
    }

    ~SkTDStackNester() {
        Rec* rec = fRec;
        while (rec != &fInitialRec) {
            Rec* next = rec->fNext;
            delete rec;
            rec = next;
        }
    }

    /**
     * Return the number of objects in the current nesting level.
     */
    int count() const { return fLocalCount; }

    /**
     * Whether the current nesting level is empty.
     */
    bool empty() const { return fLocalCount == 0; }

    /**
     * The current nesting level.
     */
    int nestingLevel() const {
        return fNestingLevel;
    }

    /**
     * Analogous to an SkCanvas::save(). When unnest() is called, the state of this SkTDStackNester
     * will return to its state when nest() was called.
     *
     * After a call to nest(), fLocalCount is reset to 0, since the stack is on a new nesting
     * level.
     */
    void nest() {
        SkASSERT(fNestingLevel >= 0);
        if (fNestingLevel < MAX_NESTING) {
            fNestings[fNestingLevel] = fLocalCount;
            fLocalCount = 0;
        } else {
            // We are are past max nesting levels. We will still continue to work, but we might fail
            // to properly ignore errors. Ideally it should only mean poor rendering in exceptional
            // cases.
            SkPdfReport(kWarning_SkPdfIssueSeverity, kStackNestingOverflow_SkPdfIssue,
                        "Past maximum nesting level", NULL, NULL);
        }
        fNestingLevel++;
    }

    /**
     * Analagous to an SkCanvas::restore(). Will revert this stack to the state it was in the last
     * time nest() was called. It is an error to call unnest() more times than nest() has been
     * called.
     */
    void unnest() {
        SkASSERT(fNestingLevel >= 0);
        if (0 == fNestingLevel) {
            SkPdfReport(kWarning_SkPdfIssueSeverity, kStackNestingOverflow_SkPdfIssue,
                        "Nesting underflow", NULL, NULL);
            return;
        }

        fNestingLevel--;
        if (fNestingLevel < MAX_NESTING) {
            while (fLocalCount > 0) {
                // FIXME (scroggo): Pass the object?
                SkPdfReport(kInfo_SkPdfIssueSeverity, kUnusedObject_SkPdfIssue,
                            "Unused object when calling unnest!", NULL, NULL);
                this->pop();
            }
            fLocalCount = fNestings[fNestingLevel];
        }
    }

    /**
     * Add an object to the stack, and return a pointer to it for modification.
     */
    T* push() {
        SkASSERT(fCount <= kSlotCount);
        if (fCount == kSlotCount) {
            Rec* rec = new Rec();
            rec->fNext = fRec;
            fRec = rec;
            fCount = 0;
        }
        SkDEBUGCODE(++fTotalCount;)
        ++fLocalCount;
        return &fRec->fSlots[fCount++];
    }

    /**
     * Add an object to the stack, copied from elem.
     */
    void push(const T& elem) { *this->push() = elem; }

    /**
     * Return the top element.
     */
    const T& top() const {
        SkASSERT(fRec && fCount > 0);
        return fRec->fSlots[fCount - 1];
    }

    /**
     * Return the top element.
     */
    T& top() {
        SkASSERT(fRec && fCount > 0);
        return fRec->fSlots[fCount - 1];
    }

    /**
     * Pop an object off the stack (via pop()), and copy its members into elem.
     */
    void pop(T* elem) {
        if (elem) {
            *elem = fRec->fSlots[fCount - 1];
        }
        this->pop();
    }

    /**
     * Pop an object off the stack. It is an error to call pop() more times
     * than push() has been called in total or since the last call to nest().
     */
    void pop() {
        SkASSERT(fCount > 0 && fRec);
        SkASSERT(fLocalCount > 0);
        --fLocalCount;
        SkDEBUGCODE(--fTotalCount;)
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
        // Number of objects held per Rec. Storing multiple objects in one Rec
        // means that we call new less often.
        kSlotCount  = 64
    };

    struct Rec {
        Rec* fNext;
        T    fSlots[kSlotCount];
    };

    // First Rec, requiring no allocation.
    Rec     fInitialRec;
    // The Rec on top of the stack.
    Rec*    fRec;
    // Number of objects in fRec.
    int     fCount;
    // Number of objects in the current nesting level.
    int     fLocalCount;
    // Array of counts of objects per nesting level.
    // Only valid for fNestings[0] through fNestings[fNestingLevel-1].
    int     fNestings[MAX_NESTING];
    // Current nesting level.
    int     fNestingLevel;
    // Total number of objects in this SkTDStackNester.
    SkDEBUGCODE(int     fTotalCount;)

    // For testing.
    friend class SkTDStackNesterTester;
};
#endif // SkTDStackNester_DEFINED
