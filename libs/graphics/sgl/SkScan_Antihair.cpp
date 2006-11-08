/* libs/graphics/sgl/SkScan_Antihair.cpp
**
** Copyright 2006, Google Inc.
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

#include "SkScan.h"
#include "SkBlitter.h"
#include "SkRegion.h"
#include "SkFDot6.h"

#define HLINE_STACK_BUFFER      100

//#define TEST_GAMMA

#ifdef TEST_GAMMA
    static U8 gGammaTable[256];
    #define ApplyGamma(table, alpha)    (table)[alpha]

    static void build_gamma_table()
    {
        static bool gInit = false;

        if (gInit == false)
        {
            for (int i = 0; i < 256; i++)
            {
                SkFixed n = i * 257;
                n += n >> 15;
                SkASSERT(n >= 0 && n <= SK_Fixed1);
                n = SkFixedSqrt(n);
                n = n * 255 >> 16;
            //  SkDebugf("morph %d -> %d\n", i, n);
                gGammaTable[i] = SkToU8(n);
            }
            gInit = true;
        }
    }
#else
    #define ApplyGamma(table, alpha)    SkToU8(alpha)
#endif


static void call_hline_blitter(SkBlitter* blitter, int x, int y, int count, U8 alpha)
{
    SkASSERT(count > 0);

    S16 runs[HLINE_STACK_BUFFER + 1];
    U8  aa[HLINE_STACK_BUFFER];

    aa[0] = ApplyGamma(gGammaTable, alpha);
    do {
        int n = count;
        if (n > HLINE_STACK_BUFFER)
            n = HLINE_STACK_BUFFER;

        runs[0] = SkToS16(n);
        runs[n] = 0;
        blitter->blitAntiH(x, y, aa, runs);
        x += n;
        count -= n;
    } while (count > 0);
}

static void hline(int x, int stopx, SkFixed fy, SkFixed /*slope*/, SkBlitter* blitter)
{
    SkASSERT(x < stopx);
    int count = stopx - x;
    fy += SK_Fixed1/2;

    int y = fy >> 16;
    U8  a = (U8)(fy >> 8);

    // lower line
    if (a)
        call_hline_blitter(blitter, x, y, count, a);

    // upper line
    a = (U8)(255 - a);
    if (a)
        call_hline_blitter(blitter, x, y - 1, count, a);
}

static void horish(int x, int stopx, SkFixed fy, SkFixed dy, SkBlitter* blitter)
{
    SkASSERT(x < stopx);

#ifdef TEST_GAMMA
    const U8* gamma = gGammaTable;
#endif
    S16 runs[2];
    U8  aa[1];

    runs[0] = 1;
    runs[1] = 0;

    fy += SK_Fixed1/2;
    do {
        int lower_y = fy >> 16;
        U8  a = (U8)(fy >> 8);

        if (a)
        {
            aa[0] = ApplyGamma(gamma, a);
            blitter->blitAntiH(x, lower_y, aa, runs);
            // the clipping blitters might edit runs, but should not affect us
            SkASSERT(runs[0] == 1);
            SkASSERT(runs[1] == 0);
        }
        a = (U8)(255 - a);
        if (a)
        {
            aa[0] = ApplyGamma(gamma, a);
            blitter->blitAntiH(x, lower_y - 1, aa, runs);
            // the clipping blitters might edit runs, but should not affect us
            SkASSERT(runs[0] == 1);
            SkASSERT(runs[1] == 0);
        }
        fy += dy;
    } while (++x < stopx);
}

static void vline(int y, int stopy, SkFixed fx, SkFixed /*slope*/, SkBlitter* blitter)
{
    SkASSERT(y < stopy);
    fx += SK_Fixed1/2;

    int x = fx >> 16;
    int a = (U8)(fx >> 8);

    if (a)
        blitter->blitV(x, y, stopy - y, ApplyGamma(gGammaTable, a));
    a = 255 - a;
    if (a)
        blitter->blitV(x - 1, y, stopy - y, ApplyGamma(gGammaTable, a));
}

static void vertish(int y, int stopy, SkFixed fx, SkFixed dx, SkBlitter* blitter)
{
    SkASSERT(y < stopy);
#ifdef TEST_GAMMA
    const U8* gamma = gGammaTable;
#endif
    S16 runs[3];
    U8  aa[2];

    runs[0] = 1;
    runs[2] = 0;

    fx += SK_Fixed1/2;
    do {
        int x = fx >> 16;
        U8  a = (U8)(fx >> 8);

        aa[0] = ApplyGamma(gamma, 255 - a);
        aa[1] = ApplyGamma(gamma, a);
        // the clippng blitters might overwrite this guy, so we have to reset it each time
        runs[1] = 1;
        blitter->blitAntiH(x - 1, y, aa, runs);
        // the clipping blitters might edit runs, but should not affect us
        SkASSERT(runs[0] == 1);
        SkASSERT(runs[2] == 0);
        fx += dx;
    } while (++y < stopy);
}

typedef void (*LineProc)(int istart, int istop, SkFixed fstart, SkFixed slope, SkBlitter*);

static inline SkFixed fastfixdiv(SkFDot6 a, SkFDot6 b)
{
    SkASSERT((a << 16 >> 16) == a);
    SkASSERT(b != 0);
    return (a << 16) / b;
}
static inline SkFDot6 fastfixmul(SkFixed fixed, SkFDot6 b)
{
    SkASSERT(SkAbs32(fixed) <= SK_Fixed1 && SkAbs32(b) <= SkIntToFDot6(511));
    return (fixed * b + 0x8000) >> 16;
}

static void do_anti_hairline(SkFDot6 x0, SkFDot6 y0, SkFDot6 x1, SkFDot6 y1,
                             const SkRect16* clip, SkBlitter* blitter)
{
    // check that we're no larger than 511 pixels (so we can do a faster div).
    // if we are, subdivide and call again

    if (SkAbs32(x1 - x0) > SkIntToFDot6(511) || SkAbs32(y1 - y0) > SkIntToFDot6(511))
    {
        int hx = (x0 + x1) >> 1;
        int hy = (y0 + y1) >> 1;
        do_anti_hairline(x0, y0, hx, hy, clip, blitter);
        do_anti_hairline(hx, hy, x1, y1, clip, blitter);
        return;
    }

    int         istart, istop;
    SkFixed     fstart, slope; 
    LineProc    proc;

    if (SkAbs32(x1 - x0) > SkAbs32(y1 - y0))    // mostly horizontal
    {
        if (x0 > x1)    // we want to go left-to-right
        {
            SkTSwap<SkFDot6>(x0, x1);
            SkTSwap<SkFDot6>(y0, y1);
        }
        istart = SkFDot6Round(x0);
        istop = SkFDot6Round(x1);
        if (istart == istop)    // too short to draw
            return;

        if (y0 == y1)   // completely horizontal, take fast case
        {
            slope = 0;
            fstart = SkFDot6ToFixed(y0);
            proc = hline;
        }
        else
        {
            slope = fastfixdiv(y1 - y0, x1 - x0);
            SkASSERT(slope >= -SK_Fixed1 && slope <= SK_Fixed1);
            fstart = SkFDot6ToFixed(y0 + fastfixmul(slope, (32 - x0) & 63));
            proc = horish;
        }

        if (clip)
        {
            if (istart >= clip->fRight || istop <= clip->fLeft)
                return;
            if (istart < clip->fLeft)
            {
                fstart += slope * (clip->fLeft - istart);
                istart = clip->fLeft;
            }
            if (istop > clip->fRight)
                istop = clip->fRight;
            SkASSERT(istart <= istop);
            if (istart == istop)
                return;
            // now test if our Y values are completely inside the clip
            int top, bottom;
            if (slope >= 0) // T2B
            {
                top = SkFixedFloor(fstart - SK_FixedHalf);
                bottom = SkFixedCeil(fstart + (istop - istart - 1) * slope + SK_FixedHalf);
            }
            else            // B2T
            {
                bottom = SkFixedCeil(fstart + SK_FixedHalf);
                top = SkFixedFloor(fstart + (istop - istart - 1) * slope - SK_FixedHalf);
            }
            if (top >= clip->fBottom || bottom <= clip->fTop)
                return;
            if (clip->fTop <= top && clip->fBottom >= bottom)
                clip = nil;
        }
    }
    else    // mostly vertical
    {
        if (y0 > y1)    // we want to go top-to-bottom
        {
            SkTSwap<SkFDot6>(x0, x1);
            SkTSwap<SkFDot6>(y0, y1);
        }
        istart = SkFDot6Round(y0);
        istop = SkFDot6Round(y1);
        if (istart == istop)    // too short to draw
            return;

        if (x0 == x1)
        {
            slope = 0;
            fstart = SkFDot6ToFixed(x0);
            proc = vline;
        }
        else
        {
            slope = fastfixdiv(x1 - x0, y1 - y0);
            SkASSERT(slope <= SK_Fixed1 && slope >= -SK_Fixed1);
            fstart = SkFDot6ToFixed(x0 + fastfixmul(slope, (32 - y0) & 63));
            proc = vertish;
        }

        if (clip)
        {
            if (istart >= clip->fBottom || istop <= clip->fTop)
                return;
            if (istart < clip->fTop)
            {
                fstart += slope * (clip->fTop - istart);
                istart = clip->fTop;
            }
            if (istop > clip->fBottom)
                istop = clip->fBottom;
            SkASSERT(istart <= istop);
            if (istart == istop)
                return;
            // now test if our X values are completely inside the clip
            int left, right;
            if (slope >= 0) // L2R
            {
                left = SkFixedFloor(fstart - SK_FixedHalf);
                right = SkFixedCeil(fstart + (istop - istart - 1) * slope + SK_FixedHalf);
            }
            else            // R2L
            {
                right = SkFixedCeil(fstart + SK_FixedHalf);
                left = SkFixedFloor(fstart + (istop - istart - 1) * slope - SK_FixedHalf);
            }
            if (left >= clip->fRight || right <= clip->fLeft)
                return;
            if (clip->fLeft <= left && clip->fRight >= right)
                clip = nil;
        }
    }

    SkRectClipBlitter   rectClipper;
    if (clip)
    {
        rectClipper.init(blitter, *clip);
        blitter = &rectClipper;
    }
    proc(istart, istop, fstart, slope, blitter);
}

void SkScan::AntiHairLine(const SkPoint& pt0, const SkPoint& pt1,
                          const SkRegion* clip, SkBlitter* blitter)
{
    if (clip && clip->isEmpty())
        return;

    SkASSERT(clip == nil || !clip->getBounds().isEmpty());

#ifdef TEST_GAMMA
    build_gamma_table();
#endif

    SkFDot6 x0 = SkScalarToFDot6(pt0.fX);
    SkFDot6 y0 = SkScalarToFDot6(pt0.fY);
    SkFDot6 x1 = SkScalarToFDot6(pt1.fX);
    SkFDot6 y1 = SkScalarToFDot6(pt1.fY);

    if (clip)
    {
        SkFDot6     left = SkMin32(x0, x1);
        SkFDot6     top = SkMin32(y0, y1);
        SkFDot6     right = SkMax32(x0, x1);
        SkFDot6     bottom = SkMax32(y0, y1);
        SkRect16    ir;

        ir.set( SkFDot6Round(left) - 1,
                SkFDot6Round(top) - 1,
                SkFDot6Round(right) + 1,
                SkFDot6Round(bottom) + 1);

        if (clip->quickReject(ir))
            return;
        if (!clip->quickContains(ir))
        {
            SkRegion::Cliperator iter(*clip, ir);
            const SkRect16*      r = &iter.rect();

            while (!iter.done())
            {
                do_anti_hairline(x0, y0, x1, y1, r, blitter);
                iter.next();
            }
            return;
        }
        // fall through to no-clip case
    }
    do_anti_hairline(x0, y0, x1, y1, nil, blitter);
}

void SkScan::AntiHairRect(const SkRect& rect, const SkRegion* clip, SkBlitter* blitter)
{
    if (clip)
    {
        SkRect16    ir;
        SkRect      r = rect;

        r.inset(-SK_Scalar1/2, -SK_Scalar1/2);
        r.roundOut(&ir);
        if (clip->quickReject(ir))
            return;
        if (clip->quickContains(ir))
            clip = nil;
    }

    SkPoint p0, p1;

    p0.set(rect.fLeft, rect.fTop);
    p1.set(rect.fRight, rect.fTop);
    SkScan::AntiHairLine(p0, p1, clip, blitter);
    p0.set(rect.fRight, rect.fBottom);
    SkScan::AntiHairLine(p0, p1, clip, blitter);
    p1.set(rect.fLeft, rect.fBottom);
    SkScan::AntiHairLine(p0, p1, clip, blitter);
    p0.set(rect.fLeft, rect.fTop);
    SkScan::AntiHairLine(p0, p1, clip, blitter);
}


