/* libs/graphics/sgl/SkScan_Antihair.cpp
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

#include "SkScan.h"
#include "SkBlitter.h"
#include "SkColorPriv.h"
#include "SkRegion.h"
#include "SkFDot6.h"

#define HLINE_STACK_BUFFER      100

static inline int SmallDot6Scale(int value, int dot6) {
    SkASSERT((int16_t)value == value);
    SkASSERT((unsigned)dot6 <= 64);
    return SkMulS16(value, dot6) >> 6;
}

//#define TEST_GAMMA

#ifdef TEST_GAMMA
    static uint8_t gGammaTable[256];
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

///////////////////////////////////////////////////////////////////////////////

static void call_hline_blitter(SkBlitter* blitter, int x, int y, int count, U8CPU alpha)
{
    SkASSERT(count > 0);

    int16_t runs[HLINE_STACK_BUFFER + 1];
    uint8_t  aa[HLINE_STACK_BUFFER];

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

static SkFixed hline(int x, int stopx, SkFixed fy, SkFixed /*slope*/, SkBlitter* blitter, int mod64)
{
    SkASSERT(x < stopx);
    int count = stopx - x;
    fy += SK_Fixed1/2;

    int y = fy >> 16;
    uint8_t  a = (uint8_t)(fy >> 8);

    // lower line
    unsigned ma = SmallDot6Scale(a, mod64);
    if (ma) {
        call_hline_blitter(blitter, x, y, count, ma);
    }

    // upper line
    ma = SmallDot6Scale(255 - a, mod64);
    if (ma) {
        call_hline_blitter(blitter, x, y - 1, count, ma);
    }
    
    return fy - SK_Fixed1/2;
}

static SkFixed horish(int x, int stopx, SkFixed fy, SkFixed dy, SkBlitter* blitter, int mod64)
{
    SkASSERT(x < stopx);

#ifdef TEST_GAMMA
    const uint8_t* gamma = gGammaTable;
#endif
    int16_t runs[2];
    uint8_t  aa[1];

    runs[0] = 1;
    runs[1] = 0;

    fy += SK_Fixed1/2;
    do {
        int lower_y = fy >> 16;
        uint8_t  a = (uint8_t)(fy >> 8);
        unsigned ma = SmallDot6Scale(a, mod64);
        if (ma)
        {
            aa[0] = ApplyGamma(gamma, ma);
            blitter->blitAntiH(x, lower_y, aa, runs);
            // the clipping blitters might edit runs, but should not affect us
            SkASSERT(runs[0] == 1);
            SkASSERT(runs[1] == 0);
        }
        ma = SmallDot6Scale(255 - a, mod64);
        if (ma)
        {
            aa[0] = ApplyGamma(gamma, ma);
            blitter->blitAntiH(x, lower_y - 1, aa, runs);
            // the clipping blitters might edit runs, but should not affect us
            SkASSERT(runs[0] == 1);
            SkASSERT(runs[1] == 0);
        }
        fy += dy;
    } while (++x < stopx);
    
    return fy - SK_Fixed1/2;
}

static SkFixed vline(int y, int stopy, SkFixed fx, SkFixed /*slope*/, SkBlitter* blitter, int mod64)
{
    SkASSERT(y < stopy);
    fx += SK_Fixed1/2;

    int x = fx >> 16;
    int a = (uint8_t)(fx >> 8);

    unsigned ma = SmallDot6Scale(a, mod64);
    if (ma)
        blitter->blitV(x, y, stopy - y, ApplyGamma(gGammaTable, ma));
    ma = SmallDot6Scale(255 - a, mod64);
    if (ma)
        blitter->blitV(x - 1, y, stopy - y, ApplyGamma(gGammaTable, ma));
    
    return fx - SK_Fixed1/2;
}

static SkFixed vertish(int y, int stopy, SkFixed fx, SkFixed dx, SkBlitter* blitter, int mod64)
{
    SkASSERT(y < stopy);
#ifdef TEST_GAMMA
    const uint8_t* gamma = gGammaTable;
#endif
    int16_t runs[3];
    uint8_t  aa[2];

    runs[0] = 1;
    runs[2] = 0;

    fx += SK_Fixed1/2;
    do {
        int x = fx >> 16;
        uint8_t  a = (uint8_t)(fx >> 8);

        aa[0] = ApplyGamma(gamma, SmallDot6Scale(255 - a, mod64));
        aa[1] = ApplyGamma(gamma, SmallDot6Scale(a, mod64));
        // the clippng blitters might overwrite this guy, so we have to reset it each time
        runs[1] = 1;
        blitter->blitAntiH(x - 1, y, aa, runs);
        // the clipping blitters might edit runs, but should not affect us
        SkASSERT(runs[0] == 1);
        SkASSERT(runs[2] == 0);
        fx += dx;
    } while (++y < stopy);

    return fx - SK_Fixed1/2;
}

typedef SkFixed (*LineProc)(int istart, int istop, SkFixed fstart, SkFixed slope, SkBlitter*, int);

static inline SkFixed fastfixdiv(SkFDot6 a, SkFDot6 b)
{
    SkASSERT((a << 16 >> 16) == a);
    SkASSERT(b != 0);
    return (a << 16) / b;
}

static void do_anti_hairline(SkFDot6 x0, SkFDot6 y0, SkFDot6 x1, SkFDot6 y1,
                             const SkIRect* clip, SkBlitter* blitter)
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

    int         scaleStart, scaleStop;
    int         istart, istop;
    SkFixed     fstart, slope; 
    LineProc    proc;

    if (SkAbs32(x1 - x0) > SkAbs32(y1 - y0))    // mostly horizontal
    {
        if (x0 > x1) {    // we want to go left-to-right
            SkTSwap<SkFDot6>(x0, x1);
            SkTSwap<SkFDot6>(y0, y1);
        }

        istart = SkFDot6Floor(x0);
        istop = SkFDot6Ceil(x1);
        fstart = SkFDot6ToFixed(y0);
        if (y0 == y1) {   // completely horizontal, take fast case
            slope = 0;
            proc = hline;
        } else {
            slope = fastfixdiv(y1 - y0, x1 - x0);
            SkASSERT(slope >= -SK_Fixed1 && slope <= SK_Fixed1);
            fstart += (slope * (32 - (x0 & 63)) + 32) >> 6;
            proc = horish;
        }
        
        SkASSERT(istop > istart);
        if (istop - istart == 1) {
            scaleStart = x1 - x0;
            SkASSERT(scaleStart >= 0 && scaleStart <= 64);
            scaleStop = 0;
        } else {
            scaleStart = 64 - (x0 & 63);
            scaleStop = x1 & 63;
        }

        if (clip)
        {
            if (istart >= clip->fRight || istop <= clip->fLeft)
                return;
            if (istart < clip->fLeft)
            {
                fstart += slope * (clip->fLeft - istart);
                istart = clip->fLeft;
                scaleStart = 64;
            }
            if (istop > clip->fRight) {
                istop = clip->fRight;
                scaleStop = 64;
            }
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
                clip = NULL;
        }
    }
    else    // mostly vertical
    {
        if (y0 > y1)    // we want to go top-to-bottom
        {
            SkTSwap<SkFDot6>(x0, x1);
            SkTSwap<SkFDot6>(y0, y1);
        }

        istart = SkFDot6Floor(y0);
        istop = SkFDot6Ceil(y1);
        fstart = SkFDot6ToFixed(x0);
        if (x0 == x1)
        {
            if (y0 == y1) { // are we zero length?
                return;     // nothing to do
            }
            slope = 0;
            proc = vline;
        }
        else
        {
            slope = fastfixdiv(x1 - x0, y1 - y0);
            SkASSERT(slope <= SK_Fixed1 && slope >= -SK_Fixed1);
            fstart += (slope * (32 - (y0 & 63)) + 32) >> 6;
            proc = vertish;
        }

        SkASSERT(istop > istart);
        if (istop - istart == 1) {
            scaleStart = y1 - y0;
            SkASSERT(scaleStart >= 0 && scaleStart <= 64);
            scaleStop = 0;
        } else {
            scaleStart = 64 - (y0 & 63);
            scaleStop = y1 & 63;
        }
        
        if (clip)
        {
            if (istart >= clip->fBottom || istop <= clip->fTop)
                return;
            if (istart < clip->fTop)
            {
                fstart += slope * (clip->fTop - istart);
                istart = clip->fTop;
                scaleStart = 64;
            }
            if (istop > clip->fBottom) {
                istop = clip->fBottom;
                scaleStop = 64;
            }
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
                clip = NULL;
        }
    }

    SkRectClipBlitter   rectClipper;
    if (clip)
    {
        rectClipper.init(blitter, *clip);
        blitter = &rectClipper;
    }
    
    fstart = proc(istart, istart + 1, fstart, slope, blitter, scaleStart);
    istart += 1;
    int fullSpans = istop - istart - 1;
    if (fullSpans > 0) {
        fstart = proc(istart, istart + fullSpans, fstart, slope, blitter, 64);
    }
    if (scaleStop > 0) {
        proc(istop - 1, istop, fstart, slope, blitter, scaleStop);
    }
}

void SkScan::AntiHairLine(const SkPoint& pt0, const SkPoint& pt1,
                          const SkRegion* clip, SkBlitter* blitter)
{
    if (clip && clip->isEmpty())
        return;

    SkASSERT(clip == NULL || !clip->getBounds().isEmpty());

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
        SkIRect     ir;

        ir.set( SkFDot6Floor(left) - 1,
                SkFDot6Floor(top) - 1,
                SkFDot6Ceil(right) + 1,
                SkFDot6Ceil(bottom) + 1);

        if (clip->quickReject(ir))
            return;
        if (!clip->quickContains(ir))
        {
            SkRegion::Cliperator iter(*clip, ir);
            const SkIRect*       r = &iter.rect();

            while (!iter.done())
            {
                do_anti_hairline(x0, y0, x1, y1, r, blitter);
                iter.next();
            }
            return;
        }
        // fall through to no-clip case
    }
    do_anti_hairline(x0, y0, x1, y1, NULL, blitter);
}

void SkScan::AntiHairRect(const SkRect& rect, const SkRegion* clip, SkBlitter* blitter)
{
    if (clip)
    {
        SkIRect ir;
        SkRect  r = rect;

        r.inset(-SK_Scalar1/2, -SK_Scalar1/2);
        r.roundOut(&ir);
        if (clip->quickReject(ir))
            return;
        if (clip->quickContains(ir))
            clip = NULL;
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

//////////////////////////////////////////////////////////////////////////////////////////

typedef int FDot8;  // 24.8 integer fixed point

static inline FDot8 SkFixedToFDot8(SkFixed x) {
    return (x + 0x80) >> 8;
}

static void do_scanline(FDot8 L, int top, FDot8 R, U8CPU alpha, SkBlitter* blitter)
{
    SkASSERT(L < R);
    
    if ((L >> 8) == ((R - 1) >> 8))  // 1x1 pixel
    {
        blitter->blitV(L >> 8, top, 1, SkAlphaMul(alpha, R - L));
        return;
    }
    
    int left = L >> 8;
    
    if (L & 0xFF)
    {
        blitter->blitV(left, top, 1, SkAlphaMul(alpha, 256 - (L & 0xFF)));
        left += 1;
    }

    int rite = R >> 8;
    int width = rite - left;
    if (width > 0)
        call_hline_blitter(blitter, left, top, width, alpha);

    if (R & 0xFF)
        blitter->blitV(rite, top, 1, SkAlphaMul(alpha, R & 0xFF));
}

static void antifillrect(const SkXRect& xr, SkBlitter* blitter)
{
    FDot8 L = SkFixedToFDot8(xr.fLeft);
    FDot8 T = SkFixedToFDot8(xr.fTop);
    FDot8 R = SkFixedToFDot8(xr.fRight);
    FDot8 B = SkFixedToFDot8(xr.fBottom);
    
    // check for empty now that we're in our reduced precision space
    if (L >= R || T >= B)
        return;
    
    int top = T >> 8;
    if (top == ((B - 1) >> 8))   // just one scanline high
    {
        do_scanline(L, top, R, B - T - 1, blitter);
        return;
    }
    
    if (T & 0xFF)
    {
        do_scanline(L, top, R, 256 - (T & 0xFF), blitter);
        top += 1;
    }
    
    int bot = B >> 8;
    int height = bot - top;
    if (height > 0)
    {
        int left = L >> 8;
        if (L & 0xFF)
        {
            blitter->blitV(left, top, height, 256 - (L & 0xFF));
            left += 1;
        }
        int rite = R >> 8;
        int width = rite - left;
        if (width > 0)
            blitter->blitRect(left, top, width, height);
        if (R & 0xFF)
            blitter->blitV(rite, top, height, R & 0xFF);
    }
    
    if (B & 0xFF)
        do_scanline(L, bot, R, B & 0xFF, blitter);
}

///////////////////////////////////////////////////////////////////////////////

void SkScan::AntiFillXRect(const SkXRect& xr, const SkRegion* clip,
                          SkBlitter* blitter) {
    if (clip) {
        SkIRect outerBounds;
        XRect_roundOut(xr, &outerBounds);

        if (clip->isRect()) {
            const SkIRect& clipBounds = clip->getBounds();

            if (clipBounds.contains(outerBounds)) {
                antifillrect(xr, blitter);
            } else {
                SkXRect tmpR;
                // this keeps our original edges fractional
                XRect_set(&tmpR, clipBounds);
                if (tmpR.intersect(xr)) {
                    antifillrect(tmpR, blitter);
                }
            }
        } else {
            SkRegion::Cliperator clipper(*clip, outerBounds);
            const SkIRect&       rr = clipper.rect();
            
            while (!clipper.done()) {
                SkXRect  tmpR;
                
                // this keeps our original edges fractional
                XRect_set(&tmpR, rr);
                if (tmpR.intersect(xr)) {
                    antifillrect(tmpR, blitter);
                }
                clipper.next();
            }
        }
    } else {
        antifillrect(xr, blitter);
    }
}

#ifdef SK_SCALAR_IS_FLOAT

/*  This guy takes a float-rect, but with the key improvement that it has
    already been clipped, so we know that it is safe to convert it into a
    XRect (fixedpoint), as it won't overflow.
*/
static void antifillrect(const SkRect& r, SkBlitter* blitter) {
    SkXRect xr;
    
    XRect_set(&xr, r);
    antifillrect(xr, blitter);
}

/*  We repeat the clipping logic of AntiFillXRect because the float rect might
    overflow if we blindly converted it to an XRect. This sucks that we have to
    repeat the clipping logic, but I don't see how to share the code/logic.
 
    We clip r (as needed) into one or more (smaller) float rects, and then pass
    those to our version of antifillrect, which converts it into an XRect and
    then calls the blit.
*/
void SkScan::AntiFillRect(const SkRect& r, const SkRegion* clip,
                          SkBlitter* blitter) {
    if (clip) {
        SkIRect outerBounds;
        r.roundOut(&outerBounds);
        
        if (clip->isRect()) {
            const SkIRect& clipBounds = clip->getBounds();
            
            if (clipBounds.contains(outerBounds)) {
                antifillrect(r, blitter);
            } else {
                SkRect tmpR;
                // this keeps our original edges fractional
                tmpR.set(clipBounds);
                if (tmpR.intersect(r)) {
                    antifillrect(tmpR, blitter);
                }
            }
        } else {
            SkRegion::Cliperator clipper(*clip, outerBounds);
            const SkIRect&       rr = clipper.rect();
            
            while (!clipper.done()) {
                SkRect  tmpR;
                // this keeps our original edges fractional
                tmpR.set(rr);
                if (tmpR.intersect(r)) {
                    antifillrect(tmpR, blitter);
                }
                clipper.next();
            }
        }
    } else {
        antifillrect(r, blitter);
    }
}

#endif


