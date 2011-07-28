
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkAntiRun.h"
#include "SkUtils.h"

void SkAlphaRuns::reset(int width) {
    SkASSERT(width > 0);

#ifdef SK_DEBUG
    sk_memset16((uint16_t*)fRuns, (uint16_t)(-42), width);
#endif
    fRuns[0] = SkToS16(width);
    fRuns[width] = 0;
    fAlpha[0] = 0;

    SkDEBUGCODE(fWidth = width;)
    SkDEBUGCODE(this->validate();)
}

void SkAlphaRuns::Break(int16_t runs[], uint8_t alpha[], int x, int count) {
    SkASSERT(count > 0 && x >= 0);

//  SkAlphaRuns::BreakAt(runs, alpha, x);
//  SkAlphaRuns::BreakAt(&runs[x], &alpha[x], count);

    int16_t* next_runs = runs + x;
    uint8_t*  next_alpha = alpha + x;

    while (x > 0) {
        int n = runs[0];
        SkASSERT(n > 0);

        if (x < n) {
            alpha[x] = alpha[0];
            runs[0] = SkToS16(x);
            runs[x] = SkToS16(n - x);
            break;
        }
        runs += n;
        alpha += n;
        x -= n;
    }

    runs = next_runs;
    alpha = next_alpha;
    x = count;

    for (;;) {
        int n = runs[0];
        SkASSERT(n > 0);

        if (x < n) {
            alpha[x] = alpha[0];
            runs[0] = SkToS16(x);
            runs[x] = SkToS16(n - x);
            break;
        }
        x -= n;
        if (x <= 0) {
            break;
        }
        runs += n;
        alpha += n;
    }
}

int SkAlphaRuns::add(int x, U8CPU startAlpha, int middleCount, U8CPU stopAlpha,
                     U8CPU maxValue, int offsetX) {
    SkASSERT(middleCount >= 0);
    SkASSERT(x >= 0 && x + (startAlpha != 0) + middleCount + (stopAlpha != 0) <= fWidth);

    SkASSERT(fRuns[offsetX] >= 0);

    int16_t*    runs = fRuns + offsetX;
    uint8_t*    alpha = fAlpha + offsetX;
    uint8_t*    lastAlpha = alpha;
    x -= offsetX;

    if (startAlpha) {
        SkAlphaRuns::Break(runs, alpha, x, 1);
        /*  I should be able to just add alpha[x] + startAlpha.
            However, if the trailing edge of the previous span and the leading
            edge of the current span round to the same super-sampled x value,
            I might overflow to 256 with this add, hence the funny subtract (crud).
        */
        unsigned tmp = alpha[x] + startAlpha;
        SkASSERT(tmp <= 256);
        alpha[x] = SkToU8(tmp - (tmp >> 8));    // was (tmp >> 7), but that seems wrong if we're trying to catch 256

        runs += x + 1;
        alpha += x + 1;
        x = 0;
        lastAlpha += x; // we don't want the +1
        SkDEBUGCODE(this->validate();)
    }

    if (middleCount) {
        SkAlphaRuns::Break(runs, alpha, x, middleCount);
        alpha += x;
        runs += x;
        x = 0;
        do {
            alpha[0] = SkToU8(alpha[0] + maxValue);
            int n = runs[0];
            SkASSERT(n <= middleCount);
            alpha += n;
            runs += n;
            middleCount -= n;
        } while (middleCount > 0);
        SkDEBUGCODE(this->validate();)
        lastAlpha = alpha;
    }

    if (stopAlpha) {
        SkAlphaRuns::Break(runs, alpha, x, 1);
        alpha += x;
        alpha[0] = SkToU8(alpha[0] + stopAlpha);
        SkDEBUGCODE(this->validate();)
        lastAlpha = alpha;
    }

    return lastAlpha - fAlpha;  // new offsetX
}

#ifdef SK_DEBUG
    void SkAlphaRuns::assertValid(int y, int maxStep) const {
        int max = (y + 1) * maxStep - (y == maxStep - 1);

        const int16_t* runs = fRuns;
        const uint8_t*   alpha = fAlpha;

        while (*runs) {
            SkASSERT(*alpha <= max);
            alpha += *runs;
            runs += *runs;
        }
    }

    void SkAlphaRuns::dump() const {
        const int16_t* runs = fRuns;
        const uint8_t* alpha = fAlpha;

        SkDebugf("Runs");
        while (*runs) {
            int n = *runs;

            SkDebugf(" %02x", *alpha);
            if (n > 1) {
                SkDebugf(",%d", n);
            }
            alpha += n;
            runs += n;
        }
        SkDebugf("\n");
    }

    void SkAlphaRuns::validate() const {
        SkASSERT(fWidth > 0);

        int         count = 0;
        const int16_t*  runs = fRuns;

        while (*runs) {
            SkASSERT(*runs > 0);
            count += *runs;
            SkASSERT(count <= fWidth);
            runs += *runs;
        }
        SkASSERT(count == fWidth);
    }
#endif

