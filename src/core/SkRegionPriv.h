/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkRegionPriv_DEFINED
#define SkRegionPriv_DEFINED

#include "SkRegion.h"

#include "SkAtomics.h"
#include "SkMalloc.h"

inline bool SkRegionValueIsSentinel(int32_t value) {
    return value == (int32_t)SkRegion::kRunTypeSentinel;
}

#define assert_sentinel(value, isSentinel) \
    SkASSERT(SkRegionValueIsSentinel(value) == isSentinel)

//SkDEBUGCODE(extern int32_t gRgnAllocCounter;)

#ifdef SK_DEBUG
// Given the first interval (just past the interval-count), compute the
// interval count, by search for the x-sentinel
//
static int compute_intervalcount(const SkRegion::RunType runs[]) {
    const SkRegion::RunType* curr = runs;
    while (*curr < SkRegion::kRunTypeSentinel) {
        SkASSERT(curr[0] < curr[1]);
        SkASSERT(curr[1] < SkRegion::kRunTypeSentinel);
        curr += 2;
    }
    return SkToInt((curr - runs) >> 1);
}
#endif

struct SkRegion::RunHead {
private:

public:
    int32_t fRefCnt;
    int32_t fRunCount;

    /**
     *  Number of spans with different Y values. This does not count the initial
     *  Top value, nor does it count the final Y-Sentinel value. In the logical
     *  case of a rectangle, this would return 1, and an empty region would
     *  return 0.
     */
    int getYSpanCount() const {
        return fYSpanCount;
    }

    /**
     *  Number of intervals in the entire region. This equals the number of
     *  rects that would be returned by the Iterator. In the logical case of
     *  a rect, this would return 1, and an empty region would return 0.
     */
    int getIntervalCount() const {
        return fIntervalCount;
    }

    static RunHead* Alloc(int count) {
        //SkDEBUGCODE(sk_atomic_inc(&gRgnAllocCounter);)
        //SkDEBUGF(("************** gRgnAllocCounter::alloc %d\n", gRgnAllocCounter));

        if (count < SkRegion::kRectRegionRuns) {
            return nullptr;
        }

        const int64_t size = sk_64_mul(count, sizeof(RunType)) + sizeof(RunHead);
        if (count < 0 || !sk_64_isS32(size)) { SK_ABORT("Invalid Size"); }

        RunHead* head = (RunHead*)sk_malloc_throw(size);
        head->fRefCnt = 1;
        head->fRunCount = count;
        // these must be filled in later, otherwise we will be invalid
        head->fYSpanCount = 0;
        head->fIntervalCount = 0;
        return head;
    }

    static RunHead* Alloc(int count, int yspancount, int intervalCount) {
        if (yspancount <= 0 || intervalCount <= 1) {
            return nullptr;
        }

        RunHead* head = Alloc(count);
        if (!head) {
            return nullptr;
        }
        head->fYSpanCount = yspancount;
        head->fIntervalCount = intervalCount;
        return head;
    }

    SkRegion::RunType* writable_runs() {
        SkASSERT(fRefCnt == 1);
        return (SkRegion::RunType*)(this + 1);
    }

    const SkRegion::RunType* readonly_runs() const {
        return (const SkRegion::RunType*)(this + 1);
    }

    RunHead* ensureWritable() {
        RunHead* writable = this;
        if (fRefCnt > 1) {
            // We need to alloc & copy the current region before we call
            // sk_atomic_dec because it could be freed in the meantime,
            // otherwise.
            writable = Alloc(fRunCount, fYSpanCount, fIntervalCount);
            memcpy(writable->writable_runs(), this->readonly_runs(),
                   fRunCount * sizeof(RunType));

            // fRefCount might have changed since we last checked.
            // If we own the last reference at this point, we need to
            // free the memory.
            if (sk_atomic_dec(&fRefCnt) == 1) {
                sk_free(this);
            }
        }
        return writable;
    }

    /**
     *  Given a scanline (including its Bottom value at runs[0]), return the next
     *  scanline. Asserts that there is one (i.e. runs[0] < Sentinel)
     */
    static SkRegion::RunType* SkipEntireScanline(const SkRegion::RunType runs[]) {
        // we are not the Y Sentinel
        SkASSERT(runs[0] < SkRegion::kRunTypeSentinel);

        const int intervals = runs[1];
        SkASSERT(runs[2 + intervals * 2] == SkRegion::kRunTypeSentinel);
#ifdef SK_DEBUG
        {
            int n = compute_intervalcount(&runs[2]);
            SkASSERT(n == intervals);
        }
#endif

        // skip the entire line [B N [L R] S]
        runs += 1 + 1 + intervals * 2 + 1;
        return const_cast<SkRegion::RunType*>(runs);
    }


    /**
     *  Return the scanline that contains the Y value. This requires that the Y
     *  value is already known to be contained within the bounds of the region,
     *  and so this routine never returns nullptr.
     *
     *  It returns the beginning of the scanline, starting with its Bottom value.
     */
    SkRegion::RunType* findScanline(int y) const {
        const RunType* runs = this->readonly_runs();

        // if the top-check fails, we didn't do a quick check on the bounds
        SkASSERT(y >= runs[0]);

        runs += 1;  // skip top-Y
        for (;;) {
            int bottom = runs[0];
            // If we hit this, we've walked off the region, and our bounds check
            // failed.
            SkASSERT(bottom < SkRegion::kRunTypeSentinel);
            if (y < bottom) {
                break;
            }
            runs = SkipEntireScanline(runs);
        }
        return const_cast<SkRegion::RunType*>(runs);
    }

    // Copy src runs into us, computing interval counts and bounds along the way
    void computeRunBounds(SkIRect* bounds) {
        RunType* runs = this->writable_runs();
        bounds->fTop = *runs++;

        int bot;
        int ySpanCount = 0;
        int intervalCount = 0;
        int left = SK_MaxS32;
        int rite = SK_MinS32;

        do {
            bot = *runs++;
            SkASSERT(bot < SkRegion::kRunTypeSentinel);
            ySpanCount += 1;

            const int intervals = *runs++;
            SkASSERT(intervals >= 0);
            SkASSERT(intervals < SkRegion::kRunTypeSentinel);

            if (intervals > 0) {
#ifdef SK_DEBUG
                {
                    int n = compute_intervalcount(runs);
                    SkASSERT(n == intervals);
                }
#endif
                RunType L = runs[0];
                SkASSERT(L < SkRegion::kRunTypeSentinel);
                if (left > L) {
                    left = L;
                }

                runs += intervals * 2;
                RunType R = runs[-1];
                SkASSERT(R < SkRegion::kRunTypeSentinel);
                if (rite < R) {
                    rite = R;
                }

                intervalCount += intervals;
            }
            SkASSERT(SkRegion::kRunTypeSentinel == *runs);
            runs += 1;  // skip x-sentinel

            // test Y-sentinel
        } while (SkRegion::kRunTypeSentinel > *runs);

#ifdef SK_DEBUG
        // +1 to skip the last Y-sentinel
        int runCount = SkToInt(runs - this->writable_runs() + 1);
        SkASSERT(runCount == fRunCount);
#endif

        fYSpanCount = ySpanCount;
        fIntervalCount = intervalCount;

        bounds->fLeft = left;
        bounds->fRight = rite;
        bounds->fBottom = bot;
    }

private:
    int32_t fYSpanCount;
    int32_t fIntervalCount;
};

#endif
