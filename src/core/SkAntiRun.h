
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkAntiRun_DEFINED
#define SkAntiRun_DEFINED

#include "SkBlitter.h"

class SkAlphaRuns {
public:
    int16_t*    fRuns;
    uint8_t*     fAlpha;

    bool empty() const {
        SkASSERT(fRuns[0] > 0);
        return fAlpha[0] == 0 && fRuns[fRuns[0]] == 0;
    }

    void    reset(int width);
    
    /**
     *  Returns the offsetX value that should be passed on the next call,
     *  assuming we're on the same scanline. If the caller is switching
     *  scanlines, then offsetX should be 0 when this is called.
     */
    int add(int x, U8CPU startAlpha, int middleCount, U8CPU stopAlpha,
            U8CPU maxValue, int offsetX);

    SkDEBUGCODE(void assertValid(int y, int maxStep) const;)
    SkDEBUGCODE(void dump() const;)

    static void Break(int16_t runs[], uint8_t alpha[], int x, int count);

    static void BreakAt(int16_t runs[], uint8_t alpha[], int x) {
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
    }

private:
    SkDEBUGCODE(int fWidth;)
    SkDEBUGCODE(void validate() const;)
};

#endif

