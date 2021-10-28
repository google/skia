/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkImageInfo.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrTypes.h"
#include "include/private/GrTypesPriv.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkOpts.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrSurfaceProxy.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/SurfaceFillContext.h"
#include "tests/Test.h"
#include "tools/gpu/GrContextFactory.h"
#include "tools/gpu/ProxyUtils.h"

#include <initializer_list>
#include <utility>

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(CopySurface, reporter, ctxInfo) {
    auto dContext = ctxInfo.directContext();

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
            for (auto sRenderable : {GrRenderable::kYes, GrRenderable::kNo}) {
                for (auto dRenderable : {GrRenderable::kYes, GrRenderable::kNo}) {
                    for (const SkIRect& srcRect : kSrcRects) {
                        for (const SkIPoint& dstPoint : kDstPoints) {
                            for (const SkImageInfo& ii: kImageInfos) {
                                GrCPixmap srcPM(ii, srcPixels.get(), kRowBytes);
                                GrPixmap  dstPM(ii, dstPixels.get(), kRowBytes);
                                auto srcView = sk_gpu_test::MakeTextureProxyViewFromData(
                                        dContext, sRenderable, sOrigin, srcPM);
                                auto dstView = sk_gpu_test::MakeTextureProxyViewFromData(
                                        dContext, dRenderable, dOrigin, dstPM);

                                // Should always work if the color type is RGBA, but may not work
                                // for BGRA
                                if (ii.colorType() == kRGBA_8888_SkColorType) {
                                    if (!srcView || !dstView) {
                                        ERRORF(reporter,
                                               "Could not create surfaces for copy surface test.");
                                        continue;
                                    }
                                } else {
                                    if (!dContext->defaultBackendFormat(
                                            kBGRA_8888_SkColorType, GrRenderable::kNo).isValid()) {
                                        continue;
                                    }
                                    if (!srcView || !dstView) {
                                        ERRORF(reporter,
                                               "Could not create surfaces for copy surface test.");
                                        continue;
                                    }
                                }

                                auto dstContext = dContext->priv().makeSC(std::move(dstView),
                                                                          ii.colorInfo());

                                bool result = false;
                                if (sOrigin == dOrigin) {
                                    result = dstContext->testCopy(srcView.refProxy(),
                                                                  srcRect,
                                                                  dstPoint);
                                } else if (dRenderable == GrRenderable::kYes) {
                                    SkASSERT(dstContext->asFillContext());
                                    result = dstContext->asFillContext()->blitTexture(
                                            std::move(srcView), srcRect, dstPoint);
                                }

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
                                if (sOrigin != dOrigin && dRenderable == GrRenderable::kNo) {
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
                                GrPixmap readPM(ii, read.get(), kRowBytes);
                                if (!dstContext->readPixels(dContext, readPM, {0, 0})) {
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
