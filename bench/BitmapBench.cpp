/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkPaint.h"
#include "include/core/SkString.h"
#include "include/utils/SkRandom.h"
#include "tools/ToolUtils.h"

/*  Variants for bitmaps

    - src depth (32 w+w/o alpha), 565, 4444, index, a8
    - paint options: filtering, dither, alpha
    - matrix options: translate, scale, rotate, persp
    - tiling: none, repeat, mirror, clamp

 */

class BitmapBench : public Benchmark {
    const SkColorType   fColorType;
    const SkAlphaType   fAlphaType;
    const bool          fForceUpdate; //bitmap marked as dirty before each draw. forces bitmap to be updated on device cache
    const bool          fIsVolatile;
    const bool          fDoScale;

    SkBitmap            fBitmap;
    SkPaint             fPaint;
    SkString            fName;

    enum { W = 128 };
    enum { H = 128 };
public:
    BitmapBench(SkColorType ct, SkAlphaType at, bool forceUpdate, bool isVolatile, bool doScale)
        : fColorType(ct)
        , fAlphaType(at)
        , fForceUpdate(forceUpdate)
        , fIsVolatile(isVolatile)
        , fDoScale(doScale)
    {}

protected:
    const char* onGetName() override {
        fName.set("bitmap");
        fName.appendf("_%s%s",
                      ToolUtils::colortype_name(fColorType),
                      kOpaque_SkAlphaType == fAlphaType ? "" : "_A");
        if (fDoScale) {
            fName.append("_scale");
        }
        if (fForceUpdate) {
            fName.append("_update");
        }
        if (fIsVolatile) {
            fName.append("_volatile");
        }

        return fName.c_str();
    }

    void onDelayedSetup() override {
        SkBitmap bm;

        bm.allocPixels(SkImageInfo::Make(W, H, fColorType, fAlphaType));
        bm.eraseColor(kOpaque_SkAlphaType == fAlphaType ? SK_ColorBLACK : 0);

        this->onDrawIntoBitmap(bm);

        fBitmap = bm;
        fBitmap.setIsVolatile(fIsVolatile);
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        if (fDoScale) {
            canvas->scale(.99f, .99f);
        }
        SkIPoint dim = this->getSize();
        SkRandom rand;

        SkPaint paint(fPaint);
        this->setupPaint(&paint);

        const SkBitmap& bitmap = fBitmap;
        const SkScalar x0 = SkIntToScalar(-bitmap.width() / 2);
        const SkScalar y0 = SkIntToScalar(-bitmap.height() / 2);

        for (int i = 0; i < loops; i++) {
            SkScalar x = x0 + rand.nextUScalar1() * dim.fX;
            SkScalar y = y0 + rand.nextUScalar1() * dim.fY;

            if (fForceUpdate)
                bitmap.notifyPixelsChanged();

            canvas->drawBitmap(bitmap, x, y, &paint);
        }
    }

     virtual void onDrawIntoBitmap(const SkBitmap& bm) {
        const int w = bm.width();
        const int h = bm.height();

        SkCanvas canvas(bm);
        SkPaint p;
        p.setAntiAlias(true);
        p.setColor(SK_ColorRED);
        canvas.drawCircle(SkIntToScalar(w)/2, SkIntToScalar(h)/2,
                          SkIntToScalar(SkMin32(w, h))*3/8, p);

        SkRect r;
        r.setWH(SkIntToScalar(w), SkIntToScalar(h));
        p.setStyle(SkPaint::kStroke_Style);
        p.setStrokeWidth(SkIntToScalar(4));
        p.setColor(SK_ColorBLUE);
        canvas.drawRect(r, p);
    }

private:
    typedef Benchmark INHERITED;
};

/** Explicitly invoke some filter types to improve coverage of acceleration
    procs. */

enum Flags {
    kScale_Flag             = 1 << 0,
    kRotate_Flag            = 1 << 1,
    kBilerp_Flag            = 1 << 2,
    kBicubic_Flag           = 1 << 3,
};

static bool isBilerp(uint32_t flags) {
    return (flags & (kBilerp_Flag | kBicubic_Flag)) == (kBilerp_Flag);
}

static bool isBicubic(uint32_t flags) {
    return (flags & (kBilerp_Flag | kBicubic_Flag)) == (kBilerp_Flag | kBicubic_Flag);
}

class FilterBitmapBench : public BitmapBench {
    uint32_t    fFlags;
    SkString    fFullName;
public:
    FilterBitmapBench(SkColorType ct, SkAlphaType at,
                      bool forceUpdate, bool isVolitile, uint32_t flags)
        : INHERITED(ct, at, forceUpdate, isVolitile, false)
        , fFlags(flags) {
    }

protected:
    const char* onGetName() override {
        fFullName.set(INHERITED::onGetName());
        if (fFlags & kScale_Flag) {
            fFullName.append("_scale");
        }
        if (fFlags & kRotate_Flag) {
            fFullName.append("_rotate");
        }
        if (isBilerp(fFlags)) {
            fFullName.append("_bilerp");
        } else if (isBicubic(fFlags)) {
            fFullName.append("_bicubic");
        }

        return fFullName.c_str();
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkISize dim = canvas->getBaseLayerSize();
        if (fFlags & kScale_Flag) {
            const SkScalar x = SkIntToScalar(dim.fWidth) / 2;
            const SkScalar y = SkIntToScalar(dim.fHeight) / 2;

            canvas->translate(x, y);
            // just enough so we can't take the sprite case
            canvas->scale(1.1f, 1.1f);
            canvas->translate(-x, -y);
        }
        if (fFlags & kRotate_Flag) {
            const SkScalar x = SkIntToScalar(dim.fWidth) / 2;
            const SkScalar y = SkIntToScalar(dim.fHeight) / 2;
            canvas->rotate(SkIntToScalar(35), x, y);
        }
        INHERITED::onDraw(loops, canvas);
    }

    void setupPaint(SkPaint* paint) override {
        this->INHERITED::setupPaint(paint);

        int index = 0;
        if (fFlags & kBilerp_Flag) {
            index |= 1;
        }
        if (fFlags & kBicubic_Flag) {
            index |= 2;
        }
        static const SkFilterQuality gQualitys[] = {
            kNone_SkFilterQuality,
            kLow_SkFilterQuality,
            kMedium_SkFilterQuality,
            kHigh_SkFilterQuality
        };
        paint->setFilterQuality(gQualitys[index]);
}

private:
    typedef BitmapBench INHERITED;
};

/** Verify optimizations that test source alpha values. */

class SourceAlphaBitmapBench : public BitmapBench {
public:
    enum SourceAlpha { kOpaque_SourceAlpha, kTransparent_SourceAlpha,
                       kTwoStripes_SourceAlpha, kThreeStripes_SourceAlpha};
private:
    SkString    fFullName;
    SourceAlpha fSourceAlpha;
public:
    SourceAlphaBitmapBench(SourceAlpha alpha, SkColorType ct,
                bool forceUpdate = false, bool bitmapVolatile = false)
        : INHERITED(ct, kPremul_SkAlphaType, forceUpdate, bitmapVolatile, false)
        , fSourceAlpha(alpha) {
    }

protected:
    const char* onGetName() override {
        fFullName.set(INHERITED::onGetName());

        if (fSourceAlpha == kOpaque_SourceAlpha) {
                fFullName.append("_source_opaque");
        } else if (fSourceAlpha == kTransparent_SourceAlpha) {
                fFullName.append("_source_transparent");
        } else if (fSourceAlpha == kTwoStripes_SourceAlpha) {
                fFullName.append("_source_stripes_two");
        } else if (fSourceAlpha == kThreeStripes_SourceAlpha) {
                fFullName.append("_source_stripes_three");
        }

        return fFullName.c_str();
    }

    void onDrawIntoBitmap(const SkBitmap& bm) override {
        const int w = bm.width();
        const int h = bm.height();

        if (kOpaque_SourceAlpha == fSourceAlpha) {
            bm.eraseColor(SK_ColorBLACK);
        } else if (kTransparent_SourceAlpha == fSourceAlpha) {
            bm.eraseColor(0);
        } else if (kTwoStripes_SourceAlpha == fSourceAlpha) {
            bm.eraseColor(0);

            SkCanvas canvas(bm);
            SkPaint p;
            p.setAntiAlias(false);
            p.setStyle(SkPaint::kFill_Style);
            p.setColor(SK_ColorRED);

            // Draw red vertical stripes on transparent background
            SkRect r;
            for (int x = 0; x < w; x+=2)
            {
                r.setLTRB(SkIntToScalar(x), 0, SkIntToScalar(x+1), SkIntToScalar(h));
                canvas.drawRect(r, p);
            }

        } else if (kThreeStripes_SourceAlpha == fSourceAlpha) {
            bm.eraseColor(0);

            SkCanvas canvas(bm);
            SkPaint p;
            p.setAntiAlias(false);
            p.setStyle(SkPaint::kFill_Style);

            // Draw vertical stripes on transparent background with a pattern
            // where the first pixel is fully transparent, the next is semi-transparent
            // and the third is fully opaque.
            SkRect r;
            for (int x = 0; x < w; x++)
            {
                if (x % 3 == 0) {
                    continue; // Keep transparent
                } else if (x % 3 == 1) {
                    p.setColor(SkColorSetARGB(127, 127, 127, 127)); // Semi-transparent
                } else if (x % 3 == 2) {
                    p.setColor(SK_ColorRED); // Opaque
                }
                r.setLTRB(SkIntToScalar(x), 0, SkIntToScalar(x+1), SkIntToScalar(h));
                canvas.drawRect(r, p);
            }
        }
    }

private:
    typedef BitmapBench INHERITED;
};

DEF_BENCH( return new BitmapBench(kN32_SkColorType, kPremul_SkAlphaType, false, false, false); )
DEF_BENCH( return new BitmapBench(kN32_SkColorType, kOpaque_SkAlphaType, false, false, false); )
DEF_BENCH( return new BitmapBench(kN32_SkColorType, kOpaque_SkAlphaType, false, false, true); )
DEF_BENCH( return new BitmapBench(kRGB_565_SkColorType, kOpaque_SkAlphaType, false, false, false); )
DEF_BENCH( return new BitmapBench(kN32_SkColorType, kOpaque_SkAlphaType, true, true, false); )
DEF_BENCH( return new BitmapBench(kN32_SkColorType, kOpaque_SkAlphaType, true, false, false); )

// scale filter -> S32_opaque_D32_filter_DX_{SSE2,SSSE3} and Fact9 is also for S32_D16_filter_DX_SSE2
DEF_BENCH( return new FilterBitmapBench(kN32_SkColorType, kPremul_SkAlphaType, false, false, kScale_Flag | kBilerp_Flag); )
DEF_BENCH( return new FilterBitmapBench(kN32_SkColorType, kOpaque_SkAlphaType, false, false, kScale_Flag | kBilerp_Flag); )
DEF_BENCH( return new FilterBitmapBench(kN32_SkColorType, kOpaque_SkAlphaType, true, true, kScale_Flag | kBilerp_Flag); )
DEF_BENCH( return new FilterBitmapBench(kN32_SkColorType, kOpaque_SkAlphaType, true, false, kScale_Flag | kBilerp_Flag); )

// The following two cases test the performance regression of b/70172912 .
DEF_BENCH( return new FilterBitmapBench(kRGB_565_SkColorType, kOpaque_SkAlphaType, false, false, kScale_Flag | kBilerp_Flag); )
DEF_BENCH( return new BitmapBench(kRGB_565_SkColorType, kOpaque_SkAlphaType, false, false, true); )

// scale rotate filter -> S32_opaque_D32_filter_DXDY_{SSE2,SSSE3}
DEF_BENCH( return new FilterBitmapBench(kN32_SkColorType, kPremul_SkAlphaType, false, false, kScale_Flag | kRotate_Flag | kBilerp_Flag); )
DEF_BENCH( return new FilterBitmapBench(kN32_SkColorType, kOpaque_SkAlphaType, false, false, kScale_Flag | kRotate_Flag | kBilerp_Flag); )
DEF_BENCH( return new FilterBitmapBench(kN32_SkColorType, kOpaque_SkAlphaType, true, true, kScale_Flag | kRotate_Flag | kBilerp_Flag); )
DEF_BENCH( return new FilterBitmapBench(kN32_SkColorType, kOpaque_SkAlphaType, true, false, kScale_Flag | kRotate_Flag | kBilerp_Flag); )

DEF_BENCH( return new FilterBitmapBench(kN32_SkColorType, kPremul_SkAlphaType, false, false, kScale_Flag | kBilerp_Flag | kBicubic_Flag); )
DEF_BENCH( return new FilterBitmapBench(kN32_SkColorType, kPremul_SkAlphaType, false, false, kScale_Flag | kRotate_Flag | kBilerp_Flag | kBicubic_Flag); )

// source alpha tests -> S32A_Opaque_BlitRow32_{arm,neon}
DEF_BENCH( return new SourceAlphaBitmapBench(SourceAlphaBitmapBench::kOpaque_SourceAlpha, kN32_SkColorType); )
DEF_BENCH( return new SourceAlphaBitmapBench(SourceAlphaBitmapBench::kTransparent_SourceAlpha, kN32_SkColorType); )
DEF_BENCH( return new SourceAlphaBitmapBench(SourceAlphaBitmapBench::kTwoStripes_SourceAlpha, kN32_SkColorType); )
DEF_BENCH( return new SourceAlphaBitmapBench(SourceAlphaBitmapBench::kThreeStripes_SourceAlpha, kN32_SkColorType); )
