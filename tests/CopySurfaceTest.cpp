/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#include "include/core/SkImageInfo.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/gpu/GrContext.h"
#include "include/gpu/GrTypes.h"
#include "include/private/GrSurfaceProxy.h"
#include "include/private/GrTextureProxy.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkUtils.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrSurfaceContext.h"
#include "src/gpu/SkGr.h"
#include "tests/Test.h"
#include "tools/gpu/GrContextFactory.h"
#include "tools/gpu/ProxyUtils.h"

#include <utility>

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(CopySurface, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    static const int kW = 10;
    static const int kH = 10;
    static const size_t kRowBytes = sizeof(uint32_t) * kW;

    SkAutoTMalloc<uint32_t> srcPixels(kW * kH);
    for (int i = 0; i < kW * kH; ++i) {
        srcPixels.get()[i] = i;
    }

    SkAutoTMalloc<uint32_t> dstPixels(kW * kH);
    for (int i = 0; i < kW * kH; ++i) {
        dstPixels.get()[i] = ~i;
    }

    static const SkIRect kSrcRects[] {
        { 0,  0, kW  , kH  },
        {-1, -1, kW+1, kH+1},
        { 1,  1, kW-1, kH-1},
        { 5,  5, 6   , 6   },
    };

    static const SkIPoint kDstPoints[] {
        { 0   ,  0   },
        { 1   ,  1   },
        { kW/2,  kH/4},
        { kW-1,  kH-1},
        { kW  ,  kH  },
        { kW+1,  kH+2},
        {-1   , -1   },
    };

    static const SkImageInfo kImageInfos[] {
        SkImageInfo::Make(kW, kH, kRGBA_8888_SkColorType, kPremul_SkAlphaType),
        SkImageInfo::Make(kW, kH, kBGRA_8888_SkColorType, kPremul_SkAlphaType)
    };

    SkAutoTMalloc<uint32_t> read(kW * kH);

    for (auto sOrigin : {kBottomLeft_GrSurfaceOrigin, kTopLeft_GrSurfaceOrigin}) {
        for (auto dOrigin : {kBottomLeft_GrSurfaceOrigin, kTopLeft_GrSurfaceOrigin}) {
            for (auto sRT : {true, false}) {
                for (auto dRT : {true, false}) {
                    for (auto srcRect : kSrcRects) {
                        for (auto dstPoint : kDstPoints) {
                            for (auto ii: kImageInfos) {
                                auto src = sk_gpu_test::MakeTextureProxyFromData(
                                        context, sRT, kW, kH, ii.colorType(), sOrigin,
                                        srcPixels.get(), kRowBytes);
                                auto dst = sk_gpu_test::MakeTextureProxyFromData(
                                        context, dRT, kW, kH, ii.colorType(), dOrigin,
                                        dstPixels.get(), kRowBytes);

                                // Should always work if the color type is RGBA, but may not work
                                // for BGRA
                                if (ii.colorType() == kRGBA_8888_SkColorType) {
                                    if (!src || !dst) {
                                        ERRORF(reporter,
                                               "Could not create surfaces for copy surface test.");
                                        continue;
                                    }
                                } else {
                                    GrPixelConfig config =
                                            SkColorType2GrPixelConfig(kBGRA_8888_SkColorType);
                                    if (!context->priv().caps()->isConfigTexturable(config)) {
                                        continue;
                                    }
                                    if (!src || !dst) {
                                        ERRORF(reporter,
                                               "Could not create surfaces for copy surface test.");
                                        continue;
                                    }
                                }

                                sk_sp<GrSurfaceContext> dstContext =
                                        context->priv().makeWrappedSurfaceContext(std::move(dst));

                                bool result = dstContext->copy(src.get(), srcRect, dstPoint);

                                bool expectedResult = true;
                                SkIPoint dstOffset = { dstPoint.fX - srcRect.fLeft,
                                                       dstPoint.fY - srcRect.fTop };
                                SkIRect copiedDstRect = SkIRect::MakeXYWH(dstPoint.fX,
                                                                          dstPoint.fY,
                                                                          srcRect.width(),
                                                                          srcRect.height());

                                SkIRect copiedSrcRect;
                                if (!copiedSrcRect.intersect(srcRect, SkIRect::MakeWH(kW, kH))) {
                                    expectedResult = false;
                                } else {
                                    // If the src rect was clipped, apply same clipping to each side
                                    // of copied dst rect.
                                    copiedDstRect.fLeft += copiedSrcRect.fLeft - srcRect.fLeft;
                                    copiedDstRect.fTop += copiedSrcRect.fTop - srcRect.fTop;
                                    copiedDstRect.fRight -= copiedSrcRect.fRight - srcRect.fRight;
                                    copiedDstRect.fBottom -= copiedSrcRect.fBottom -
                                                             srcRect.fBottom;
                                }
                                if (copiedDstRect.isEmpty() ||
                                    !copiedDstRect.intersect(SkIRect::MakeWH(kW, kH))) {
                                    expectedResult = false;
                                }
                                // To make the copied src rect correct we would apply any dst
                                // clipping back to the src rect, but we don't use it again so
                                // don't bother.
                                if (expectedResult != result) {
                                    ERRORF(reporter, "Expected return value %d from copySurface, "
                                           "got %d.", expectedResult, result);
                                    continue;
                                }

                                if (!expectedResult || !result) {
                                    continue;
                                }

                                sk_memset32(read.get(), 0, kW * kH);
                                if (!dstContext->readPixels(ii, read.get(), kRowBytes, 0, 0)) {
                                    ERRORF(reporter, "Error calling readPixels");
                                    continue;
                                }

                                bool abort = false;
                                // Validate that pixels inside copiedDstRect received the correct
                                // value from src and that those outside were not modified.
                                for (int y = 0; y < kH && !abort; ++y) {
                                    for (int x = 0; x < kW; ++x) {
                                        uint32_t r = read.get()[y * kW + x];
                                        if (copiedDstRect.contains(x, y)) {
                                            int sx = x - dstOffset.fX;
                                            int sy = y - dstOffset.fY;
                                            uint32_t s = srcPixels.get()[sy * kW + sx];
                                            if (s != r) {
                                                ERRORF(reporter, "Expected dst %d,%d to contain "
                                                       "0x%08x copied from src location %d,%d. Got "
                                                       "0x%08x", x, y, s, sx, sy, r);
                                                abort = true;
                                                break;
                                            }
                                        } else {
                                            uint32_t d = dstPixels.get()[y * kW + x];
                                            if (d != r) {
                                                ERRORF(reporter, "Expected dst %d,%d to be "
                                                       "unmodified (0x%08x). Got 0x%08x",
                                                       x, y, d, r);
                                                abort = true;
                                                break;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
