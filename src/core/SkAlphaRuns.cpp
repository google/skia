/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkTo.h"
#include "src/core/SkAntiRun.h"
#include "src/core/SkOpts.h"

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
