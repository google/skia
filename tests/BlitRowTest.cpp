#include "Test.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkRect.h"

static inline const char* boolStr(bool value) {
    return value ? "true" : "false";
}

// these are in the same order as the SkBitmap::Config enum
static const char* gConfigName[] = {
    "None", "A1", "A8", "Index8", "565", "4444", "8888", "RLE_Index8"
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

static int proc_bad(const void* ptr, int, uint32_t, uint32_t* bad) {
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

static void TestBlitRow(skiatest::Reporter* reporter) {
    static const int W = 256;

    static const SkBitmap::Config gDstConfig[] = {
        SkBitmap::kARGB_8888_Config,
        SkBitmap::kRGB_565_Config,
        SkBitmap::kARGB_4444_Config,
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
    paint.setDither(true);

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
                    continue;
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

#include "TestClassDef.h"
DEFINE_TESTCLASS("BlitRow", TestBlitRowClass, TestBlitRow)
