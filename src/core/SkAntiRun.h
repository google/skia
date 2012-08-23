
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkAntiRun_DEFINED
#define SkAntiRun_DEFINED

#include "SkBlitter.h"

/** Sparse array of run-length-encoded alpha (supersampling coverage) values.
    Sparseness allows us to independently compose several paths into the
    same SkAlphaRuns buffer.
*/

class SkAlphaRuns {
public:
    int16_t*    fRuns;
    uint8_t*     fAlpha;

    /// Returns true if the scanline contains only a single run,
    /// of alpha value 0.
    bool empty() const {
        SkASSERT(fRuns[0] > 0);
        return fAlpha[0] == 0 && fRuns[fRuns[0]] == 0;
    }

    /// Reinitialize for a new scanline.
    void    reset(int width);

    /**
     *  Insert into the buffer a run starting at (x-offsetX):
     *      if startAlpha > 0
     *          one pixel with value += startAlpha,
     *              max 255
     *      if middleCount > 0
     *          middleCount pixels with value += maxValue
     *      if stopAlpha > 0
     *          one pixel with value += stopAlpha
     *  Returns the offsetX value that should be passed on the next call,
     *  assuming we're on the same scanline. If the caller is switching
     *  scanlines, then offsetX should be 0 when this is called.
     */
    int add(int x, U8CPU startAlpha, int middleCount, U8CPU stopAlpha,
            U8CPU maxValue, int offsetX);

    SkDEBUGCODE(void assertValid(int y, int maxStep) const;)
    SkDEBUGCODE(void dump() const;)

    /**
     * Break the runs in the buffer at offsets x and x+count, properly
     * updating the runs to the right and left.
     *   i.e. from the state AAAABBBB, run-length encoded as A4B4,
     *   Break(..., 2, 5) would produce AAAABBBB rle as A2A2B3B1.
     * Allows add() to sum another run to some of the new sub-runs.
     *   i.e. adding ..CCCCC. would produce AADDEEEB, rle as A2D2E3B1.
     */
    static void Break(int16_t runs[], uint8_t alpha[], int x, int count);

    /**
     * Cut (at offset x in the buffer) a run into two shorter runs with
     * matching alpha values.
     * Used by the RectClipBlitter to trim a RLE encoding to match the
     * clipping rectangle.
     */
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

