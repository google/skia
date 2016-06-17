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
    nullptr (for speed), and thus draw outside of the clip by a pixel, which might
    only look bad, but it might also access memory outside of the valid range
    allcoated for the device bitmap.

    This define enables our fix to outset our "bounds" by 1, thus avoiding the
    chance of the bug, but at the cost of sometimes taking the rectblitter
    case (i.e. not setting the clip to nullptr) when we might not actually need
    to. If we can improve/fix the actual calculations, then we can remove this
    step.
 */
#define OUTSET_BEFORE_CLIP_TEST     true

#define HLINE_STACK_BUFFER      100

static inline int SmallDot6Scale(int value, int dot6) {
    SkASSERT((int16_t)value == value);
    SkASSERT((unsigned)dot6 <= 64);
    return (value * dot6) >> 6;
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

class SkAntiHairBlitter {
public:
    SkAntiHairBlitter() : fBlitter(nullptr) {}
    virtual ~SkAntiHairBlitter() {}

    SkBlitter* getBlitter() const { return fBlitter; }

    void setup(SkBlitter* blitter) {
        fBlitter = blitter;
    }

    virtual SkFixed drawCap(int x, SkFixed fy, SkFixed slope, int mod64) = 0;
    virtual SkFixed drawLine(int x, int stopx, SkFixed fy, SkFixed slope) = 0;

private:
    SkBlitter*  fBlitter;
};

class HLine_SkAntiHairBlitter : public SkAntiHairBlitter {
public:
    SkFixed drawCap(int x, SkFixed fy, SkFixed slope, int mod64) override {
        fy += SK_Fixed1/2;

        int y = fy >> 16;
        uint8_t  a = (uint8_t)(fy >> 8);

        // lower line
        unsigned ma = SmallDot6Scale(a, mod64);
        if (ma) {
            call_hline_blitter(this->getBlitter(), x, y, 1, ma);
        }

        // upper line
        ma = SmallDot6Scale(255 - a, mod64);
        if (ma) {
            call_hline_blitter(this->getBlitter(), x, y - 1, 1, ma);
        }

        return fy - SK_Fixed1/2;
    }

    virtual SkFixed drawLine(int x, int stopx, SkFixed fy,
                             SkFixed slope) override {
        SkASSERT(x < stopx);
        int count = stopx - x;
        fy += SK_Fixed1/2;

        int y = fy >> 16;
        uint8_t  a = (uint8_t)(fy >> 8);

        // lower line
        if (a) {
            call_hline_blitter(this->getBlitter(), x, y, count, a);
        }

        // upper line
        a = 255 - a;
        if (a) {
            call_hline_blitter(this->getBlitter(), x, y - 1, count, a);
        }

        return fy - SK_Fixed1/2;
    }
};

class Horish_SkAntiHairBlitter : public SkAntiHairBlitter {
public:
    SkFixed drawCap(int x, SkFixed fy, SkFixed dy, int mod64) override {
        fy += SK_Fixed1/2;

        int lower_y = fy >> 16;
        uint8_t  a = (uint8_t)(fy >> 8);
        unsigned a0 = SmallDot6Scale(255 - a, mod64);
        unsigned a1 = SmallDot6Scale(a, mod64);
        this->getBlitter()->blitAntiV2(x, lower_y - 1, a0, a1);

        return fy + dy - SK_Fixed1/2;
    }

    SkFixed drawLine(int x, int stopx, SkFixed fy, SkFixed dy) override {
        SkASSERT(x < stopx);

        fy += SK_Fixed1/2;
        SkBlitter* blitter = this->getBlitter();
        do {
            int lower_y = fy >> 16;
            uint8_t  a = (uint8_t)(fy >> 8);
            blitter->blitAntiV2(x, lower_y - 1, 255 - a, a);
            fy += dy;
        } while (++x < stopx);

        return fy - SK_Fixed1/2;
    }
};

class VLine_SkAntiHairBlitter : public SkAntiHairBlitter {
public:
    SkFixed drawCap(int y, SkFixed fx, SkFixed dx, int mod64) override {
        SkASSERT(0 == dx);
        fx += SK_Fixed1/2;

        int x = fx >> 16;
        int a = (uint8_t)(fx >> 8);

        unsigned ma = SmallDot6Scale(a, mod64);
        if (ma) {
            this->getBlitter()->blitV(x, y, 1, ma);
        }
        ma = SmallDot6Scale(255 - a, mod64);
        if (ma) {
            this->getBlitter()->blitV(x - 1, y, 1, ma);
        }

        return fx - SK_Fixed1/2;
    }

    SkFixed drawLine(int y, int stopy, SkFixed fx, SkFixed dx) override {
        SkASSERT(y < stopy);
        SkASSERT(0 == dx);
        fx += SK_Fixed1/2;

        int x = fx >> 16;
        int a = (uint8_t)(fx >> 8);

        if (a) {
            this->getBlitter()->blitV(x, y, stopy - y, a);
        }
        a = 255 - a;
        if (a) {
            this->getBlitter()->blitV(x - 1, y, stopy - y, a);
        }

        return fx - SK_Fixed1/2;
    }
};

class Vertish_SkAntiHairBlitter : public SkAntiHairBlitter {
public:
    SkFixed drawCap(int y, SkFixed fx, SkFixed dx, int mod64) override {
        fx += SK_Fixed1/2;

        int x = fx >> 16;
        uint8_t a = (uint8_t)(fx >> 8);
        this->getBlitter()->blitAntiH2(x - 1, y,
                                       SmallDot6Scale(255 - a, mod64), SmallDot6Scale(a, mod64));

        return fx + dx - SK_Fixed1/2;
    }

    SkFixed drawLine(int y, int stopy, SkFixed fx, SkFixed dx) override {
        SkASSERT(y < stopy);
        fx += SK_Fixed1/2;
        do {
            int x = fx >> 16;
            uint8_t a = (uint8_t)(fx >> 8);
            this->getBlitter()->blitAntiH2(x - 1, y, 255 - a, a);
            fx += dx;
        } while (++y < stopy);

        return fx - SK_Fixed1/2;
    }
};

static inline SkFixed fastfixdiv(SkFDot6 a, SkFDot6 b) {
    SkASSERT((SkLeftShift(a, 16) >> 16) == a);
    SkASSERT(b != 0);
    return SkLeftShift(a, 16) / b;
}

#define SkBITCOUNT(x)   (sizeof(x) << 3)

#if 1
// returns high-bit set iff x==0x8000...
static inline int bad_int(int x) {
    return x & -x;
}

static int any_bad_ints(int a, int b, int c, int d) {
    return (bad_int(a) | bad_int(b) | bad_int(c) | bad_int(d)) >> (SkBITCOUNT(int) - 1);
}
#else
static inline int good_int(int x) {
    return x ^ (1 << (SkBITCOUNT(x) - 1));
}

static int any_bad_ints(int a, int b, int c, int d) {
    return !(good_int(a) & good_int(b) & good_int(c) & good_int(d));
}
#endif

#ifdef SK_DEBUG
static bool canConvertFDot6ToFixed(SkFDot6 x) {
    const int maxDot6 = SK_MaxS32 >> (16 - 6);
    return SkAbs32(x) <= maxDot6;
}
#endif

/*
 *  We want the fractional part of ordinate, but we want multiples of 64 to
 *  return 64, not 0, so we can't just say (ordinate & 63).
 *  We basically want to compute those bits, and if they're 0, return 64.
 *  We can do that w/o a branch with an extra sub and add.
 */
static int contribution_64(SkFDot6 ordinate) {
#if 0
    int result = ordinate & 63;
    if (0 == result) {
        result = 64;
    }
#else
    int result = ((ordinate - 1) & 63) + 1;
#endif
    SkASSERT(result > 0 && result <= 64);
    return result;
}

static void do_anti_hairline(SkFDot6 x0, SkFDot6 y0, SkFDot6 x1, SkFDot6 y1,
                             const SkIRect* clip, SkBlitter* blitter) {
    // check for integer NaN (0x80000000) which we can't handle (can't negate it)
    // It appears typically from a huge float (inf or nan) being converted to int.
    // If we see it, just don't draw.
    if (any_bad_ints(x0, y0, x1, y1)) {
        return;
    }

    // The caller must clip the line to [-32767.0 ... 32767.0] ahead of time
    // (in dot6 format)
    SkASSERT(canConvertFDot6ToFixed(x0));
    SkASSERT(canConvertFDot6ToFixed(y0));
    SkASSERT(canConvertFDot6ToFixed(x1));
    SkASSERT(canConvertFDot6ToFixed(y1));

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

    HLine_SkAntiHairBlitter     hline_blitter;
    Horish_SkAntiHairBlitter    horish_blitter;
    VLine_SkAntiHairBlitter     vline_blitter;
    Vertish_SkAntiHairBlitter   vertish_blitter;
    SkAntiHairBlitter*          hairBlitter = nullptr;

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
            hairBlitter = &hline_blitter;
        } else {
            slope = fastfixdiv(y1 - y0, x1 - x0);
            SkASSERT(slope >= -SK_Fixed1 && slope <= SK_Fixed1);
            fstart += (slope * (32 - (x0 & 63)) + 32) >> 6;
            hairBlitter = &horish_blitter;
        }

        SkASSERT(istop > istart);
        if (istop - istart == 1) {
            // we are within a single pixel
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
                if (istop - istart == 1) {
                    // we are within a single pixel
                    scaleStart = contribution_64(x1);
                    scaleStop = 0;
                }
            }
            if (istop > clip->fRight) {
                istop = clip->fRight;
                scaleStop = 0;  // so we don't draw this last column
            }

            SkASSERT(istart <= istop);
            if (istart == istop) {
                return;
            }
            // now test if our Y values are completely inside the clip
            int top, bottom;
            if (slope >= 0) { // T2B
                top = SkFixedFloorToInt(fstart - SK_FixedHalf);
                bottom = SkFixedCeilToInt(fstart + (istop - istart - 1) * slope + SK_FixedHalf);
            } else {           // B2T
                bottom = SkFixedCeilToInt(fstart + SK_FixedHalf);
                top = SkFixedFloorToInt(fstart + (istop - istart - 1) * slope - SK_FixedHalf);
            }
#ifdef OUTSET_BEFORE_CLIP_TEST
            top -= 1;
            bottom += 1;
#endif
            if (top >= clip->fBottom || bottom <= clip->fTop) {
                return;
            }
            if (clip->fTop <= top && clip->fBottom >= bottom) {
                clip = nullptr;
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
            hairBlitter = &vline_blitter;
        } else {
            slope = fastfixdiv(x1 - x0, y1 - y0);
            SkASSERT(slope <= SK_Fixed1 && slope >= -SK_Fixed1);
            fstart += (slope * (32 - (y0 & 63)) + 32) >> 6;
            hairBlitter = &vertish_blitter;
        }

        SkASSERT(istop > istart);
        if (istop - istart == 1) {
            // we are within a single pixel
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
                if (istop - istart == 1) {
                    // we are within a single pixel
                    scaleStart = contribution_64(y1);
                    scaleStop = 0;
                }
            }
            if (istop > clip->fBottom) {
                istop = clip->fBottom;
                scaleStop = 0;  // so we don't draw this last row
            }

            SkASSERT(istart <= istop);
            if (istart == istop)
                return;

            // now test if our X values are completely inside the clip
            int left, right;
            if (slope >= 0) { // L2R
                left = SkFixedFloorToInt(fstart - SK_FixedHalf);
                right = SkFixedCeilToInt(fstart + (istop - istart - 1) * slope + SK_FixedHalf);
            } else {           // R2L
                right = SkFixedCeilToInt(fstart + SK_FixedHalf);
                left = SkFixedFloorToInt(fstart + (istop - istart - 1) * slope - SK_FixedHalf);
            }
#ifdef OUTSET_BEFORE_CLIP_TEST
            left -= 1;
            right += 1;
#endif
            if (left >= clip->fRight || right <= clip->fLeft) {
                return;
            }
            if (clip->fLeft <= left && clip->fRight >= right) {
                clip = nullptr;
            }
        }
    }

    SkRectClipBlitter   rectClipper;
    if (clip) {
        rectClipper.init(blitter, *clip);
        blitter = &rectClipper;
    }

    SkASSERT(hairBlitter);
    hairBlitter->setup(blitter);

#ifdef SK_DEBUG
    if (scaleStart > 0 && scaleStop > 0) {
        // be sure we don't draw twice in the same pixel
        SkASSERT(istart < istop - 1);
    }
#endif

    fstart = hairBlitter->drawCap(istart, fstart, slope, scaleStart);
    istart += 1;
    int fullSpans = istop - istart - (scaleStop > 0);
    if (fullSpans > 0) {
        fstart = hairBlitter->drawLine(istart, istart + fullSpans, fstart, slope);
    }
    if (scaleStop > 0) {
        hairBlitter->drawCap(istop - 1, fstart, slope, scaleStop);
    }
}

void SkScan::AntiHairLineRgn(const SkPoint array[], int arrayCount, const SkRegion* clip,
                             SkBlitter* blitter) {
    if (clip && clip->isEmpty()) {
        return;
    }

    SkASSERT(clip == nullptr || !clip->getBounds().isEmpty());

#ifdef TEST_GAMMA
    build_gamma_table();
#endif

    const SkScalar max = SkIntToScalar(32767);
    const SkRect fixedBounds = SkRect::MakeLTRB(-max, -max, max, max);

    SkRect clipBounds;
    if (clip) {
        clipBounds.set(clip->getBounds());
        /*  We perform integral clipping later on, but we do a scalar clip first
         to ensure that our coordinates are expressible in fixed/integers.

         antialiased hairlines can draw up to 1/2 of a pixel outside of
         their bounds, so we need to outset the clip before calling the
         clipper. To make the numerics safer, we outset by a whole pixel,
         since the 1/2 pixel boundary is important to the antihair blitter,
         we don't want to risk numerical fate by chopping on that edge.
         */
        clipBounds.outset(SK_Scalar1, SK_Scalar1);
    }

    for (int i = 0; i < arrayCount - 1; ++i) {
        SkPoint pts[2];

        // We have to pre-clip the line to fit in a SkFixed, so we just chop
        // the line. TODO find a way to actually draw beyond that range.
        if (!SkLineClipper::IntersectLine(&array[i], fixedBounds, pts)) {
            continue;
        }

        if (clip && !SkLineClipper::IntersectLine(pts, clipBounds, pts)) {
            continue;
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
                continue;
            }
            if (!clip->quickContains(ir)) {
                SkRegion::Cliperator iter(*clip, ir);
                const SkIRect*       r = &iter.rect();

                while (!iter.done()) {
                    do_anti_hairline(x0, y0, x1, y1, r, blitter);
                    iter.next();
                }
                continue;
            }
            // fall through to no-clip case
        }
        do_anti_hairline(x0, y0, x1, y1, nullptr, blitter);
    }
}

void SkScan::AntiHairRect(const SkRect& rect, const SkRasterClip& clip,
                          SkBlitter* blitter) {
    SkPoint pts[5];

    pts[0].set(rect.fLeft, rect.fTop);
    pts[1].set(rect.fRight, rect.fTop);
    pts[2].set(rect.fRight, rect.fBottom);
    pts[3].set(rect.fLeft, rect.fBottom);
    pts[4] = pts[0];
    SkScan::AntiHairLine(pts, 5, clip, blitter);
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
    if (nullptr == clip) {
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
            AntiFillXRect(xr, nullptr, blitter);
        } else {
            SkAAClipBlitterWrapper wrapper(clip, blitter);
            blitter = wrapper.getBlitter();

            AntiFillXRect(xr, &wrapper.getRgn(), wrapper.getBlitter());
        }
    }
}

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

        const SkIRect outerBounds = newR.roundOut();

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

///////////////////////////////////////////////////////////////////////////////

#define SkAlphaMulRound(a, b)   SkMulDiv255Round(a, b)

// calls blitRect() if the rectangle is non-empty
static void fillcheckrect(int L, int T, int R, int B, SkBlitter* blitter) {
    if (L < R && T < B) {
        blitter->blitRect(L, T, R - L, B - T);
    }
}

static inline FDot8 SkScalarToFDot8(SkScalar x) {
    return (int)(x * 256);
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
        FDot8 widClamp = R - L;
        // border case clamp 256 to 255 instead of going through call_hline_blitter
        // see skbug/4406
        widClamp = widClamp - (widClamp >> 8);
        blitter->blitV(L >> 8, top, 1, InvAlphaMul(alpha, widClamp));
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
        // We want the inverse of B-T, since we're the inner-stroke
        int alpha = 256 - (B - T);
        if (alpha) {
            inner_scanline(L, top, R, alpha, blitter);
        }
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

static inline void align_thin_stroke(FDot8& edge1, FDot8& edge2) {
    SkASSERT(edge1 <= edge2);

    if (FDot8Floor(edge1) == FDot8Floor(edge2)) {
        edge2 -= (edge1 & 0xFF);
        edge1 &= ~0xFF;
    }
}

void SkScan::AntiFrameRect(const SkRect& r, const SkPoint& strokeSize,
                           const SkRegion* clip, SkBlitter* blitter) {
    SkASSERT(strokeSize.fX >= 0 && strokeSize.fY >= 0);

    SkScalar rx = SkScalarHalf(strokeSize.fX);
    SkScalar ry = SkScalarHalf(strokeSize.fY);

    // outset by the radius
    FDot8 outerL = SkScalarToFDot8(r.fLeft - rx);
    FDot8 outerT = SkScalarToFDot8(r.fTop - ry);
    FDot8 outerR = SkScalarToFDot8(r.fRight + rx);
    FDot8 outerB = SkScalarToFDot8(r.fBottom + ry);

    SkIRect outer;
    // set outer to the outer rect of the outer section
    outer.set(FDot8Floor(outerL), FDot8Floor(outerT), FDot8Ceil(outerR), FDot8Ceil(outerB));

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

    // in case we lost a bit with diameter/2
    rx = strokeSize.fX - rx;
    ry = strokeSize.fY - ry;

    // inset by the radius
    FDot8 innerL = SkScalarToFDot8(r.fLeft + rx);
    FDot8 innerT = SkScalarToFDot8(r.fTop + ry);
    FDot8 innerR = SkScalarToFDot8(r.fRight - rx);
    FDot8 innerB = SkScalarToFDot8(r.fBottom - ry);

    // For sub-unit strokes, tweak the hulls such that one of the edges coincides with the pixel
    // edge. This ensures that the general rect stroking logic below
    //   a) doesn't blit the same scanline twice
    //   b) computes the correct coverage when both edges fall within the same pixel
    if (strokeSize.fX < 1 || strokeSize.fY < 1) {
        align_thin_stroke(outerL, innerL);
        align_thin_stroke(outerT, innerT);
        align_thin_stroke(innerR, outerR);
        align_thin_stroke(innerB, outerB);
    }

    // stroke the outer hull
    antifilldot8(outerL, outerT, outerR, outerB, blitter, false);

    // set outer to the outer rect of the middle section
    outer.set(FDot8Ceil(outerL), FDot8Ceil(outerT), FDot8Floor(outerR), FDot8Floor(outerB));

    if (innerL >= innerR || innerT >= innerB) {
        fillcheckrect(outer.fLeft, outer.fTop, outer.fRight, outer.fBottom,
                      blitter);
    } else {
        SkIRect inner;
        // set inner to the inner rect of the middle section
        inner.set(FDot8Floor(innerL), FDot8Floor(innerT), FDot8Ceil(innerR), FDot8Ceil(innerB));

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
        innerstrokedot8(innerL, innerT, innerR, innerB, blitter);
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
