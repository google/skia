/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkGradientShader.h"
#include "SkRect.h"

// these are in the same order as the SkBitmap::Config enum
static const char* gConfigName[] = {
    "None", "A8", "Index8", "565", "4444", "8888"
};

/** Returns -1 on success, else the x coord of the first bad pixel, return its
    value in bad
 */
typedef int (*Proc)(const void*, int width, uint32_t expected, uint32_t* bad);

static int proc_32(const void* ptr, int w, uint32_t expected, uint32_t* bad) {
    const SkPMColor* addr = static_cast<const SkPMColor*>(ptr);
    for (int x = 0; x < w; x++) {
        if (addr[x] != expected) {
            *bad = addr[x];
            return x;
        }
    }
    return -1;
}

static int proc_16(const void* ptr, int w, uint32_t expected, uint32_t* bad) {
    const uint16_t* addr = static_cast<const uint16_t*>(ptr);
    for (int x = 0; x < w; x++) {
        if (addr[x] != expected) {
            *bad = addr[x];
            return x;
        }
    }
    return -1;
}

static int proc_8(const void* ptr, int w, uint32_t expected, uint32_t* bad) {
    const SkPMColor* addr = static_cast<const SkPMColor*>(ptr);
    for (int x = 0; x < w; x++) {
        if (SkGetPackedA32(addr[x]) != expected) {
            *bad = SkGetPackedA32(addr[x]);
            return x;
        }
    }
    return -1;
}

static int proc_bad(const void*, int, uint32_t, uint32_t* bad) {
    *bad = 0;
    return 0;
}

static Proc find_proc(const SkBitmap& bm, SkPMColor expect32, uint16_t expect16,
                      uint8_t expect8, uint32_t* expect) {
    switch (bm.config()) {
        case SkBitmap::kARGB_8888_Config:
            *expect = expect32;
            return proc_32;
        case SkBitmap::kARGB_4444_Config:
        case SkBitmap::kRGB_565_Config:
            *expect = expect16;
            return proc_16;
        case SkBitmap::kA8_Config:
            *expect = expect8;
            return proc_8;
        default:
            *expect = 0;
            return proc_bad;
    }
}

static bool check_color(const SkBitmap& bm, SkPMColor expect32,
                        uint16_t expect16, uint8_t expect8,
                        skiatest::Reporter* reporter) {
    uint32_t expect;
    Proc proc = find_proc(bm, expect32, expect16, expect8, &expect);
    for (int y = 0; y < bm.height(); y++) {
        uint32_t bad;
        int x = proc(bm.getAddr(0, y), bm.width(), expect, &bad);
        if (x >= 0) {
            SkString str;
            str.printf("BlitRow config=%s [%d %d] expected %x got %x",
                       gConfigName[bm.config()], x, y, expect, bad);
            reporter->reportFailed(str);
            return false;
        }
    }
    return true;
}

// Make sure our blits always map src==0 to a noop, and src==FF to full opaque
static void test_00_FF(skiatest::Reporter* reporter) {
    static const int W = 256;

    static const SkBitmap::Config gDstConfig[] = {
        SkBitmap::kARGB_8888_Config,
        SkBitmap::kRGB_565_Config,
//        SkBitmap::kARGB_4444_Config,
//        SkBitmap::kA8_Config,
    };

    static const struct {
        SkColor     fSrc;
        SkColor     fDst;
        SkPMColor   fResult32;
        uint16_t    fResult16;
        uint8_t     fResult8;
    } gSrcRec[] = {
        { 0,            0,          0,                                    0,      0 },
        { 0,            0xFFFFFFFF, SkPackARGB32(0xFF, 0xFF, 0xFF, 0xFF), 0xFFFF, 0xFF },
        { 0xFFFFFFFF,   0,          SkPackARGB32(0xFF, 0xFF, 0xFF, 0xFF), 0xFFFF, 0xFF },
        { 0xFFFFFFFF,   0xFFFFFFFF, SkPackARGB32(0xFF, 0xFF, 0xFF, 0xFF), 0xFFFF, 0xFF },
    };

    SkPaint paint;

    SkBitmap srcBM;
    srcBM.setConfig(SkBitmap::kARGB_8888_Config, W, 1);
    srcBM.allocPixels();

    for (size_t i = 0; i < SK_ARRAY_COUNT(gDstConfig); i++) {
        SkBitmap dstBM;
        dstBM.setConfig(gDstConfig[i], W, 1);
        dstBM.allocPixels();

        SkCanvas canvas(dstBM);
        for (size_t j = 0; j < SK_ARRAY_COUNT(gSrcRec); j++) {
            srcBM.eraseColor(gSrcRec[j].fSrc);
            dstBM.eraseColor(gSrcRec[j].fDst);

            for (int k = 0; k < 4; k++) {
                bool dither = (k & 1) != 0;
                bool blend = (k & 2) != 0;
                if (gSrcRec[j].fSrc != 0 && blend) {
                    // can't make a numerical promise about blending anything
                    // but 0
                 //   continue;
                }
                paint.setDither(dither);
                paint.setAlpha(blend ? 0x80 : 0xFF);
                canvas.drawBitmap(srcBM, 0, 0, &paint);
                if (!check_color(dstBM, gSrcRec[j].fResult32, gSrcRec[j].fResult16,
                                 gSrcRec[j].fResult8, reporter)) {
                    SkDebugf("--- src index %d dither %d blend %d\n", j, dither, blend);
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

struct Mesh {
    SkPoint     fPts[4];

    Mesh(const SkBitmap& bm, SkPaint* paint) {
        const SkScalar w = SkIntToScalar(bm.width());
        const SkScalar h = SkIntToScalar(bm.height());
        fPts[0].set(0, 0);
        fPts[1].set(w, 0);
        fPts[2].set(w, h);
        fPts[3].set(0, h);
        SkShader* s = SkShader::CreateBitmapShader(bm, SkShader::kClamp_TileMode,
                                                   SkShader::kClamp_TileMode);
        paint->setShader(s)->unref();

    }

    void draw(SkCanvas* canvas, SkPaint* paint) {
        canvas->drawVertices(SkCanvas::kTriangleFan_VertexMode, 4, fPts, fPts,
                             NULL, NULL, NULL, 0, *paint);
    }
};

#include "SkImageEncoder.h"
static void save_bm(const SkBitmap& bm, const char name[]) {
    SkImageEncoder::EncodeFile(name, bm, SkImageEncoder::kPNG_Type, 100);
}

static bool gOnce;

// Make sure our blits are invariant with the width of the blit (i.e. that
// special case for 8 at a time have the same results as narrower blits)
static void test_diagonal(skiatest::Reporter* reporter) {
    static const int W = 64;
    static const int H = W;

    static const SkBitmap::Config gDstConfig[] = {
        SkBitmap::kARGB_8888_Config,
        SkBitmap::kRGB_565_Config,
        //        SkBitmap::kARGB_4444_Config,
        //        SkBitmap::kA8_Config,
    };

    static const SkColor gDstBG[] = { 0, 0xFFFFFFFF };

    SkPaint paint;

    SkBitmap srcBM;
    srcBM.setConfig(SkBitmap::kARGB_8888_Config, W, H);
    srcBM.allocPixels();
    SkRect srcR = {
        0, 0, SkIntToScalar(srcBM.width()), SkIntToScalar(srcBM.height()) };

    // cons up a mesh to draw the bitmap with
    Mesh mesh(srcBM, &paint);

    for (size_t i = 0; i < SK_ARRAY_COUNT(gDstConfig); i++) {
        SkBitmap dstBM0, dstBM1;
        dstBM0.setConfig(gDstConfig[i], W, H);
        dstBM1.setConfig(gDstConfig[i], W, H);
        dstBM0.allocPixels();
        dstBM1.allocPixels();

        SkCanvas canvas0(dstBM0);
        SkCanvas canvas1(dstBM1);
        SkColor bgColor;

        for (size_t j = 0; j < SK_ARRAY_COUNT(gDstBG); j++) {
            bgColor = gDstBG[j];

            for (int c = 0; c <= 0xFF; c++) {
                srcBM.eraseARGB(0xFF, c, c, c);

                for (int k = 0; k < 4; k++) {
                    bool dither = (k & 1) != 0;
                    uint8_t alpha = (k & 2) ? 0x80 : 0xFF;
                    paint.setDither(dither);
                    paint.setAlpha(alpha);

                    dstBM0.eraseColor(bgColor);
                    dstBM1.eraseColor(bgColor);

                    canvas0.drawRect(srcR, paint);
                    mesh.draw(&canvas1, &paint);

                    if (!gOnce && false) {
                        save_bm(dstBM0, "drawBitmap.png");
                        save_bm(dstBM1, "drawMesh.png");
                        gOnce = true;
                    }

                    if (memcmp(dstBM0.getPixels(), dstBM1.getPixels(), dstBM0.getSize())) {
                        SkString str;
                        str.printf("Diagonal config=%s bg=0x%x dither=%d alpha=0x%x src=0x%x",
                                   gConfigName[gDstConfig[i]], bgColor, dither, alpha, c);
                        reporter->reportFailed(str);
                    }
                }
            }
        }
    }
}

DEF_TEST(BlitRow, reporter) {
    test_00_FF(reporter);
    test_diagonal(reporter);
}
