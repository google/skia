
/*
 * Copyright 2011 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkScan.h"
#include "SkBlitter.h"
#include "SkColorPriv.h"
#include "SkLineClipper.h"
#include "SkRasterClip.h"
#include "SkFDot6.h"

/*  Our attempt to compute the worst case "bounds" for the horizontal and
    vertical cases has some numerical bug in it, and we sometimes undervalue
    our extends. The bug is that when this happens, we will set the clip to
    NULL (for speed), and thus draw outside of the clip by a pixel, which might
    only look bad, but it might also access memory outside of the valid range
    allcoated for the device bitmap.

    This define enables our fix to outset our "bounds" by 1, thus avoiding the
    chance of the bug, but at the cost of sometimes taking the rectblitter
    case (i.e. not setting the clip to NULL) when we might not actually need
    to. If we can improve/fix the actual calculations, then we can remove this
    step.
 */
#define OUTSET_BEFORE_CLIP_TEST     true

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

    static void build_gamma_table() {
        static bool gInit = false;

        if (gInit == false) {
            for (int i = 0; i < 256; i++) {
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

static void call_hline_blitter(SkBlitter* blitter, int x, int y, int count,
                               U8CPU alpha) {
    SkASSERT(count > 0);

    int16_t runs[HLINE_STACK_BUFFER + 1];
    uint8_t  aa[HLINE_STACK_BUFFER];

    aa[0] = ApplyGamma(gGammaTable, alpha);
    do {
        int n = count;
        if (n > HLINE_STACK_BUFFER) {
            n = HLINE_STACK_BUFFER;
        }
        runs[0] = SkToS16(n);
        runs[n] = 0;
        blitter->blitAntiH(x, y, aa, runs);
        x += n;
        count -= n;
    } while (count > 0);
}

static SkFixed hline(int x, int stopx, SkFixed fy, SkFixed /*slope*/,
                     SkBlitter* blitter, int mod64) {
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

static SkFixed horish(int x, int stopx, SkFixed fy, SkFixed dy,
                      SkBlitter* blitter, int mod64) {
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
        if (ma) {
            aa[0] = ApplyGamma(gamma, ma);
            blitter->blitAntiH(x, lower_y, aa, runs);
            // the clipping blitters might edit runs, but should not affect us
            SkASSERT(runs[0] == 1);
            SkASSERT(runs[1] == 0);
        }
        ma = SmallDot6Scale(255 - a, mod64);
        if (ma) {
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

static SkFixed vline(int y, int stopy, SkFixed fx, SkFixed /*slope*/,
                     SkBlitter* blitter, int mod64) {
    SkASSERT(y < stopy);
    fx += SK_Fixed1/2;

    int x = fx >> 16;
    int a = (uint8_t)(fx >> 8);

    unsigned ma = SmallDot6Scale(a, mod64);
    if (ma) {
        blitter->blitV(x, y, stopy - y, ApplyGamma(gGammaTable, ma));
    }
    ma = SmallDot6Scale(255 - a, mod64);
    if (ma) {
        blitter->blitV(x - 1, y, stopy - y, ApplyGamma(gGammaTable, ma));
    }
    
    return fx - SK_Fixed1/2;
}

static SkFixed vertish(int y, int stopy, SkFixed fx, SkFixed dx,
                       SkBlitter* blitter, int mod64) {
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

typedef SkFixed (*LineProc)(int istart, int istop, SkFixed fstart,
                            SkFixed slope, SkBlitter*, int);

static inline SkFixed fastfixdiv(SkFDot6 a, SkFDot6 b) {
    SkASSERT((a << 16 >> 16) == a);
    SkASSERT(b != 0);
    return (a << 16) / b;
}

static void do_anti_hairline(SkFDot6 x0, SkFDot6 y0, SkFDot6 x1, SkFDot6 y1,
                             const SkIRect* clip, SkBlitter* blitter) {
    // check that we're no larger than 511 pixels (so we can do a faster div).
    // if we are, subdivide and call again

    if (SkAbs32(x1 - x0) > SkIntToFDot6(511) || SkAbs32(y1 - y0) > SkIntToFDot6(511)) {
        /*  instead of (x0 + x1) >> 1, we shift each separately. This is less
            precise, but avoids overflowing the intermediate result if the
            values are huge. A better fix might be to clip the original pts
            directly (i.e. do the divide), so we don't spend time subdividing
            huge lines at all.
         */
        int hx = (x0 >> 1) + (x1 >> 1);
        int hy = (y0 >> 1) + (y1 >> 1);
        do_anti_hairline(x0, y0, hx, hy, clip, blitter);
        do_anti_hairline(hx, hy, x1, y1, clip, blitter);
        return;
    }

    int         scaleStart, scaleStop;
    int         istart, istop;
    SkFixed     fstart, slope; 
    LineProc    proc;

    if (SkAbs32(x1 - x0) > SkAbs32(y1 - y0)) {   // mostly horizontal
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

        if (clip){
            if (istart >= clip->fRight || istop <= clip->fLeft) {
                return;
            }
            if (istart < clip->fLeft) {
                fstart += slope * (clip->fLeft - istart);
                istart = clip->fLeft;
                scaleStart = 64;
            }
            if (istop > clip->fRight) {
                istop = clip->fRight;
                scaleStop = 64;
            }
            SkASSERT(istart <= istop);
            if (istart == istop) {
                return;
            }
            // now test if our Y values are completely inside the clip
            int top, bottom;
            if (slope >= 0) { // T2B
                top = SkFixedFloor(fstart - SK_FixedHalf);
                bottom = SkFixedCeil(fstart + (istop - istart - 1) * slope + SK_FixedHalf);
            } else {           // B2T
                bottom = SkFixedCeil(fstart + SK_FixedHalf);
                top = SkFixedFloor(fstart + (istop - istart - 1) * slope - SK_FixedHalf);
            }
#ifdef OUTSET_BEFORE_CLIP_TEST
            top -= 1;
            bottom += 1;
#endif
            if (top >= clip->fBottom || bottom <= clip->fTop) {
                return;
            }
            if (clip->fTop <= top && clip->fBottom >= bottom) {
                clip = NULL;
            }
        }
    } else {   // mostly vertical
        if (y0 > y1) {  // we want to go top-to-bottom
            SkTSwap<SkFDot6>(x0, x1);
            SkTSwap<SkFDot6>(y0, y1);
        }

        istart = SkFDot6Floor(y0);
        istop = SkFDot6Ceil(y1);
        fstart = SkFDot6ToFixed(x0);
        if (x0 == x1) {
            if (y0 == y1) { // are we zero length?
                return;     // nothing to do
            }
            slope = 0;
            proc = vline;
        } else {
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
        
        if (clip) {
            if (istart >= clip->fBottom || istop <= clip->fTop) {
                return;
            }
            if (istart < clip->fTop) {
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
            if (slope >= 0) { // L2R
                left = SkFixedFloor(fstart - SK_FixedHalf);
                right = SkFixedCeil(fstart + (istop - istart - 1) * slope + SK_FixedHalf);
            } else {           // R2L
                right = SkFixedCeil(fstart + SK_FixedHalf);
                left = SkFixedFloor(fstart + (istop - istart - 1) * slope - SK_FixedHalf);
            }
#ifdef OUTSET_BEFORE_CLIP_TEST
            left -= 1;
            right += 1;
#endif
            if (left >= clip->fRight || right <= clip->fLeft) {
                return;
            }
            if (clip->fLeft <= left && clip->fRight >= right) {
                clip = NULL;
            }
        }
    }

    SkRectClipBlitter   rectClipper;
    if (clip) {
        rectClipper.init(blitter, *clip);
        blitter = &rectClipper;
    }
    
    fstart = proc(istart, istart + 1, fstart, slope, blitter, scaleStart);
    istart += 1;
    int fullSpans = istop - istart - (scaleStop > 0);
    if (fullSpans > 0) {
        fstart = proc(istart, istart + fullSpans, fstart, slope, blitter, 64);
    }
    if (scaleStop > 0) {
        proc(istop - 1, istop, fstart, slope, blitter, scaleStop);
    }
}

void SkScan::AntiHairLineRgn(const SkPoint& pt0, const SkPoint& pt1,
                             const SkRegion* clip, SkBlitter* blitter) {
    if (clip && clip->isEmpty()) {
        return;
    }

    SkASSERT(clip == NULL || !clip->getBounds().isEmpty());

#ifdef TEST_GAMMA
    build_gamma_table();
#endif

    SkPoint pts[2] = { pt0, pt1 };

    if (clip) {
        SkRect clipBounds;
        clipBounds.set(clip->getBounds());
        /*  We perform integral clipping later on, but we do a scalar clip first
            to ensure that our coordinates are expressible in fixed/integers.

            antialiased hairlines can draw up to 1/2 of a pixel outside of
            their bounds, so we need to outset the clip before calling the
            clipper. To make the numerics safer, we outset by a whole pixel,
            since the 1/2 pixel boundary is important to the antihair blitter,
            we don't want to risk numerical fate by chopping on that edge.
         */
        clipBounds.inset(-SK_Scalar1, -SK_Scalar1);

        if (!SkLineClipper::IntersectLine(pts, clipBounds, pts)) {
            return;
        }
    }
        
    SkFDot6 x0 = SkScalarToFDot6(pts[0].fX);
    SkFDot6 y0 = SkScalarToFDot6(pts[0].fY);
    SkFDot6 x1 = SkScalarToFDot6(pts[1].fX);
    SkFDot6 y1 = SkScalarToFDot6(pts[1].fY);

    if (clip) {
        SkFDot6 left = SkMin32(x0, x1);
        SkFDot6 top = SkMin32(y0, y1);
        SkFDot6 right = SkMax32(x0, x1);
        SkFDot6 bottom = SkMax32(y0, y1);
        SkIRect ir;

        ir.set( SkFDot6Floor(left) - 1,
                SkFDot6Floor(top) - 1,
                SkFDot6Ceil(right) + 1,
                SkFDot6Ceil(bottom) + 1);

        if (clip->quickReject(ir)) {
            return;
        }
        if (!clip->quickContains(ir)) {
            SkRegion::Cliperator iter(*clip, ir);
            const SkIRect*       r = &iter.rect();

            while (!iter.done()) {
                do_anti_hairline(x0, y0, x1, y1, r, blitter);
                iter.next();
            }
            return;
        }
        // fall through to no-clip case
    }
    do_anti_hairline(x0, y0, x1, y1, NULL, blitter);
}

void SkScan::AntiHairRect(const SkRect& rect, const SkRasterClip& clip,
                          SkBlitter* blitter) {
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

///////////////////////////////////////////////////////////////////////////////

typedef int FDot8;  // 24.8 integer fixed point

static inline FDot8 SkFixedToFDot8(SkFixed x) {
    return (x + 0x80) >> 8;
}

static void do_scanline(FDot8 L, int top, FDot8 R, U8CPU alpha,
                        SkBlitter* blitter) {
    SkASSERT(L < R);
    
    if ((L >> 8) == ((R - 1) >> 8)) {  // 1x1 pixel
        blitter->blitV(L >> 8, top, 1, SkAlphaMul(alpha, R - L));
        return;
    }
    
    int left = L >> 8;
    
    if (L & 0xFF) {
        blitter->blitV(left, top, 1, SkAlphaMul(alpha, 256 - (L & 0xFF)));
        left += 1;
    }

    int rite = R >> 8;
    int width = rite - left;
    if (width > 0) {
        call_hline_blitter(blitter, left, top, width, alpha);
    }
    if (R & 0xFF) {
        blitter->blitV(rite, top, 1, SkAlphaMul(alpha, R & 0xFF));
    }
}

static void antifilldot8(FDot8 L, FDot8 T, FDot8 R, FDot8 B, SkBlitter* blitter,
                         bool fillInner) {
    // check for empty now that we're in our reduced precision space
    if (L >= R || T >= B) {
        return;
    }
    int top = T >> 8;
    if (top == ((B - 1) >> 8)) {   // just one scanline high
        do_scanline(L, top, R, B - T - 1, blitter);
        return;
    }
    
    if (T & 0xFF) {
        do_scanline(L, top, R, 256 - (T & 0xFF), blitter);
        top += 1;
    }
    
    int bot = B >> 8;
    int height = bot - top;
    if (height > 0) {
        int left = L >> 8;
        if (left == ((R - 1) >> 8)) {   // just 1-pixel wide
            blitter->blitV(left, top, height, R - L - 1);
        } else {
            if (L & 0xFF) {
                blitter->blitV(left, top, height, 256 - (L & 0xFF));
                left += 1;
            }
            int rite = R >> 8;
            int width = rite - left;
            if (width > 0 && fillInner) {
                blitter->blitRect(left, top, width, height);
            }
            if (R & 0xFF) {
                blitter->blitV(rite, top, height, R & 0xFF);
            }
        }
    }
    
    if (B & 0xFF) {
        do_scanline(L, bot, R, B & 0xFF, blitter);
    }
}

static void antifillrect(const SkXRect& xr, SkBlitter* blitter) {
    antifilldot8(SkFixedToFDot8(xr.fLeft), SkFixedToFDot8(xr.fTop),
                 SkFixedToFDot8(xr.fRight), SkFixedToFDot8(xr.fBottom),
                 blitter, true);
}

///////////////////////////////////////////////////////////////////////////////

void SkScan::AntiFillXRect(const SkXRect& xr, const SkRegion* clip,
                          SkBlitter* blitter) {
    if (NULL == clip) {
        antifillrect(xr, blitter);
    } else {
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
    }
}

void SkScan::AntiFillXRect(const SkXRect& xr, const SkRasterClip& clip,
                           SkBlitter* blitter) {
    if (clip.isBW()) {
        AntiFillXRect(xr, &clip.bwRgn(), blitter);
    } else {
        SkIRect outerBounds;
        XRect_roundOut(xr, &outerBounds);

        if (clip.quickContains(outerBounds)) {
            AntiFillXRect(xr, NULL, blitter);
        } else {
            SkAAClipBlitterWrapper wrapper(clip, blitter);
            blitter = wrapper.getBlitter();

            AntiFillXRect(xr, &wrapper.getRgn(), wrapper.getBlitter());
        }
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
void SkScan::AntiFillRect(const SkRect& origR, const SkRegion* clip,
                          SkBlitter* blitter) {
    if (clip) {
        SkRect newR;
        newR.set(clip->getBounds());
        if (!newR.intersect(origR)) {
            return;
        }

        SkIRect outerBounds;
        newR.roundOut(&outerBounds);
        
        if (clip->isRect()) {
            antifillrect(newR, blitter);
        } else {
            SkRegion::Cliperator clipper(*clip, outerBounds);
            while (!clipper.done()) {
                newR.set(clipper.rect());
                if (newR.intersect(origR)) {
                    antifillrect(newR, blitter);
                }
                clipper.next();
            }
        }
    } else {
        antifillrect(origR, blitter);
    }
}

void SkScan::AntiFillRect(const SkRect& r, const SkRasterClip& clip,
                          SkBlitter* blitter) {
    if (clip.isBW()) {
        AntiFillRect(r, &clip.bwRgn(), blitter);
    } else {
        SkAAClipBlitterWrapper wrap(clip, blitter);
        AntiFillRect(r, &wrap.getRgn(), wrap.getBlitter());
    }
}

#endif // SK_SCALAR_IS_FLOAT

///////////////////////////////////////////////////////////////////////////////

#define SkAlphaMulRound(a, b)   SkMulDiv255Round(a, b)

// calls blitRect() if the rectangle is non-empty
static void fillcheckrect(int L, int T, int R, int B, SkBlitter* blitter) {
    if (L < R && T < B) {
        blitter->blitRect(L, T, R - L, B - T);
    }
}

static inline FDot8 SkScalarToFDot8(SkScalar x) {
#ifdef SK_SCALAR_IS_FLOAT
    return (int)(x * 256);
#else
    return x >> 8;
#endif
}

static inline int FDot8Floor(FDot8 x) {
    return x >> 8;
}

static inline int FDot8Ceil(FDot8 x) {
    return (x + 0xFF) >> 8;
}

// 1 - (1 - a)*(1 - b)
static inline U8CPU InvAlphaMul(U8CPU a, U8CPU b) {
    // need precise rounding (not just SkAlphaMul) so that values like
    // a=228, b=252 don't overflow the result
    return SkToU8(a + b - SkAlphaMulRound(a, b));
}

static void inner_scanline(FDot8 L, int top, FDot8 R, U8CPU alpha,
                           SkBlitter* blitter) {
    SkASSERT(L < R);
    
    if ((L >> 8) == ((R - 1) >> 8)) {  // 1x1 pixel
        blitter->blitV(L >> 8, top, 1, InvAlphaMul(alpha, R - L));
        return;
    }
    
    int left = L >> 8;
    if (L & 0xFF) {
        blitter->blitV(left, top, 1, InvAlphaMul(alpha, L & 0xFF));
        left += 1;
    }
    
    int rite = R >> 8;
    int width = rite - left;
    if (width > 0) {
        call_hline_blitter(blitter, left, top, width, alpha);
    }
    
    if (R & 0xFF) {
        blitter->blitV(rite, top, 1, InvAlphaMul(alpha, ~R & 0xFF));
    }
}

static void innerstrokedot8(FDot8 L, FDot8 T, FDot8 R, FDot8 B,
                            SkBlitter* blitter) {
    SkASSERT(L < R && T < B);

    int top = T >> 8;
    if (top == ((B - 1) >> 8)) {   // just one scanline high
        inner_scanline(L, top, R, B - T, blitter);
        return;
    }
    
    if (T & 0xFF) {
        inner_scanline(L, top, R, T & 0xFF, blitter);
        top += 1;
    }
    
    int bot = B >> 8;
    int height = bot - top;
    if (height > 0) {
        if (L & 0xFF) {
            blitter->blitV(L >> 8, top, height, L & 0xFF);
        }
        if (R & 0xFF) {
            blitter->blitV(R >> 8, top, height, ~R & 0xFF);
        }
    }
    
    if (B & 0xFF) {
        inner_scanline(L, bot, R, ~B & 0xFF, blitter);
    }
}

void SkScan::AntiFrameRect(const SkRect& r, const SkPoint& strokeSize,
                           const SkRegion* clip, SkBlitter* blitter) {
    SkASSERT(strokeSize.fX >= 0 && strokeSize.fY >= 0);

    SkScalar rx = SkScalarHalf(strokeSize.fX);
    SkScalar ry = SkScalarHalf(strokeSize.fY);

    // outset by the radius
    FDot8 L = SkScalarToFDot8(r.fLeft - rx);
    FDot8 T = SkScalarToFDot8(r.fTop - ry);
    FDot8 R = SkScalarToFDot8(r.fRight + rx);
    FDot8 B = SkScalarToFDot8(r.fBottom + ry);

    SkIRect outer;
    // set outer to the outer rect of the outer section
    outer.set(FDot8Floor(L), FDot8Floor(T), FDot8Ceil(R), FDot8Ceil(B));

    SkBlitterClipper clipper;
    if (clip) {
        if (clip->quickReject(outer)) {
            return;
        }
        if (!clip->contains(outer)) {
            blitter = clipper.apply(blitter, clip, &outer);
        }
        // now we can ignore clip for the rest of the function
    }
    
    // stroke the outer hull
    antifilldot8(L, T, R, B, blitter, false);

    // set outer to the outer rect of the middle section
    outer.set(FDot8Ceil(L), FDot8Ceil(T), FDot8Floor(R), FDot8Floor(B));

    // in case we lost a bit with diameter/2
    rx = strokeSize.fX - rx;
    ry = strokeSize.fY - ry;
    // inset by the radius
    L = SkScalarToFDot8(r.fLeft + rx);
    T = SkScalarToFDot8(r.fTop + ry);
    R = SkScalarToFDot8(r.fRight - rx);
    B = SkScalarToFDot8(r.fBottom - ry);

    if (L >= R || T >= B) {
        fillcheckrect(outer.fLeft, outer.fTop, outer.fRight, outer.fBottom,
                      blitter);
    } else {
        SkIRect inner;
        // set inner to the inner rect of the middle section
        inner.set(FDot8Floor(L), FDot8Floor(T), FDot8Ceil(R), FDot8Ceil(B));

        // draw the frame in 4 pieces
        fillcheckrect(outer.fLeft, outer.fTop, outer.fRight, inner.fTop,
                      blitter);
        fillcheckrect(outer.fLeft, inner.fTop, inner.fLeft, inner.fBottom,
                      blitter);
        fillcheckrect(inner.fRight, inner.fTop, outer.fRight, inner.fBottom,
                      blitter);
        fillcheckrect(outer.fLeft, inner.fBottom, outer.fRight, outer.fBottom,
                      blitter);

        // now stroke the inner rect, which is similar to antifilldot8() except that
        // it treats the fractional coordinates with the inverse bias (since its
        // inner).
        innerstrokedot8(L, T, R, B, blitter);
    }
}

void SkScan::AntiFrameRect(const SkRect& r, const SkPoint& strokeSize,
                           const SkRasterClip& clip, SkBlitter* blitter) {
    if (clip.isBW()) {
        AntiFrameRect(r, strokeSize, &clip.bwRgn(), blitter);
    } else {
        SkAAClipBlitterWrapper wrap(clip, blitter);
        AntiFrameRect(r, strokeSize, &wrap.getRgn(), wrap.getBlitter());
    }
}

