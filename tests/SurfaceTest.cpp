/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkData.h"
#include "SkDevice.h"
#include "SkImageEncoder.h"
#include "SkImage_Base.h"
#include "SkPath.h"
#include "SkRRect.h"
#include "SkSurface.h"
#include "SkUtils.h"
#include "Test.h"

#if SK_SUPPORT_GPU
#include "GrContextFactory.h"
#include "GrTest.h"
#else
class GrContextFactory;
class GrContext;
#endif

enum SurfaceType {
    kRaster_SurfaceType,
    kRasterDirect_SurfaceType,
    kGpu_SurfaceType,
    kGpuScratch_SurfaceType,

    kLastSurfaceType = kGpuScratch_SurfaceType
};
static const int kSurfaceTypeCnt = kLastSurfaceType + 1;

static void release_storage(void* pixels, void* context) {
    SkASSERT(pixels == context);
    sk_free(pixels);
}

static SkSurface* create_surface(SurfaceType surfaceType, GrContext* context,
                                 SkAlphaType at = kPremul_SkAlphaType,
                                 SkImageInfo* requestedInfo = NULL) {
    const SkImageInfo info = SkImageInfo::MakeN32(10, 10, at);

    if (requestedInfo) {
        *requestedInfo = info;
    }

    switch (surfaceType) {
        case kRaster_SurfaceType:
            return SkSurface::NewRaster(info);
        case kRasterDirect_SurfaceType: {
            const size_t rowBytes = info.minRowBytes();
            void* storage = sk_malloc_throw(info.getSafeSize(rowBytes));
            return SkSurface::NewRasterDirectReleaseProc(info, storage, rowBytes,
                                                         release_storage, storage);
        }
        case kGpu_SurfaceType:
            return SkSurface::NewRenderTarget(context, SkSurface::kNo_Budgeted, info, 0, NULL);
        case kGpuScratch_SurfaceType:
            return SkSurface::NewRenderTarget(context, SkSurface::kYes_Budgeted, info, 0, NULL);
    }
    return NULL;
}

enum ImageType {
    kRasterCopy_ImageType,
    kRasterData_ImageType,
    kRasterProc_ImageType,
    kGpu_ImageType,
    kCodec_ImageType,
};

#include "SkImageGenerator.h"

class EmptyGenerator : public SkImageGenerator {
public:
    EmptyGenerator() : SkImageGenerator(SkImageInfo::MakeN32Premul(0, 0)) {}
};

static void test_empty_image(skiatest::Reporter* reporter) {
    const SkImageInfo info = SkImageInfo::Make(0, 0, kN32_SkColorType, kPremul_SkAlphaType);
    
    REPORTER_ASSERT(reporter, NULL == SkImage::NewRasterCopy(info, NULL, 0));
    REPORTER_ASSERT(reporter, NULL == SkImage::NewRasterData(info, NULL, 0));
    REPORTER_ASSERT(reporter, NULL == SkImage::NewFromRaster(info, NULL, 0, NULL, NULL));
    REPORTER_ASSERT(reporter, NULL == SkImage::NewFromGenerator(SkNEW(EmptyGenerator)));
}

static void test_empty_surface(skiatest::Reporter* reporter, GrContext* ctx) {
    const SkImageInfo info = SkImageInfo::Make(0, 0, kN32_SkColorType, kPremul_SkAlphaType);
    
    REPORTER_ASSERT(reporter, NULL == SkSurface::NewRaster(info));
    REPORTER_ASSERT(reporter, NULL == SkSurface::NewRasterDirect(info, NULL, 0));
    if (ctx) {
        REPORTER_ASSERT(reporter, NULL ==
                        SkSurface::NewRenderTarget(ctx, SkSurface::kNo_Budgeted, info, 0, NULL));
    }
}

#if SK_SUPPORT_GPU
static void test_wrapped_texture_surface(skiatest::Reporter* reporter, GrContext* ctx) {
    if (NULL == ctx) {
        return;
    }

    const GrGpu* gpu = ctx->getGpu();
    if (!gpu) {
        return;
    }

    // Test the wrapped factory for SkSurface by creating a backend texture and then wrap it in
    // a SkSurface.
    static const int kW = 100;
    static const int kH = 100;
    static const uint32_t kOrigColor = 0xFFAABBCC;
    SkAutoTArray<uint32_t> pixels(kW * kH);
    sk_memset32(pixels.get(), kOrigColor, kW * kH);
    GrBackendObject texID = gpu->createTestingOnlyBackendTexture(pixels.get(), kW, kH,
                                                                 kRGBA_8888_GrPixelConfig);

    GrBackendTextureDesc wrappedDesc;
    wrappedDesc.fConfig = kRGBA_8888_GrPixelConfig;
    wrappedDesc.fWidth = kW;
    wrappedDesc.fHeight = kH;
    wrappedDesc.fOrigin = kBottomLeft_GrSurfaceOrigin;
    wrappedDesc.fSampleCnt = 0;
    wrappedDesc.fFlags = kRenderTarget_GrBackendTextureFlag;
    wrappedDesc.fTextureHandle = texID;

    SkAutoTUnref<SkSurface> surface(SkSurface::NewWrappedRenderTarget(ctx, wrappedDesc, NULL));
    REPORTER_ASSERT(reporter, surface);
    if (surface) {
        // Validate that we can draw to the canvas and that the original texture color is preserved
        // in pixels that aren't rendered to via the surface.
        SkPaint paint;
        static const SkColor kRectColor = ~kOrigColor | 0xFF000000;
        paint.setColor(kRectColor);
        surface->getCanvas()->drawRect(SkRect::MakeWH(SkIntToScalar(kW), SkIntToScalar(kH)/2),
                                       paint);
        SkImageInfo readInfo = SkImageInfo::MakeN32Premul(kW, kH);
        surface->readPixels(readInfo, pixels.get(), kW * sizeof(uint32_t), 0, 0);
        bool stop = false;
        SkPMColor origColorPM = SkPackARGB32((kOrigColor >> 24 & 0xFF),
                                             (kOrigColor >>  0 & 0xFF),
                                             (kOrigColor >>  8 & 0xFF),
                                             (kOrigColor >> 16 & 0xFF));
        SkPMColor rectColorPM = SkPackARGB32((kRectColor >> 24 & 0xFF),
                                             (kRectColor >> 16 & 0xFF),
                                             (kRectColor >>  8 & 0xFF),
                                             (kRectColor >>  0 & 0xFF));
        for (int y = 0; y < kH/2 && !stop; ++y) {
            for (int x = 0; x < kW && !stop; ++x) {
                REPORTER_ASSERT(reporter, rectColorPM == pixels[x + y * kW]);
                if (rectColorPM != pixels[x + y * kW]) {
                    stop = true;
                }
            }
        }
        stop = false;
        for (int y = kH/2; y < kH && !stop; ++y) {
            for (int x = 0; x < kW && !stop; ++x) {
                REPORTER_ASSERT(reporter, origColorPM == pixels[x + y * kW]);
                if (origColorPM != pixels[x + y * kW]) {
                    stop = true;
                }
            }
        }
    }
}
#endif


static void test_image(skiatest::Reporter* reporter) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(1, 1);
    size_t rowBytes = info.minRowBytes();
    size_t size = info.getSafeSize(rowBytes);
    SkData* data = SkData::NewUninitialized(size);

    REPORTER_ASSERT(reporter, data->unique());
    SkImage* image = SkImage::NewRasterData(info, data, rowBytes);
    REPORTER_ASSERT(reporter, !data->unique());
    image->unref();
    REPORTER_ASSERT(reporter, data->unique());
    data->unref();
}

// Want to ensure that our Release is called when the owning image is destroyed
struct ReleaseDataContext {
    skiatest::Reporter* fReporter;
    SkData*             fData;

    static void Release(const void* pixels, void* context) {
        ReleaseDataContext* state = (ReleaseDataContext*)context;
        REPORTER_ASSERT(state->fReporter, state->fData);
        state->fData->unref();
        state->fData = NULL;
    }
};

// May we (soon) eliminate the need to keep testing this, by hiding the bloody device!
#include "SkDevice.h"
static uint32_t get_legacy_gen_id(SkSurface* surf) {
    SkBaseDevice* device = surf->getCanvas()->getDevice_just_for_deprecated_compatibility_testing();
    return device->accessBitmap(false).getGenerationID();
}

/*
 *  Test legacy behavor of bumping the surface's device's bitmap's genID when we access its
 *  texture handle for writing.
 *
 *  Note: this needs to be tested separately from checking newImageSnapshot, as calling that
 *  can also incidentally bump the genID (when a new backing surface is created).
 */
template <class F>
static void test_texture_handle_genID(skiatest::Reporter* reporter, SkSurface* surf, F f) {
    const uint32_t gen0 = get_legacy_gen_id(surf);
    f(surf, SkSurface::kFlushRead_BackendHandleAccess);
    const uint32_t gen1 = get_legacy_gen_id(surf);
    REPORTER_ASSERT(reporter, gen0 == gen1);

    f(surf, SkSurface::kFlushWrite_BackendHandleAccess);
    const uint32_t gen2 = get_legacy_gen_id(surf);
    REPORTER_ASSERT(reporter, gen0 != gen2);

    f(surf, SkSurface::kDiscardWrite_BackendHandleAccess);
    const uint32_t gen3 = get_legacy_gen_id(surf);
    REPORTER_ASSERT(reporter, gen0 != gen3);
    REPORTER_ASSERT(reporter, gen2 != gen3);
}

template <class F>
static void test_backend_handle(skiatest::Reporter* reporter, SkSurface* surf, F f) {
    SkAutoTUnref<SkImage> image0(surf->newImageSnapshot());
    GrBackendObject obj = f(surf, SkSurface::kFlushRead_BackendHandleAccess);
    REPORTER_ASSERT(reporter, obj != 0);
    SkAutoTUnref<SkImage> image1(surf->newImageSnapshot());
    // just read access should not affect the snapshot
    REPORTER_ASSERT(reporter, image0->uniqueID() == image1->uniqueID());

    obj = f(surf, SkSurface::kFlushWrite_BackendHandleAccess);
    REPORTER_ASSERT(reporter, obj != 0);
    SkAutoTUnref<SkImage> image2(surf->newImageSnapshot());
    // expect a new image, since we claimed we would write
    REPORTER_ASSERT(reporter, image0->uniqueID() != image2->uniqueID());

    obj = f(surf, SkSurface::kDiscardWrite_BackendHandleAccess);
    REPORTER_ASSERT(reporter, obj != 0);
    SkAutoTUnref<SkImage> image3(surf->newImageSnapshot());
    // expect a new(er) image, since we claimed we would write
    REPORTER_ASSERT(reporter, image0->uniqueID() != image3->uniqueID());
    REPORTER_ASSERT(reporter, image2->uniqueID() != image3->uniqueID());
}

static SkImage* create_image(skiatest::Reporter* reporter,
                             ImageType imageType, GrContext* context, SkColor color,
                             ReleaseDataContext* releaseContext) {
    const SkPMColor pmcolor = SkPreMultiplyColor(color);
    const SkImageInfo info = SkImageInfo::MakeN32Premul(10, 10);
    const size_t rowBytes = info.minRowBytes();
    const size_t size = rowBytes * info.height();

    SkAutoTUnref<SkData> data(SkData::NewUninitialized(size));
    void* addr = data->writable_data();
    sk_memset32((SkPMColor*)addr, pmcolor, SkToInt(size >> 2));

    switch (imageType) {
        case kRasterCopy_ImageType:
            return SkImage::NewRasterCopy(info, addr, rowBytes);
        case kRasterData_ImageType:
            return SkImage::NewRasterData(info, data, rowBytes);
        case kRasterProc_ImageType:
            SkASSERT(releaseContext);
            releaseContext->fData = SkRef(data.get());
            return SkImage::NewFromRaster(info, addr, rowBytes,
                                          ReleaseDataContext::Release, releaseContext);
        case kGpu_ImageType: {
            SkAutoTUnref<SkSurface> surf(
                SkSurface::NewRenderTarget(context, SkSurface::kNo_Budgeted, info, 0));
            surf->getCanvas()->clear(color);
            // test our backing texture / rendertarget while were here...
            auto textureAccessorFunc =
                    [](SkSurface* surf, SkSurface::BackendHandleAccess access) -> GrBackendObject {
                        return surf->getTextureHandle(access); };
            auto renderTargetAccessorFunc =
                    [](SkSurface* surf, SkSurface::BackendHandleAccess access) -> GrBackendObject {
                        GrBackendObject obj;
                        SkAssertResult(surf->getRenderTargetHandle(&obj, access));
                        return obj; };
            test_backend_handle(reporter, surf, textureAccessorFunc);
            test_backend_handle(reporter, surf, renderTargetAccessorFunc);
            test_texture_handle_genID(reporter, surf, textureAccessorFunc);
            test_texture_handle_genID(reporter, surf, renderTargetAccessorFunc);

            // redraw so our returned image looks as expected.
            surf->getCanvas()->clear(color);
            return surf->newImageSnapshot();
        }
        case kCodec_ImageType: {
            SkBitmap bitmap;
            bitmap.installPixels(info, addr, rowBytes);
            SkAutoTUnref<SkData> src(
                 SkImageEncoder::EncodeData(bitmap, SkImageEncoder::kPNG_Type, 100));
            return SkImage::NewFromEncoded(src);
        }
    }
    SkASSERT(false);
    return NULL;
}

static void set_pixels(SkPMColor pixels[], int count, SkPMColor color) {
    sk_memset32(pixels, color, count);
}
static bool has_pixels(const SkPMColor pixels[], int count, SkPMColor expected) {
    for (int i = 0; i < count; ++i) {
        if (pixels[i] != expected) {
            return false;
        }
    }
    return true;
}

static void test_image_readpixels(skiatest::Reporter* reporter, SkImage* image,
                                  SkPMColor expected) {
    const SkPMColor notExpected = ~expected;

    const int w = 2, h = 2;
    const size_t rowBytes = w * sizeof(SkPMColor);
    SkPMColor pixels[w*h];

    SkImageInfo info;

    info = SkImageInfo::MakeUnknown(w, h);
    REPORTER_ASSERT(reporter, !image->readPixels(info, pixels, rowBytes, 0, 0));

    // out-of-bounds should fail
    info = SkImageInfo::MakeN32Premul(w, h);
    REPORTER_ASSERT(reporter, !image->readPixels(info, pixels, rowBytes, -w, 0));
    REPORTER_ASSERT(reporter, !image->readPixels(info, pixels, rowBytes, 0, -h));
    REPORTER_ASSERT(reporter, !image->readPixels(info, pixels, rowBytes, image->width(), 0));
    REPORTER_ASSERT(reporter, !image->readPixels(info, pixels, rowBytes, 0, image->height()));

    // top-left should succeed
    set_pixels(pixels, w*h, notExpected);
    REPORTER_ASSERT(reporter, image->readPixels(info, pixels, rowBytes, 0, 0));
    REPORTER_ASSERT(reporter, has_pixels(pixels, w*h, expected));

    // bottom-right should succeed
    set_pixels(pixels, w*h, notExpected);
    REPORTER_ASSERT(reporter, image->readPixels(info, pixels, rowBytes,
                                                image->width() - w, image->height() - h));
    REPORTER_ASSERT(reporter, has_pixels(pixels, w*h, expected));

    // partial top-left should succeed
    set_pixels(pixels, w*h, notExpected);
    REPORTER_ASSERT(reporter, image->readPixels(info, pixels, rowBytes, -1, -1));
    REPORTER_ASSERT(reporter, pixels[3] == expected);
    REPORTER_ASSERT(reporter, has_pixels(pixels, w*h - 1, notExpected));

    // partial bottom-right should succeed
    set_pixels(pixels, w*h, notExpected);
    REPORTER_ASSERT(reporter, image->readPixels(info, pixels, rowBytes,
                                                image->width() - 1, image->height() - 1));
    REPORTER_ASSERT(reporter, pixels[0] == expected);
    REPORTER_ASSERT(reporter, has_pixels(&pixels[1], w*h - 1, notExpected));
}

static void check_legacy_bitmap(skiatest::Reporter* reporter, const SkImage* image,
                                const SkBitmap& bitmap, SkImage::LegacyBitmapMode mode) {
    REPORTER_ASSERT(reporter, image->width() == bitmap.width());
    REPORTER_ASSERT(reporter, image->height() == bitmap.height());
    REPORTER_ASSERT(reporter, image->isOpaque() == bitmap.isOpaque());

    if (SkImage::kRO_LegacyBitmapMode == mode) {
        REPORTER_ASSERT(reporter, bitmap.isImmutable());
    }

    SkAutoLockPixels alp(bitmap);
    REPORTER_ASSERT(reporter, bitmap.getPixels());

    const SkImageInfo info = SkImageInfo::MakeN32(1, 1, bitmap.alphaType());
    SkPMColor imageColor;
    REPORTER_ASSERT(reporter, image->readPixels(info, &imageColor, sizeof(SkPMColor), 0, 0));
    REPORTER_ASSERT(reporter, imageColor == *bitmap.getAddr32(0, 0));
}

static void test_legacy_bitmap(skiatest::Reporter* reporter, const SkImage* image) {
    const SkImage::LegacyBitmapMode modes[] = {
        SkImage::kRO_LegacyBitmapMode,
        SkImage::kRW_LegacyBitmapMode,
    };
    for (size_t i = 0; i < SK_ARRAY_COUNT(modes); ++i) {
        SkBitmap bitmap;
        REPORTER_ASSERT(reporter, image->asLegacyBitmap(&bitmap, modes[i]));
        check_legacy_bitmap(reporter, image, bitmap, modes[i]);

        // Test subsetting to exercise the rowBytes logic.
        SkBitmap tmp;
        REPORTER_ASSERT(reporter, bitmap.extractSubset(&tmp, SkIRect::MakeWH(image->width() / 2,
                                                                             image->height() / 2)));
        SkAutoTUnref<SkImage> subsetImage(SkImage::NewFromBitmap(tmp));
        REPORTER_ASSERT(reporter, subsetImage);

        SkBitmap subsetBitmap;
        REPORTER_ASSERT(reporter, subsetImage->asLegacyBitmap(&subsetBitmap, modes[i]));
        check_legacy_bitmap(reporter, subsetImage, subsetBitmap, modes[i]);
    }
}

static void test_imagepeek(skiatest::Reporter* reporter, GrContextFactory* factory) {
    static const struct {
        ImageType   fType;
        bool        fPeekShouldSucceed;
        const char* fName;
    } gRec[] = {
        { kRasterCopy_ImageType,    true,       "RasterCopy"    },
        { kRasterData_ImageType,    true,       "RasterData"    },
        { kRasterProc_ImageType,    true,       "RasterProc"    },
        { kGpu_ImageType,           false,      "Gpu"           },
        { kCodec_ImageType,         false,      "Codec"         },
    };

    const SkColor color = SK_ColorRED;
    const SkPMColor pmcolor = SkPreMultiplyColor(color);

    GrContext* ctx = NULL;
#if SK_SUPPORT_GPU
    ctx = factory->get(GrContextFactory::kNative_GLContextType);
    if (NULL == ctx) {
        return;
    }
#endif

    ReleaseDataContext releaseCtx;
    releaseCtx.fReporter = reporter;

    for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); ++i) {
        SkImageInfo info;
        size_t rowBytes;

        releaseCtx.fData = NULL;
        SkAutoTUnref<SkImage> image(create_image(reporter, gRec[i].fType, ctx, color, &releaseCtx));
        if (!image.get()) {
            SkDebugf("failed to createImage[%d] %s\n", i, gRec[i].fName);
            continue;   // gpu may not be enabled
        }
        if (kRasterProc_ImageType == gRec[i].fType) {
            REPORTER_ASSERT(reporter, NULL != releaseCtx.fData);  // we are tracking the data
        } else {
            REPORTER_ASSERT(reporter, NULL == releaseCtx.fData);  // we ignored the context
        }

        test_legacy_bitmap(reporter, image);

        const void* addr = image->peekPixels(&info, &rowBytes);
        bool success = SkToBool(addr);
        REPORTER_ASSERT(reporter, gRec[i].fPeekShouldSucceed == success);
        if (success) {
            REPORTER_ASSERT(reporter, 10 == info.width());
            REPORTER_ASSERT(reporter, 10 == info.height());
            REPORTER_ASSERT(reporter, kN32_SkColorType == info.colorType());
            REPORTER_ASSERT(reporter, kPremul_SkAlphaType == info.alphaType() ||
                            kOpaque_SkAlphaType == info.alphaType());
            REPORTER_ASSERT(reporter, info.minRowBytes() <= rowBytes);
            REPORTER_ASSERT(reporter, pmcolor == *(const SkPMColor*)addr);
        }

        test_image_readpixels(reporter, image, pmcolor);
    }
    REPORTER_ASSERT(reporter, NULL == releaseCtx.fData);  // we released the data
}

static void test_canvaspeek(skiatest::Reporter* reporter,
                            GrContextFactory* factory) {
    static const struct {
        SurfaceType fType;
        bool        fPeekShouldSucceed;
    } gRec[] = {
        { kRaster_SurfaceType,          true    },
        { kRasterDirect_SurfaceType,    true    },
#if SK_SUPPORT_GPU
        { kGpu_SurfaceType,             false   },
        { kGpuScratch_SurfaceType,      false   },
#endif
    };

    const SkColor color = SK_ColorRED;
    const SkPMColor pmcolor = SkPreMultiplyColor(color);

    int cnt;
#if SK_SUPPORT_GPU
    cnt = GrContextFactory::kGLContextTypeCnt;
#else
    cnt = 1;
#endif

    for (int i= 0; i < cnt; ++i) {
        GrContext* context = NULL;
#if SK_SUPPORT_GPU
        GrContextFactory::GLContextType glCtxType = (GrContextFactory::GLContextType) i;
        if (!GrContextFactory::IsRenderingGLContext(glCtxType)) {
            continue;
        }
        context = factory->get(glCtxType);

        if (NULL == context) {
            continue;
        }
#endif
        for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); ++i) {
            SkImageInfo info, requestInfo;
            size_t rowBytes;

            SkAutoTUnref<SkSurface> surface(create_surface(gRec[i].fType, context,
                                                           kPremul_SkAlphaType, &requestInfo));
            surface->getCanvas()->clear(color);

            const void* addr = surface->getCanvas()->peekPixels(&info, &rowBytes);
            bool success = SkToBool(addr);
            REPORTER_ASSERT(reporter, gRec[i].fPeekShouldSucceed == success);

            SkImageInfo info2;
            size_t rb2;
            const void* addr2 = surface->peekPixels(&info2, &rb2);

            if (success) {
                REPORTER_ASSERT(reporter, requestInfo == info);
                REPORTER_ASSERT(reporter, requestInfo.minRowBytes() <= rowBytes);
                REPORTER_ASSERT(reporter, pmcolor == *(const SkPMColor*)addr);

                REPORTER_ASSERT(reporter, addr2 == addr);
                REPORTER_ASSERT(reporter, info2 == info);
                REPORTER_ASSERT(reporter, rb2 == rowBytes);
            } else {
                REPORTER_ASSERT(reporter, NULL == addr2);
            }
        }
    }
}

// For compatibility with clients that still call accessBitmap(), we need to ensure that we bump
// the bitmap's genID when we draw to it, else they won't know it has new values. When they are
// exclusively using surface/image, and we can hide accessBitmap from device, we can remove this
// test.
static void test_accessPixels(skiatest::Reporter* reporter, GrContextFactory* factory) {
    static const struct {
        SurfaceType fType;
        bool        fPeekShouldSucceed;
    } gRec[] = {
        { kRaster_SurfaceType,          true    },
        { kRasterDirect_SurfaceType,    true    },
#if SK_SUPPORT_GPU
        { kGpu_SurfaceType,             false   },
        { kGpuScratch_SurfaceType,      false   },
#endif
    };
    
    int cnt;
#if SK_SUPPORT_GPU
    cnt = GrContextFactory::kGLContextTypeCnt;
#else
    cnt = 1;
#endif
    
    for (int i= 0; i < cnt; ++i) {
        GrContext* context = NULL;
#if SK_SUPPORT_GPU
        GrContextFactory::GLContextType glCtxType = (GrContextFactory::GLContextType) i;
        if (!GrContextFactory::IsRenderingGLContext(glCtxType)) {
            continue;
        }
        context = factory->get(glCtxType);
        
        if (NULL == context) {
            continue;
        }
#endif
        for (size_t j = 0; j < SK_ARRAY_COUNT(gRec); ++j) {
            SkImageInfo info, requestInfo;
            
            SkAutoTUnref<SkSurface> surface(create_surface(gRec[j].fType, context,
                                                           kPremul_SkAlphaType,  &requestInfo));
            SkCanvas* canvas = surface->getCanvas();
            canvas->clear(0);

            SkBaseDevice* device = canvas->getDevice_just_for_deprecated_compatibility_testing();
            SkBitmap bm = device->accessBitmap(false);
            uint32_t genID0 = bm.getGenerationID();
            // Now we draw something, which needs to "dirty" the genID (sorta like copy-on-write)
            canvas->drawColor(SK_ColorBLUE);
            // Now check that we get a different genID
            uint32_t genID1 = bm.getGenerationID();
            REPORTER_ASSERT(reporter, genID0 != genID1);
        }
    }
}

static void test_snap_alphatype(skiatest::Reporter* reporter, GrContextFactory* factory) {
    GrContext* context = NULL;
#if SK_SUPPORT_GPU
    context = factory->get(GrContextFactory::kNative_GLContextType);
    if (NULL == context) {
        return;
    }
#endif
    for (int opaque = 0; opaque < 2; ++opaque) {
        SkAlphaType atype = SkToBool(opaque) ? kOpaque_SkAlphaType : kPremul_SkAlphaType;
        for (int st = 0; st < kSurfaceTypeCnt; ++st) {
            SurfaceType stype = (SurfaceType)st;
            SkAutoTUnref<SkSurface> surface(create_surface(stype, context, atype));
            REPORTER_ASSERT(reporter, surface);
            if (surface) {
                SkAutoTUnref<SkImage> image(surface->newImageSnapshot());
                REPORTER_ASSERT(reporter, image);
                if (image) {
                    REPORTER_ASSERT(reporter, image->isOpaque() == SkToBool(opaque));
                }
            }
        }
    }
}

static void test_backend_cow(skiatest::Reporter* reporter, SkSurface* surface,
                             SkSurface::BackendHandleAccess mode,
                             GrBackendObject (*func)(SkSurface*, SkSurface::BackendHandleAccess)) {
    GrBackendObject obj1 = func(surface, mode);
    SkAutoTUnref<SkImage> snap1(surface->newImageSnapshot());

    GrBackendObject obj2 = func(surface, mode);
    SkAutoTUnref<SkImage> snap2(surface->newImageSnapshot());

    // If the access mode triggers CoW, then the backend objects should reflect it.
    REPORTER_ASSERT(reporter, (obj1 == obj2) == (snap1 == snap2));
}

static void TestSurfaceCopyOnWrite(skiatest::Reporter* reporter, SurfaceType surfaceType,
                                   GrContext* context) {
    // Verify that the right canvas commands trigger a copy on write
    SkSurface* surface = create_surface(surfaceType, context);
    SkAutoTUnref<SkSurface> aur_surface(surface);
    SkCanvas* canvas = surface->getCanvas();

    const SkRect testRect =
        SkRect::MakeXYWH(SkIntToScalar(0), SkIntToScalar(0),
                         SkIntToScalar(4), SkIntToScalar(5));
    SkPath testPath;
    testPath.addRect(SkRect::MakeXYWH(SkIntToScalar(0), SkIntToScalar(0),
                                      SkIntToScalar(2), SkIntToScalar(1)));

    const SkIRect testIRect = SkIRect::MakeXYWH(0, 0, 2, 1);

    SkRegion testRegion;
    testRegion.setRect(testIRect);


    const SkColor testColor = 0x01020304;
    const SkPaint testPaint;
    const SkPoint testPoints[3] = {
        {SkIntToScalar(0), SkIntToScalar(0)},
        {SkIntToScalar(2), SkIntToScalar(1)},
        {SkIntToScalar(0), SkIntToScalar(2)}
    };
    const size_t testPointCount = 3;

    SkBitmap testBitmap;
    testBitmap.allocN32Pixels(10, 10);
    testBitmap.eraseColor(0);

    SkRRect testRRect;
    testRRect.setRectXY(testRect, SK_Scalar1, SK_Scalar1);

    SkString testText("Hello World");
    const SkPoint testPoints2[] = {
        { SkIntToScalar(0), SkIntToScalar(1) },
        { SkIntToScalar(1), SkIntToScalar(1) },
        { SkIntToScalar(2), SkIntToScalar(1) },
        { SkIntToScalar(3), SkIntToScalar(1) },
        { SkIntToScalar(4), SkIntToScalar(1) },
        { SkIntToScalar(5), SkIntToScalar(1) },
        { SkIntToScalar(6), SkIntToScalar(1) },
        { SkIntToScalar(7), SkIntToScalar(1) },
        { SkIntToScalar(8), SkIntToScalar(1) },
        { SkIntToScalar(9), SkIntToScalar(1) },
        { SkIntToScalar(10), SkIntToScalar(1) },
    };

#define EXPECT_COPY_ON_WRITE(command)                               \
    {                                                               \
        SkImage* imageBefore = surface->newImageSnapshot();         \
        SkAutoTUnref<SkImage> aur_before(imageBefore);              \
        canvas-> command ;                                          \
        SkImage* imageAfter = surface->newImageSnapshot();          \
        SkAutoTUnref<SkImage> aur_after(imageAfter);                \
        REPORTER_ASSERT(reporter, imageBefore != imageAfter);       \
    }

    EXPECT_COPY_ON_WRITE(clear(testColor))
    EXPECT_COPY_ON_WRITE(drawPaint(testPaint))
    EXPECT_COPY_ON_WRITE(drawPoints(SkCanvas::kPoints_PointMode, testPointCount, testPoints, \
        testPaint))
    EXPECT_COPY_ON_WRITE(drawOval(testRect, testPaint))
    EXPECT_COPY_ON_WRITE(drawRect(testRect, testPaint))
    EXPECT_COPY_ON_WRITE(drawRRect(testRRect, testPaint))
    EXPECT_COPY_ON_WRITE(drawPath(testPath, testPaint))
    EXPECT_COPY_ON_WRITE(drawBitmap(testBitmap, 0, 0))
    EXPECT_COPY_ON_WRITE(drawBitmapRect(testBitmap, testRect, nullptr))
    EXPECT_COPY_ON_WRITE(drawBitmapNine(testBitmap, testIRect, testRect, NULL))
    EXPECT_COPY_ON_WRITE(drawSprite(testBitmap, 0, 0, NULL))
    EXPECT_COPY_ON_WRITE(drawText(testText.c_str(), testText.size(), 0, 1, testPaint))
    EXPECT_COPY_ON_WRITE(drawPosText(testText.c_str(), testText.size(), testPoints2, \
        testPaint))
    EXPECT_COPY_ON_WRITE(drawTextOnPath(testText.c_str(), testText.size(), testPath, NULL, \
        testPaint))

    const SkSurface::BackendHandleAccess accessModes[] = {
        SkSurface::kFlushRead_BackendHandleAccess,
        SkSurface::kFlushWrite_BackendHandleAccess,
        SkSurface::kDiscardWrite_BackendHandleAccess,
    };

    for (auto access : accessModes) {
        test_backend_cow(reporter, surface, access,
                          [](SkSurface* s, SkSurface::BackendHandleAccess a) -> GrBackendObject {
            return s->getTextureHandle(a);
        });

        test_backend_cow(reporter, surface, access,
                          [](SkSurface* s, SkSurface::BackendHandleAccess a) -> GrBackendObject {
            GrBackendObject result;
            if (!s->getRenderTargetHandle(&result, a)) {
                return 0;
            }
            return result;
        });
    }
}

static void TestSurfaceWritableAfterSnapshotRelease(skiatest::Reporter* reporter,
                                                    SurfaceType surfaceType,
                                                    GrContext* context) {
    // This test succeeds by not triggering an assertion.
    // The test verifies that the surface remains writable (usable) after
    // acquiring and releasing a snapshot without triggering a copy on write.
    SkAutoTUnref<SkSurface> surface(create_surface(surfaceType, context));
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(1);
    surface->newImageSnapshot()->unref();  // Create and destroy SkImage
    canvas->clear(2);  // Must not assert internally
}

#if SK_SUPPORT_GPU
static void Test_crbug263329(skiatest::Reporter* reporter,
                             SurfaceType surfaceType,
                             GrContext* context) {
    // This is a regression test for crbug.com/263329
    // Bug was caused by onCopyOnWrite releasing the old surface texture
    // back to the scratch texture pool even though the texture is used
    // by and active SkImage_Gpu.
    SkAutoTUnref<SkSurface> surface1(create_surface(surfaceType, context));
    SkAutoTUnref<SkSurface> surface2(create_surface(surfaceType, context));
    SkCanvas* canvas1 = surface1->getCanvas();
    SkCanvas* canvas2 = surface2->getCanvas();
    canvas1->clear(1);
    SkAutoTUnref<SkImage> image1(surface1->newImageSnapshot());
    // Trigger copy on write, new backing is a scratch texture
    canvas1->clear(2);
    SkAutoTUnref<SkImage> image2(surface1->newImageSnapshot());
    // Trigger copy on write, old backing should not be returned to scratch
    // pool because it is held by image2
    canvas1->clear(3);

    canvas2->clear(4);
    SkAutoTUnref<SkImage> image3(surface2->newImageSnapshot());
    // Trigger copy on write on surface2. The new backing store should not
    // be recycling a texture that is held by an existing image.
    canvas2->clear(5);
    SkAutoTUnref<SkImage> image4(surface2->newImageSnapshot());
    REPORTER_ASSERT(reporter, as_IB(image4)->getTexture() != as_IB(image3)->getTexture());
    // The following assertion checks crbug.com/263329
    REPORTER_ASSERT(reporter, as_IB(image4)->getTexture() != as_IB(image2)->getTexture());
    REPORTER_ASSERT(reporter, as_IB(image4)->getTexture() != as_IB(image1)->getTexture());
    REPORTER_ASSERT(reporter, as_IB(image3)->getTexture() != as_IB(image2)->getTexture());
    REPORTER_ASSERT(reporter, as_IB(image3)->getTexture() != as_IB(image1)->getTexture());
    REPORTER_ASSERT(reporter, as_IB(image2)->getTexture() != as_IB(image1)->getTexture());
}

static void TestGetTexture(skiatest::Reporter* reporter,
                                 SurfaceType surfaceType,
                                 GrContext* context) {
    SkAutoTUnref<SkSurface> surface(create_surface(surfaceType, context));
    SkAutoTUnref<SkImage> image(surface->newImageSnapshot());
    GrTexture* texture = as_IB(image)->getTexture();
    if (surfaceType == kGpu_SurfaceType || surfaceType == kGpuScratch_SurfaceType) {
        REPORTER_ASSERT(reporter, texture);
        REPORTER_ASSERT(reporter, 0 != texture->getTextureHandle());
    } else {
        REPORTER_ASSERT(reporter, NULL == texture);
    }
    surface->notifyContentWillChange(SkSurface::kDiscard_ContentChangeMode);
    REPORTER_ASSERT(reporter, as_IB(image)->getTexture() == texture);
}

#include "GrGpuResourcePriv.h"
#include "SkGpuDevice.h"
#include "SkImage_Gpu.h"
#include "SkSurface_Gpu.h"

SkSurface::Budgeted is_budgeted(SkSurface* surf) {
    return ((SkSurface_Gpu*)surf)->getDevice()->accessRenderTarget()->resourcePriv().isBudgeted() ?
        SkSurface::kYes_Budgeted : SkSurface::kNo_Budgeted;
}

SkSurface::Budgeted is_budgeted(SkImage* image) {
    return ((SkImage_Gpu*)image)->getTexture()->resourcePriv().isBudgeted() ?
        SkSurface::kYes_Budgeted : SkSurface::kNo_Budgeted;
}

static void test_surface_budget(skiatest::Reporter* reporter, GrContext* context) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(8,8);
    for (int i = 0; i < 2; ++i) {
        SkSurface::Budgeted sbudgeted = i ? SkSurface::kYes_Budgeted : SkSurface::kNo_Budgeted;
        for (int j = 0; j < 2; ++j) {
            SkSurface::Budgeted ibudgeted = j ? SkSurface::kYes_Budgeted : SkSurface::kNo_Budgeted;
            SkAutoTUnref<SkSurface>
                surface(SkSurface::NewRenderTarget(context, sbudgeted, info, 0));
            SkASSERT(surface);
            REPORTER_ASSERT(reporter, sbudgeted == is_budgeted(surface));

            SkAutoTUnref<SkImage> image(surface->newImageSnapshot(ibudgeted));

            // Initially the image shares a texture with the surface, and the surface decides
            // whether it is budgeted or not.
            REPORTER_ASSERT(reporter, sbudgeted == is_budgeted(surface));
            REPORTER_ASSERT(reporter, sbudgeted == is_budgeted(image));

            // Now trigger copy-on-write
            surface->getCanvas()->clear(SK_ColorBLUE);

            // They don't share a texture anymore. They should each have made their own budget
            // decision.
            REPORTER_ASSERT(reporter, sbudgeted == is_budgeted(surface));
            REPORTER_ASSERT(reporter, ibudgeted == is_budgeted(image));
        }
    }
}

#endif

static void TestSurfaceNoCanvas(skiatest::Reporter* reporter,
                                          SurfaceType surfaceType,
                                          GrContext* context,
                                          SkSurface::ContentChangeMode mode) {
    // Verifies the robustness of SkSurface for handling use cases where calls
    // are made before a canvas is created.
    {
        // Test passes by not asserting
        SkSurface* surface = create_surface(surfaceType, context);
        SkAutoTUnref<SkSurface> aur_surface(surface);
        surface->notifyContentWillChange(mode);
        SkDEBUGCODE(surface->validate();)
    }
    {
        SkSurface* surface = create_surface(surfaceType, context);
        SkAutoTUnref<SkSurface> aur_surface(surface);
        SkImage* image1 = surface->newImageSnapshot();
        SkAutoTUnref<SkImage> aur_image1(image1);
        SkDEBUGCODE(image1->validate();)
        SkDEBUGCODE(surface->validate();)
        surface->notifyContentWillChange(mode);
        SkDEBUGCODE(image1->validate();)
        SkDEBUGCODE(surface->validate();)
        SkImage* image2 = surface->newImageSnapshot();
        SkAutoTUnref<SkImage> aur_image2(image2);
        SkDEBUGCODE(image2->validate();)
        SkDEBUGCODE(surface->validate();)
        REPORTER_ASSERT(reporter, image1 != image2);
    }

}

DEF_GPUTEST(Surface, reporter, factory) {
    test_image(reporter);

    TestSurfaceCopyOnWrite(reporter, kRaster_SurfaceType, NULL);
    TestSurfaceWritableAfterSnapshotRelease(reporter, kRaster_SurfaceType, NULL);
    TestSurfaceNoCanvas(reporter, kRaster_SurfaceType, NULL, SkSurface::kDiscard_ContentChangeMode);
    TestSurfaceNoCanvas(reporter, kRaster_SurfaceType, NULL, SkSurface::kRetain_ContentChangeMode);

    test_empty_image(reporter);
    test_empty_surface(reporter, NULL);

    test_imagepeek(reporter, factory);
    test_canvaspeek(reporter, factory);

    test_accessPixels(reporter, factory);

    test_snap_alphatype(reporter, factory);

#if SK_SUPPORT_GPU
    TestGetTexture(reporter, kRaster_SurfaceType, NULL);
    if (factory) {
        for (int i= 0; i < GrContextFactory::kGLContextTypeCnt; ++i) {
            GrContextFactory::GLContextType glCtxType = (GrContextFactory::GLContextType) i;
            if (!GrContextFactory::IsRenderingGLContext(glCtxType)) {
                continue;
            }
            GrContext* context = factory->get(glCtxType);
            if (context) {
                Test_crbug263329(reporter, kGpu_SurfaceType, context);
                Test_crbug263329(reporter, kGpuScratch_SurfaceType, context);
                TestSurfaceCopyOnWrite(reporter, kGpu_SurfaceType, context);
                TestSurfaceCopyOnWrite(reporter, kGpuScratch_SurfaceType, context);
                TestSurfaceWritableAfterSnapshotRelease(reporter, kGpu_SurfaceType, context);
                TestSurfaceWritableAfterSnapshotRelease(reporter, kGpuScratch_SurfaceType, context);
                TestSurfaceNoCanvas(reporter, kGpu_SurfaceType, context, SkSurface::kDiscard_ContentChangeMode);
                TestSurfaceNoCanvas(reporter, kGpuScratch_SurfaceType, context, SkSurface::kDiscard_ContentChangeMode);
                TestSurfaceNoCanvas(reporter, kGpu_SurfaceType, context, SkSurface::kRetain_ContentChangeMode);
                TestSurfaceNoCanvas(reporter, kGpuScratch_SurfaceType, context, SkSurface::kRetain_ContentChangeMode);
                TestGetTexture(reporter, kGpu_SurfaceType, context);
                TestGetTexture(reporter, kGpuScratch_SurfaceType, context);
                test_empty_surface(reporter, context);
                test_surface_budget(reporter, context);
                test_wrapped_texture_surface(reporter, context);
            }
        }
    }
#endif
}

#if SK_SUPPORT_GPU

struct ReleaseTextureContext {
    ReleaseTextureContext(skiatest::Reporter* reporter) {
        fReporter = reporter;
        fIsReleased = false;
    }

    skiatest::Reporter* fReporter;
    bool                fIsReleased;

    void doRelease() {
        REPORTER_ASSERT(fReporter, false == fIsReleased);
        fIsReleased = true;
    }

    static void ReleaseProc(void* context) {
        ((ReleaseTextureContext*)context)->doRelease();
    }
};

static SkImage* make_desc_image(GrContext* ctx, int w, int h, GrBackendObject texID,
                                ReleaseTextureContext* releaseContext) {
    GrBackendTextureDesc desc;
    desc.fConfig = kSkia8888_GrPixelConfig;
    // need to be a rendertarget for now...
    desc.fFlags = kRenderTarget_GrBackendTextureFlag;
    desc.fWidth = w;
    desc.fHeight = h;
    desc.fSampleCnt = 0;
    desc.fTextureHandle = texID;
    return releaseContext
                ? SkImage::NewFromTexture(ctx, desc, kPremul_SkAlphaType,
                                          ReleaseTextureContext::ReleaseProc, releaseContext)
                : SkImage::NewFromTextureCopy(ctx, desc, kPremul_SkAlphaType);
}

static void test_image_color(skiatest::Reporter* reporter, SkImage* image, SkPMColor expected) {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(1, 1);
    SkPMColor pixel;
    REPORTER_ASSERT(reporter, image->readPixels(info, &pixel, sizeof(pixel), 0, 0));
    REPORTER_ASSERT(reporter, pixel == expected);
}

DEF_GPUTEST(SkImage_NewFromTexture, reporter, factory) {
    GrContext* ctx = factory->get(GrContextFactory::kNative_GLContextType);
    if (!ctx) {
        REPORTER_ASSERT(reporter, false);
        return;
    }
    GrTextureProvider* provider = ctx->textureProvider();
    
    const int w = 10;
    const int h = 10;
    SkPMColor storage[w * h];
    const SkPMColor expected0 = SkPreMultiplyColor(SK_ColorRED);
    sk_memset32(storage, expected0, w * h);
    
    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;  // needs to be a rendertarget for readpixels();
    desc.fOrigin = kDefault_GrSurfaceOrigin;
    desc.fWidth = w;
    desc.fHeight = h;
    desc.fConfig = kSkia8888_GrPixelConfig;
    desc.fSampleCnt = 0;
    
    SkAutoTUnref<GrTexture> tex(provider->createTexture(desc, false, storage, w * 4));
    if (!tex) {
        REPORTER_ASSERT(reporter, false);
        return;
    }

    GrBackendObject srcTex = tex->getTextureHandle();
    ReleaseTextureContext releaseCtx(reporter);

    SkAutoTUnref<SkImage> refImg(make_desc_image(ctx, w, h, srcTex, &releaseCtx));
    SkAutoTUnref<SkImage> cpyImg(make_desc_image(ctx, w, h, srcTex, NULL));

    test_image_color(reporter, refImg, expected0);
    test_image_color(reporter, cpyImg, expected0);

    // Now lets jam new colors into our "external" texture, and see if the images notice
    const SkPMColor expected1 = SkPreMultiplyColor(SK_ColorBLUE);
    sk_memset32(storage, expected1, w * h);
    tex->writePixels(0, 0, w, h, kSkia8888_GrPixelConfig, storage, GrContext::kFlushWrites_PixelOp);

    // The cpy'd one should still see the old color
#if 0
    // There is no guarantee that refImg sees the new color. We are free to have made a copy. Our
    // write pixels call violated the contract with refImg and refImg is now undefined.
    test_image_color(reporter, refImg, expected1);
#endif
    test_image_color(reporter, cpyImg, expected0);

    // Now exercise the release proc
    REPORTER_ASSERT(reporter, !releaseCtx.fIsReleased);
    refImg.reset(NULL); // force a release of the image
    REPORTER_ASSERT(reporter, releaseCtx.fIsReleased);
}
#endif
