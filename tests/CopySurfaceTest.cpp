/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <initializer_list>
#include "Test.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrProxyProvider.h"
#include "GrResourceProvider.h"
#include "GrSurfaceContext.h"
#include "GrSurfaceProxy.h"
#include "GrTextureProxy.h"

#include "SkUtils.h"

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(CopySurface, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();
    static const int kW = 10;
    static const int kH = 10;
    static const size_t kRowBytes = sizeof(uint32_t) * kW;

    GrSurfaceDesc baseDesc;
    baseDesc.fConfig = kRGBA_8888_GrPixelConfig;
    baseDesc.fWidth = kW;
    baseDesc.fHeight = kH;

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

    const SkImageInfo ii = SkImageInfo::Make(kW, kH, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    SkAutoTMalloc<uint32_t> read(kW * kH);

    for (auto sOrigin : {kBottomLeft_GrSurfaceOrigin, kTopLeft_GrSurfaceOrigin}) {
        for (auto dOrigin : {kBottomLeft_GrSurfaceOrigin, kTopLeft_GrSurfaceOrigin}) {
            for (auto sFlags: {kRenderTarget_GrSurfaceFlag, kNone_GrSurfaceFlags}) {
                for (auto dFlags: {kRenderTarget_GrSurfaceFlag, kNone_GrSurfaceFlags}) {
                    for (auto srcRect : kSrcRects) {
                        for (auto dstPoint : kDstPoints) {
                            GrSurfaceDesc srcDesc = baseDesc;
                            srcDesc.fOrigin = sOrigin;
                            srcDesc.fFlags = sFlags;
                            GrSurfaceDesc dstDesc = baseDesc;
                            dstDesc.fOrigin = dOrigin;
                            dstDesc.fFlags = dFlags;

                            sk_sp<GrTextureProxy> src = proxyProvider->createTextureProxy(
                                             srcDesc, SkBudgeted::kNo, srcPixels.get(), kRowBytes);
                            sk_sp<GrTextureProxy> dst = proxyProvider->createTextureProxy(
                                             dstDesc, SkBudgeted::kNo, dstPixels.get(), kRowBytes);
                            if (!src || !dst) {
                                ERRORF(reporter,
                                       "Could not create surfaces for copy surface test.");
                                continue;
                            }

                            sk_sp<GrSurfaceContext> dstContext =
                                   context->contextPriv().makeWrappedSurfaceContext(std::move(dst));

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
                                // If the src rect was clipped, apply same clipping to each side of
                                // copied dst rect.
                                copiedDstRect.fLeft += copiedSrcRect.fLeft - srcRect.fLeft;
                                copiedDstRect.fTop += copiedSrcRect.fTop - srcRect.fTop;
                                copiedDstRect.fRight -= copiedSrcRect.fRight - srcRect.fRight;
                                copiedDstRect.fBottom -= copiedSrcRect.fBottom - srcRect.fBottom;
                            }
                            if (copiedDstRect.isEmpty() ||
                                !copiedDstRect.intersect(SkIRect::MakeWH(kW, kH))) {
                                expectedResult = false;
                            }
                            // To make the copied src rect correct we would apply any dst clipping
                            // back to the src rect, but we don't use it again so don't bother.
                            if (expectedResult != result) {
                                ERRORF(reporter, "Expected return value %d from copySurface, got "
                                       "%d.", expectedResult, result);
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
                            // Validate that pixels inside copiedDstRect received the correct value
                            // from src and that those outside were not modified.
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
                                            ERRORF(reporter, "Expected dst %d,%d to be unmodified ("
                                                   "0x%08x). Got 0x%08x", x, y, d, r);
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
#endif
