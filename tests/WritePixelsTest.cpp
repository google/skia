/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkMathPriv.h"
#include "SkRegion.h"
#include "SkSurface.h"
#include "Test.h"
#include "sk_tool_utils.h"

#if SK_SUPPORT_GPU
#include "GrContextFactory.h"
#include "SkGpuDevice.h"
#else
class GrContext;
class GrContextFactory;
#endif

static const int DEV_W = 100, DEV_H = 100;
static const SkIRect DEV_RECT = SkIRect::MakeWH(DEV_W, DEV_H);
static const SkRect DEV_RECT_S = SkRect::MakeWH(DEV_W * SK_Scalar1,
                                                DEV_H * SK_Scalar1);
static const U8CPU DEV_PAD = 0xee;

static SkPMColor get_canvas_color(int x, int y) {
    SkASSERT(x >= 0 && x < DEV_W);
    SkASSERT(y >= 0 && y < DEV_H);

    U8CPU r = x;
    U8CPU g = y;
    U8CPU b = 0xc;

    U8CPU a = 0x0;
    switch ((x+y) % 5) {
        case 0:
            a = 0xff;
            break;
        case 1:
            a = 0x80;
            break;
        case 2:
            a = 0xCC;
            break;
        case 3:
            a = 0x00;
            break;
        case 4:
            a = 0x01;
            break;
    }
    return SkPremultiplyARGBInline(a, r, g, b);
}

// assumes any premu/.unpremul has been applied
static uint32_t pack_color_type(SkColorType ct, U8CPU a, U8CPU r, U8CPU g, U8CPU b) {
    uint32_t r32;
    uint8_t* result = reinterpret_cast<uint8_t*>(&r32);
    switch (ct) {
        case kBGRA_8888_SkColorType:
            result[0] = b;
            result[1] = g;
            result[2] = r;
            result[3] = a;
            break;
        case kRGBA_8888_SkColorType:
            result[0] = r;
            result[1] = g;
            result[2] = b;
            result[3] = a;
            break;
        default:
            SkASSERT(0);
            return 0;
    }
    return r32;
}

static uint32_t get_bitmap_color(int x, int y, int w, SkColorType ct, SkAlphaType at) {
    int n = y * w + x;
    U8CPU b = n & 0xff;
    U8CPU g = (n >> 8) & 0xff;
    U8CPU r = (n >> 16) & 0xff;
    U8CPU a = 0;
    switch ((x+y) % 5) {
        case 4:
            a = 0xff;
            break;
        case 3:
            a = 0x80;
            break;
        case 2:
            a = 0xCC;
            break;
        case 1:
            a = 0x01;
            break;
        case 0:
            a = 0x00;
            break;
    }
    if (kPremul_SkAlphaType == at) {
        r = SkMulDiv255Ceiling(r, a);
        g = SkMulDiv255Ceiling(g, a);
        b = SkMulDiv255Ceiling(b, a);
    }
    return pack_color_type(ct, a, r, g , b);
}

static void fill_canvas(SkCanvas* canvas) {
    SkBitmap bmp;
    if (bmp.isNull()) {
        bmp.allocN32Pixels(DEV_W, DEV_H);
        for (int y = 0; y < DEV_H; ++y) {
            for (int x = 0; x < DEV_W; ++x) {
                *bmp.getAddr32(x, y) = get_canvas_color(x, y);
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

/**
 *  Lucky for us, alpha is always in the same spot (SK_A32_SHIFT), for both RGBA and BGRA.
 *  Thus this routine doesn't need to know the exact colortype
 */
static uint32_t premul(uint32_t color) {
    unsigned a = SkGetPackedA32(color);
    // these next three are not necessarily r,g,b in that order, but they are r,g,b in some order.
    unsigned c0 = SkGetPackedR32(color);
    unsigned c1 = SkGetPackedG32(color);
    unsigned c2 = SkGetPackedB32(color);
    c0 = SkMulDiv255Ceiling(c0, a);
    c1 = SkMulDiv255Ceiling(c1, a);
    c2 = SkMulDiv255Ceiling(c2, a);
    return SkPackARGB32NoCheck(a, c0, c1, c2);
}

static SkPMColor convert_to_PMColor(SkColorType ct, SkAlphaType at, uint32_t color) {
    if (kUnpremul_SkAlphaType == at) {
        color = premul(color);
    }
    switch (ct) {
        case kRGBA_8888_SkColorType:
            color = SkSwizzle_RGBA_to_PMColor(color);
            break;
        case kBGRA_8888_SkColorType:
            color = SkSwizzle_BGRA_to_PMColor(color);
            break;
        default:
            SkASSERT(0);
            break;
    }
    return color;
}

static bool check_pixel(SkPMColor a, SkPMColor b, bool didPremulConversion) {
    if (!didPremulConversion) {
        return a == b;
    }
    int32_t aA = static_cast<int32_t>(SkGetPackedA32(a));
    int32_t aR = static_cast<int32_t>(SkGetPackedR32(a));
    int32_t aG = static_cast<int32_t>(SkGetPackedG32(a));
    int32_t aB = SkGetPackedB32(a);

    int32_t bA = static_cast<int32_t>(SkGetPackedA32(b));
    int32_t bR = static_cast<int32_t>(SkGetPackedR32(b));
    int32_t bG = static_cast<int32_t>(SkGetPackedG32(b));
    int32_t bB = static_cast<int32_t>(SkGetPackedB32(b));

    return aA == bA &&
           SkAbs32(aR - bR) <= 1 &&
           SkAbs32(aG - bG) <= 1 &&
           SkAbs32(aB - bB) <= 1;
}

static bool check_write(skiatest::Reporter* reporter, SkCanvas* canvas, const SkBitmap& bitmap,
                       int writeX, int writeY) {
    const SkImageInfo canvasInfo = canvas->imageInfo();
    size_t canvasRowBytes;
    const uint32_t* canvasPixels;

    // Can't use canvas->peekPixels(), as we are trying to look at GPU pixels sometimes as well.
    // At some point this will be unsupported, as we won't allow accessBitmap() to magically call
    // readPixels for the client.
    SkBitmap secretDevBitmap;
    canvas->readPixels(canvasInfo.bounds(), &secretDevBitmap);

    SkAutoLockPixels alp(secretDevBitmap);
    canvasRowBytes = secretDevBitmap.rowBytes();
    canvasPixels = static_cast<const uint32_t*>(secretDevBitmap.getPixels());

    if (NULL == canvasPixels) {
        return false;
    }

    if (canvasInfo.width() != DEV_W ||
        canvasInfo.height() != DEV_H ||
        canvasInfo.colorType() != kN32_SkColorType) {
        return false;
    }

    const SkImageInfo bmInfo = bitmap.info();

    SkIRect writeRect = SkIRect::MakeXYWH(writeX, writeY, bitmap.width(), bitmap.height());
    for (int cy = 0; cy < DEV_H; ++cy) {
        for (int cx = 0; cx < DEV_W; ++cx) {
            SkPMColor canvasPixel = canvasPixels[cx];
            if (writeRect.contains(cx, cy)) {
                int bx = cx - writeX;
                int by = cy - writeY;
                uint32_t bmpColor8888 = get_bitmap_color(bx, by, bitmap.width(),
                                                       bmInfo.colorType(), bmInfo.alphaType());
                bool mul = (kUnpremul_SkAlphaType == bmInfo.alphaType());
                SkPMColor bmpPMColor = convert_to_PMColor(bmInfo.colorType(), bmInfo.alphaType(),
                                                          bmpColor8888);
                if (!check_pixel(bmpPMColor, canvasPixel, mul)) {
                    ERRORF(reporter, "Expected canvas pixel at %d, %d to be 0x%08x, got 0x%08x. "
                           "Write performed premul: %d", cx, cy, bmpPMColor, canvasPixel, mul);
                    return false;
                }
            } else {
                SkPMColor testColor = get_canvas_color(cx, cy);
                if (canvasPixel != testColor) {
                    ERRORF(reporter, "Canvas pixel outside write rect at %d, %d changed."
                           " Should be 0x%08x, got 0x%08x. ", cx, cy, testColor, canvasPixel);
                    return false;
                }
            }
        }
        if (cy != DEV_H -1) {
            const char* pad = reinterpret_cast<const char*>(canvasPixels + DEV_W);
            for (size_t px = 0; px < canvasRowBytes - 4 * DEV_W; ++px) {
                bool check;
                REPORTER_ASSERT(reporter, check = (pad[px] == static_cast<char>(DEV_PAD)));
                if (!check) {
                    return false;
                }
            }
        }
        canvasPixels += canvasRowBytes/4;
    }

    return true;
}

enum DevType {
    kRaster_DevType,
#if SK_SUPPORT_GPU
    kGpu_BottomLeft_DevType,
    kGpu_TopLeft_DevType,
#endif
};

struct CanvasConfig {
    DevType fDevType;
    bool fTightRowBytes;
};

static const CanvasConfig gCanvasConfigs[] = {
    {kRaster_DevType, true},
    {kRaster_DevType, false},
#if SK_SUPPORT_GPU
    {kGpu_BottomLeft_DevType, true}, // row bytes has no meaning on gpu devices
    {kGpu_TopLeft_DevType, true}, // row bytes has no meaning on gpu devices
#endif
};

#include "SkMallocPixelRef.h"

// This is a tricky pattern, because we have to setConfig+rowBytes AND specify
// a custom pixelRef (which also has to specify its rowBytes), so we have to be
// sure that the two rowBytes match (and the infos match).
//
static bool alloc_row_bytes(SkBitmap* bm, const SkImageInfo& info, size_t rowBytes) {
    if (!bm->setInfo(info, rowBytes)) {
        return false;
    }
    SkPixelRef* pr = SkMallocPixelRef::NewAllocate(info, rowBytes, NULL);
    bm->setPixelRef(pr)->unref();
    return true;
}

static void free_pixels(void* pixels, void* ctx) {
    sk_free(pixels);
}

static SkSurface* create_surface(const CanvasConfig& c, GrContext* grCtx) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(DEV_W, DEV_H);
    switch (c.fDevType) {
        case kRaster_DevType: {
            const size_t rowBytes = c.fTightRowBytes ? info.minRowBytes() : 4 * DEV_W + 100;
            const size_t size = info.getSafeSize(rowBytes);
            void* pixels = sk_malloc_throw(size);
            // if rowBytes isn't tight then set the padding to a known value
            if (!c.fTightRowBytes) {
                memset(pixels, DEV_PAD, size);
            }
            return SkSurface::NewRasterDirectReleaseProc(info, pixels, rowBytes, free_pixels, NULL);
        }
#if SK_SUPPORT_GPU
        case kGpu_BottomLeft_DevType:
        case kGpu_TopLeft_DevType:
            GrSurfaceDesc desc;
            desc.fFlags = kRenderTarget_GrSurfaceFlag;
            desc.fWidth = DEV_W;
            desc.fHeight = DEV_H;
            desc.fConfig = kSkia8888_GrPixelConfig;
            desc.fOrigin = kGpu_TopLeft_DevType == c.fDevType ?
                kTopLeft_GrSurfaceOrigin : kBottomLeft_GrSurfaceOrigin;
            SkAutoTUnref<GrTexture> texture(grCtx->textureProvider()->createTexture(desc, false));
            return SkSurface::NewRenderTargetDirect(texture->asRenderTarget());
#endif
    }
    return NULL;
}

static bool setup_bitmap(SkBitmap* bm, SkColorType ct, SkAlphaType at, int w, int h, int tightRB) {
    size_t rowBytes = tightRB ? 0 : 4 * w + 60;
    SkImageInfo info = SkImageInfo::Make(w, h, ct, at);
    if (!alloc_row_bytes(bm, info, rowBytes)) {
        return false;
    }
    SkAutoLockPixels alp(*bm);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            *bm->getAddr32(x, y) = get_bitmap_color(x, y, w, ct, at);
        }
    }
    return true;
}

static void call_writepixels(SkCanvas* canvas) {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(1, 1);
    SkPMColor pixel = 0;
    canvas->writePixels(info, &pixel, sizeof(SkPMColor), 0, 0);
}

static void test_surface_genid(skiatest::Reporter* reporter) {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(100, 100);
    SkAutoTUnref<SkSurface> surface(SkSurface::NewRaster(info));
    uint32_t genID1 = surface->generationID();
    call_writepixels(surface->getCanvas());
    uint32_t genID2 = surface->generationID();
    REPORTER_ASSERT(reporter, genID1 != genID2);
}

DEF_GPUTEST(WritePixels, reporter, factory) {
    test_surface_genid(reporter);

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

    for (size_t i = 0; i < SK_ARRAY_COUNT(gCanvasConfigs); ++i) {
        int glCtxTypeCnt = 1;
#if SK_SUPPORT_GPU
        bool isGPUDevice = kGpu_TopLeft_DevType == gCanvasConfigs[i].fDevType ||
                           kGpu_BottomLeft_DevType == gCanvasConfigs[i].fDevType;
        if (isGPUDevice) {
            glCtxTypeCnt = GrContextFactory::kGLContextTypeCnt;
        }
#endif
        for (int glCtxType = 0; glCtxType < glCtxTypeCnt; ++glCtxType) {
            GrContext* context = NULL;
#if SK_SUPPORT_GPU
            if (isGPUDevice) {
                GrContextFactory::GLContextType type =
                    static_cast<GrContextFactory::GLContextType>(glCtxType);
                if (!GrContextFactory::IsRenderingGLContext(type)) {
                    continue;
                }
                context = factory->get(type);
                if (NULL == context) {
                    continue;
                }
            }
#endif

            SkAutoTUnref<SkSurface> surface(create_surface(gCanvasConfigs[i], context));
            SkCanvas& canvas = *surface->getCanvas();

            static const struct {
                SkColorType fColorType;
                SkAlphaType fAlphaType;
            } gSrcConfigs[] = {
                { kRGBA_8888_SkColorType, kPremul_SkAlphaType },
                { kRGBA_8888_SkColorType, kUnpremul_SkAlphaType },
                { kBGRA_8888_SkColorType, kPremul_SkAlphaType },
                { kBGRA_8888_SkColorType, kUnpremul_SkAlphaType },
            };
            for (size_t r = 0; r < SK_ARRAY_COUNT(testRects); ++r) {
                const SkIRect& rect = testRects[r];
                for (int tightBmp = 0; tightBmp < 2; ++tightBmp) {
                    for (size_t c = 0; c < SK_ARRAY_COUNT(gSrcConfigs); ++c) {
                        const SkColorType ct = gSrcConfigs[c].fColorType;
                        const SkAlphaType at = gSrcConfigs[c].fAlphaType;

                        fill_canvas(&canvas);
                        SkBitmap bmp;
                        REPORTER_ASSERT(reporter, setup_bitmap(&bmp, ct, at, rect.width(),
                                                               rect.height(), SkToBool(tightBmp)));
                        uint32_t idBefore = surface->generationID();

                       // sk_tool_utils::write_pixels(&canvas, bmp, rect.fLeft, rect.fTop, ct, at);
                        canvas.writePixels(bmp, rect.fLeft, rect.fTop);

                        uint32_t idAfter = surface->generationID();
                        REPORTER_ASSERT(reporter, check_write(reporter, &canvas, bmp,
                                                              rect.fLeft, rect.fTop));

                        // we should change the genID iff pixels were actually written.
                        SkIRect canvasRect = SkIRect::MakeSize(canvas.getDeviceSize());
                        SkIRect writeRect = SkIRect::MakeXYWH(rect.fLeft, rect.fTop,
                                                              bmp.width(), bmp.height());
                        bool intersects = SkIRect::Intersects(canvasRect, writeRect) ;
                        REPORTER_ASSERT(reporter, intersects == (idBefore != idAfter));
                    }
                }
            }
        }
    }
}
