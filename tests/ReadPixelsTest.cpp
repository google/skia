
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkCanvas.h"
#include "SkRegion.h"
#include "SkGpuDevice.h"


static const int DEV_W = 100, DEV_H = 100;
static const SkIRect DEV_RECT = SkIRect::MakeWH(DEV_W, DEV_H);
static const SkRect DEV_RECT_S = SkRect::MakeWH(DEV_W * SK_Scalar1, 
                                                DEV_H * SK_Scalar1);

namespace {
SkPMColor getCanvasColor(int x, int y) {
    SkASSERT(x >= 0 && x < DEV_W);
    SkASSERT(y >= 0 && y < DEV_H);

    U8CPU r = x;
    U8CPU g = y;
    U8CPU b = 0xc;

    U8CPU a = 0xff;
    switch (x % 5) {
        case 0:
            a = 0xff;
            break;
        case 1:
            a = 0x80;
            break;
        case 2:
            a = 0xCC;
            break;
        case 4:
            a = 0x01;
            break;
        case 3:
            a = 0x00;
            break;
    }
    return SkPremultiplyARGBInline(a, r, g, b);
}
    
SkPMColor getBitmapColor(int x, int y, int w, int h) {
    int n = y * w + x;

    U8CPU b = n & 0xff;
    U8CPU g = (n >> 8) & 0xff;
    U8CPU r = (n >> 16) & 0xff;
    return SkPackARGB32(0xff, r, g , b);
}

SkPMColor convertConfig8888ToPMColor(SkCanvas::Config8888 config8888,
                                     uint32_t color) {
    const uint8_t* c = reinterpret_cast<uint8_t*>(&color);
    U8CPU a,r,g,b;
    bool mul = false;
    switch (config8888) {
        case SkCanvas::kNative_Premul_Config8888:
            return color;
        case SkCanvas::kNative_Unpremul_Config8888:
            mul = true;
            a = SkGetPackedA32(color);
            r = SkGetPackedR32(color);
            g = SkGetPackedG32(color);
            b = SkGetPackedB32(color);
            break;
        case SkCanvas::kBGRA_Unpremul_Config8888:
            mul = true; // fallthru
        case SkCanvas::kBGRA_Premul_Config8888:
            a = static_cast<U8CPU>(c[3]);
            r = static_cast<U8CPU>(c[2]);
            g = static_cast<U8CPU>(c[1]);
            b = static_cast<U8CPU>(c[0]);
            break;
        case SkCanvas::kRGBA_Unpremul_Config8888:
            mul = true; // fallthru
        case SkCanvas::kRGBA_Premul_Config8888:
            a = static_cast<U8CPU>(c[3]);
            r = static_cast<U8CPU>(c[0]);
            g = static_cast<U8CPU>(c[1]);
            b = static_cast<U8CPU>(c[2]);
            break;
    }
    if (mul) {
        r = SkMulDiv255Ceiling(r, a);
        g = SkMulDiv255Ceiling(g, a);
        b = SkMulDiv255Ceiling(b, a);
    }
    return SkPackARGB32(a, r, g, b);
}

void fillCanvas(SkCanvas* canvas) {
    static SkBitmap bmp;
    if (bmp.isNull()) {
        bmp.setConfig(SkBitmap::kARGB_8888_Config, DEV_W, DEV_H);
        bool alloc = bmp.allocPixels();
        SkASSERT(alloc);
        SkAutoLockPixels alp(bmp);
        intptr_t pixels = reinterpret_cast<intptr_t>(bmp.getPixels());
        for (int y = 0; y < DEV_H; ++y) {
            for (int x = 0; x < DEV_W; ++x) {
                SkPMColor* pixel = reinterpret_cast<SkPMColor*>(pixels + y * bmp.rowBytes() + x * bmp.bytesPerPixel());
                *pixel = getCanvasColor(x, y);
            }
        }
    }
    canvas->save();
    canvas->setMatrix(SkMatrix::I());
    canvas->clipRect(DEV_RECT_S, SkRegion::kReplace_Op);
    SkPaint paint;
    paint.setXfermodeMode(SkXfermode::kSrc_Mode);
    canvas->drawBitmap(bmp, 0, 0, &paint);
    canvas->restore();
}
    
void fillBitmap(SkBitmap* bitmap) {
    SkASSERT(bitmap->lockPixelsAreWritable());
    SkAutoLockPixels alp(*bitmap);
    int w = bitmap->width();
    int h = bitmap->height();
    intptr_t pixels = reinterpret_cast<intptr_t>(bitmap->getPixels());
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            SkPMColor* pixel = reinterpret_cast<SkPMColor*>(pixels + y * bitmap->rowBytes() + x * bitmap->bytesPerPixel());
            *pixel = getBitmapColor(x, y, w, h);
        }
    }
}

// checks the bitmap contains correct pixels after the readPixels
// if the bitmap was prefilled with pixels it checks that these weren't
// overwritten in the area outside the readPixels.
bool checkRead(skiatest::Reporter* reporter,
               const SkBitmap& bitmap,
               int x, int y,
               bool checkCanvasPixels,
               bool checkBitmapPixels,
               SkCanvas::Config8888 config8888) {
    SkASSERT(SkBitmap::kARGB_8888_Config == bitmap.config());
    SkASSERT(!bitmap.isNull());
    SkASSERT(checkCanvasPixels || checkBitmapPixels);
    
    int bw = bitmap.width();
    int bh = bitmap.height();

    SkIRect srcRect = SkIRect::MakeXYWH(x, y, bw, bh);
    SkIRect clippedSrcRect = DEV_RECT;
    if (!clippedSrcRect.intersect(srcRect)) {
        clippedSrcRect.setEmpty();
    }

    SkAutoLockPixels alp(bitmap);
    intptr_t pixels = reinterpret_cast<intptr_t>(bitmap.getPixels());
    for (int by = 0; by < bh; ++by) {
        for (int bx = 0; bx < bw; ++bx) {
            int devx = bx + srcRect.fLeft;
            int devy = by + srcRect.fTop;
            
            SkPMColor pixel = *reinterpret_cast<SkPMColor*>(pixels + by * bitmap.rowBytes() + bx * bitmap.bytesPerPixel());

            if (clippedSrcRect.contains(devx, devy)) {
                if (checkCanvasPixels) {
                    SkPMColor canvasPixel = getCanvasColor(devx, devy);
                    pixel = convertConfig8888ToPMColor(config8888, pixel);
                    REPORTER_ASSERT(reporter, canvasPixel == pixel);
                    if (getCanvasColor(devx, devy) != pixel) {
                        return false;
                    }
                }
            } else if (checkBitmapPixels) {
                REPORTER_ASSERT(reporter, getBitmapColor(bx, by, bw, bh) == pixel);
                if (getBitmapColor(bx, by, bw, bh) != pixel) {
                    return false;
                }
            }
        }
    }
    return true;
}

enum BitmapInit {
    kFirstBitmapInit = 0,
    
    kNoPixels_BitmapInit = kFirstBitmapInit,
    kTight_BitmapInit,
    kRowBytes_BitmapInit,
    
    kBitmapInitCnt
};

BitmapInit nextBMI(BitmapInit bmi) {
    int x = bmi;
    return static_cast<BitmapInit>(++x);
}


void init_bitmap(SkBitmap* bitmap, const SkIRect& rect, BitmapInit init) {
    int w = rect.width();
    int h = rect.height();
    int rowBytes = 0;
    bool alloc = true;
    switch (init) {
        case kNoPixels_BitmapInit:
            alloc = false;
        case kTight_BitmapInit:
            break;
        case kRowBytes_BitmapInit:
            rowBytes = w * sizeof(SkPMColor) + 16 * sizeof(SkPMColor);
            break;
        default:
            SkASSERT(0);
            break;
    }
    bitmap->setConfig(SkBitmap::kARGB_8888_Config, w, h, rowBytes);
    if (alloc) {
        bitmap->allocPixels();
    }
}

void ReadPixelsTest(skiatest::Reporter* reporter, GrContext* context) {
    SkCanvas canvas;
    
    const SkIRect testRects[] = {
        // entire thing
        DEV_RECT,
        // larger on all sides
        SkIRect::MakeLTRB(-10, -10, DEV_W + 10, DEV_H + 10),
        // fully contained
        SkIRect::MakeLTRB(DEV_W / 4, DEV_H / 4, 3 * DEV_W / 4, 3 * DEV_H / 4),
        // outside top left
        SkIRect::MakeLTRB(-10, -10, -1, -1),
        // touching top left corner
        SkIRect::MakeLTRB(-10, -10, 0, 0),
        // overlapping top left corner
        SkIRect::MakeLTRB(-10, -10, DEV_W / 4, DEV_H / 4),
        // overlapping top left and top right corners
        SkIRect::MakeLTRB(-10, -10, DEV_W  + 10, DEV_H / 4),
        // touching entire top edge
        SkIRect::MakeLTRB(-10, -10, DEV_W  + 10, 0),
        // overlapping top right corner
        SkIRect::MakeLTRB(3 * DEV_W / 4, -10, DEV_W  + 10, DEV_H / 4),
        // contained in x, overlapping top edge
        SkIRect::MakeLTRB(DEV_W / 4, -10, 3 * DEV_W  / 4, DEV_H / 4),
        // outside top right corner
        SkIRect::MakeLTRB(DEV_W + 1, -10, DEV_W + 10, -1),
        // touching top right corner
        SkIRect::MakeLTRB(DEV_W, -10, DEV_W + 10, 0),
        // overlapping top left and bottom left corners
        SkIRect::MakeLTRB(-10, -10, DEV_W / 4, DEV_H + 10),
        // touching entire left edge
        SkIRect::MakeLTRB(-10, -10, 0, DEV_H + 10),
        // overlapping bottom left corner
        SkIRect::MakeLTRB(-10, 3 * DEV_H / 4, DEV_W / 4, DEV_H + 10),
        // contained in y, overlapping left edge
        SkIRect::MakeLTRB(-10, DEV_H / 4, DEV_W / 4, 3 * DEV_H / 4),
        // outside bottom left corner
        SkIRect::MakeLTRB(-10, DEV_H + 1, -1, DEV_H + 10),
        // touching bottom left corner
        SkIRect::MakeLTRB(-10, DEV_H, 0, DEV_H + 10),
        // overlapping bottom left and bottom right corners
        SkIRect::MakeLTRB(-10, 3 * DEV_H / 4, DEV_W + 10, DEV_H + 10),
        // touching entire left edge
        SkIRect::MakeLTRB(0, DEV_H, DEV_W, DEV_H + 10),
        // overlapping bottom right corner
        SkIRect::MakeLTRB(3 * DEV_W / 4, 3 * DEV_H / 4, DEV_W + 10, DEV_H + 10),
        // overlapping top right and bottom right corners
        SkIRect::MakeLTRB(3 * DEV_W / 4, -10, DEV_W + 10, DEV_H + 10),
    };

    for (int dtype = 0; dtype < 2; ++dtype) {

        if (0 == dtype) {
            canvas.setDevice(new SkDevice(SkBitmap::kARGB_8888_Config,
                                          DEV_W,
                                          DEV_H,
                                          false))->unref();
        } else {
#if SK_SCALAR_IS_FIXED
            // GPU device known not to work in the fixed pt build.
            continue;
#endif
            canvas.setDevice(new SkGpuDevice(context,
                                             SkBitmap::kARGB_8888_Config,
                                             DEV_W,
                                             DEV_H))->unref();
        }
        fillCanvas(&canvas);

        static const SkCanvas::Config8888 gReadConfigs[] = {
            SkCanvas::kNative_Premul_Config8888,
            SkCanvas::kNative_Unpremul_Config8888,
            SkCanvas::kBGRA_Premul_Config8888,
            SkCanvas::kBGRA_Unpremul_Config8888,
            SkCanvas::kRGBA_Premul_Config8888,
            SkCanvas::kRGBA_Unpremul_Config8888,
        };
        for (int rect = 0; rect < SK_ARRAY_COUNT(testRects); ++rect) {
            const SkIRect& srcRect = testRects[rect];
            for (BitmapInit bmi = kFirstBitmapInit;
                 bmi < kBitmapInitCnt;
                 bmi = nextBMI(bmi)) {
                for (int c = 0; c < SK_ARRAY_COUNT(gReadConfigs); ++c) {
                    SkCanvas::Config8888 config8888 = gReadConfigs[c];
                    SkBitmap bmp;
                    init_bitmap(&bmp, srcRect, bmi);

                    // if the bitmap has pixels allocated before the readPixels,
                    // note that and fill them with pattern
                    bool startsWithPixels = !bmp.isNull();
                    if (startsWithPixels) {
                        fillBitmap(&bmp);
                    }

                    bool success =
                        canvas.readPixels(&bmp, srcRect.fLeft,
                                          srcRect.fTop, config8888);

                    // non-native not implemented on GPU yet
                    bool expectSuccess =
                        SkIRect::Intersects(srcRect, DEV_RECT) &&
                        !(1 == dtype &&
                          config8888 != SkCanvas::kNative_Premul_Config8888);

                    // determine whether we expected the read to succeed.
                    REPORTER_ASSERT(reporter, success == expectSuccess);

                    if (success || startsWithPixels) {
                        checkRead(reporter, bmp, srcRect.fLeft, srcRect.fTop,
                                  success, startsWithPixels, config8888);
                    } else {
                        // if we had no pixels beforehand and the readPixels
                        // failed then our bitmap should still not have pixels
                        REPORTER_ASSERT(reporter, bmp.isNull());
                    }
                }
                // check the old webkit version of readPixels that clips the
                // bitmap size
                SkBitmap wkbmp;
                bool success = canvas.readPixels(srcRect, &wkbmp);
                SkIRect clippedRect = DEV_RECT;
                if (clippedRect.intersect(srcRect)) {
                    REPORTER_ASSERT(reporter, success);
                    checkRead(reporter, wkbmp, clippedRect.fLeft,
                              clippedRect.fTop, true, false,
                              SkCanvas::kNative_Premul_Config8888);
                } else {
                    REPORTER_ASSERT(reporter, !success);
                }
            }
        }
    }
}
}

#include "TestClassDef.h"
DEFINE_GPUTESTCLASS("ReadPixels", ReadPixelsTestClass, ReadPixelsTest)

