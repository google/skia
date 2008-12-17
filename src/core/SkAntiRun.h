/* libs/graphics/sgl/SkAntiRun.h
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

#ifndef SkAntiRun_DEFINED
#define SkAntiRun_DEFINED

#include "SkBlitter.h"

inline int sk_make_nonzero_neg_one(int x)
{
    return (x | -x) >> 31;
}

#if 0
template <int kShift> class SkAntiRun {
    static uint8_t coverage_to_alpha(int aa)
    {
        aa <<= 8 - 2*kShift;
        aa -= aa >> (8 - kShift - 1);
        return SkToU8(aa);
    }
public:
    void set(int start, int stop)
    {
        SkASSERT(start >= 0 && stop > start);

#if 1
        int fb = start & kMask;
        int fe = stop & kMask;
        int n = (stop >> kShift) - (start >> kShift) - 1;

        if (n < 0)
        {
            fb = fe - fb;
            n = 0;
            fe = 0;
        }
        else
        {
            if (fb == 0)
                n += 1;
            else
                fb = (1 << kShift) - fb;
        }

        fStartAlpha = coverage_to_alpha(fb);
        fMiddleCount = n;
        fStopAlpha = coverage_to_alpha(fe);
#else
        int x0 = start >> kShift;
        int x1 = (stop - 1) >> kShift;
        int middle = x1 - x0;
        int aa;

        if (middle == 0)
        {
            aa = stop - start;
            aa <<= 8 - 2*kShift;
            aa -= aa >> (8 - kShift - 1);
            SkASSERT(aa > 0 && aa < kMax);
            fStartAlpha = SkToU8(aa);
            fMiddleCount = 0;
            fStopAlpha = 0;
        }
        else
        {
            int aa = start & kMask;
            aa <<= 8 - 2*kShift;
            aa -= aa >> (8 - kShift - 1);
            SkASSERT(aa >= 0 && aa < kMax);
            if (aa)
                fStartAlpha = SkToU8(kMax - aa);
            else
            {
                fStartAlpha = 0;
                middle += 1;
            }
            aa = stop & kMask;
            aa <<= 8 - 2*kShift;
            aa -= aa >> (8 - kShift - 1);
            SkASSERT(aa >= 0 && aa < kMax);
            middle += sk_make_nonzero_neg_one(aa);

            fStopAlpha = SkToU8(aa);
            fMiddleCount = middle;
        }
        SkASSERT(fStartAlpha < kMax);
        SkASSERT(fStopAlpha < kMax);
#endif
    }

    void blit(int x, int y, SkBlitter* blitter)
    {
        int16_t runs[2];
        runs[0] = 1;
        runs[1] = 0;

        if (fStartAlpha)
        {
            blitter->blitAntiH(x, y, &fStartAlpha, runs);
            x += 1;
        }
        if (fMiddleCount)
        {
            blitter->blitH(x, y, fMiddleCount);
            x += fMiddleCount;
        }
        if (fStopAlpha)
            blitter->blitAntiH(x, y, &fStopAlpha, runs);
    }

    uint8_t  getStartAlpha() const { return fStartAlpha; }
    int getMiddleCount() const { return fMiddleCount; }
    uint8_t  getStopAlpha() const { return fStopAlpha; }

private:
    uint8_t  fStartAlpha, fStopAlpha;
    int fMiddleCount;

    enum {
        kMask = (1 << kShift) - 1,
        kMax = (1 << (8 - kShift)) - 1
    };
};
#endif

////////////////////////////////////////////////////////////////////////////////////

class SkAlphaRuns {
public:
    int16_t*    fRuns;
    uint8_t*     fAlpha;

    bool    empty() const
    {
        SkASSERT(fRuns[0] > 0);
        return fAlpha[0] == 0 && fRuns[fRuns[0]] == 0;
    }
    void    reset(int width);
    void    add(int x, U8CPU startAlpha, int middleCount, U8CPU stopAlpha, U8CPU maxValue);
    SkDEBUGCODE(void assertValid(int y, int maxStep) const;)
    SkDEBUGCODE(void dump() const;)

    static void Break(int16_t runs[], uint8_t alpha[], int x, int count);
    static void BreakAt(int16_t runs[], uint8_t alpha[], int x)
    {
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
    }

private:
    SkDEBUGCODE(int fWidth;)
    SkDEBUGCODE(void validate() const;)
};

#endif

