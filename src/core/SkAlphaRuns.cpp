/* libs/graphics/sgl/SkAlphaRuns.cpp
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#include "SkAntiRun.h"

void SkAlphaRuns::reset(int width)
{
    SkASSERT(width > 0);

    fRuns[0] = SkToS16(width);
    fRuns[width] = 0;
    fAlpha[0] = 0;

    SkDEBUGCODE(fWidth = width;)
    SkDEBUGCODE(this->validate();)
}

void SkAlphaRuns::Break(int16_t runs[], uint8_t alpha[], int x, int count)
{
    SkASSERT(count > 0 && x >= 0);

//  SkAlphaRuns::BreakAt(runs, alpha, x);
//  SkAlphaRuns::BreakAt(&runs[x], &alpha[x], count);

    int16_t* next_runs = runs + x;
    uint8_t*  next_alpha = alpha + x;

    while (x > 0)
    {
        int n = runs[0];
        SkASSERT(n > 0);

        if (x < n)
        {
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

    for (;;)
    {
        int n = runs[0];
        SkASSERT(n > 0);

        if (x < n)
        {
            alpha[x] = alpha[0];
            runs[0] = SkToS16(x);
            runs[x] = SkToS16(n - x);
            break;
        }
        x -= n;
        if (x <= 0)
            break;

        runs += n;
        alpha += n;
    }
}

void SkAlphaRuns::add(int x, U8CPU startAlpha, int middleCount, U8CPU stopAlpha, U8CPU maxValue)
{
    SkASSERT(middleCount >= 0);
    SkASSERT(x >= 0 && x + (startAlpha != 0) + middleCount + (stopAlpha != 0) <= fWidth);

    int16_t*    runs = fRuns;
    uint8_t*     alpha = fAlpha;

    if (startAlpha)
    {
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
        SkDEBUGCODE(this->validate();)
    }
    if (middleCount)
    {
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
    }
    if (stopAlpha)
    {
        SkAlphaRuns::Break(runs, alpha, x, 1);
        alpha[x] = SkToU8(alpha[x] + stopAlpha);
        SkDEBUGCODE(this->validate();)
    }
}

#ifdef SK_DEBUG
    void SkAlphaRuns::assertValid(int y, int maxStep) const
    {
        int max = (y + 1) * maxStep - (y == maxStep - 1);

        const int16_t* runs = fRuns;
        const uint8_t*   alpha = fAlpha;

        while (*runs)
        {
            SkASSERT(*alpha <= max);
            alpha += *runs;
            runs += *runs;
        }
    }

    void SkAlphaRuns::dump() const
    {
        const int16_t* runs = fRuns;
        const uint8_t* alpha = fAlpha;

        SkDebugf("Runs");
        while (*runs)
        {
            int n = *runs;

            SkDebugf(" %02x", *alpha);
            if (n > 1)
                SkDebugf(",%d", n);
            alpha += n;
            runs += n;
        }
        SkDebugf("\n");
    }

    void SkAlphaRuns::validate() const
    {
        SkASSERT(fWidth > 0);

        int         count = 0;
        const int16_t*  runs = fRuns;

        while (*runs)
        {
            SkASSERT(*runs > 0);
            count += *runs;
            SkASSERT(count <= fWidth);
            runs += *runs;
        }
        SkASSERT(count == fWidth);
    }
#endif

