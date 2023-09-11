/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/private/SkColorData.h"
#include "src/core/SkCanvasPriv.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/GrCanvas.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/SurfaceFillContext.h"

namespace skiagm {

// Size of each clear
static constexpr int kSize = 64;

DEF_SIMPLE_GPU_GM_CAN_FAIL(clear_swizzle, rContext, canvas, errorMsg, 6*kSize, 2*kSize) {
    if (rContext->abandoned()) {
        *errorMsg = GM::kErrorMsg_DrawSkippedGpuOnly;
        return DrawResult::kSkip;
    }

    auto sfc = skgpu::ganesh::TopDeviceSurfaceFillContext(canvas);
    if (!sfc) {
        *errorMsg = GM::kErrorMsg_DrawSkippedGpuOnly;
        return DrawResult::kSkip;
    }

    auto make_offscreen = [&](const SkISize dimensions) {
        skgpu::Swizzle readSwizzle  = skgpu::Swizzle::Concat(sfc->readSwizzle(),
                                                             skgpu::Swizzle{"bgra"});
        skgpu::Swizzle writeSwizzle = skgpu::Swizzle::Concat(sfc->readSwizzle(),
                                                             skgpu::Swizzle{"bgra"});
        return rContext->priv().makeSFC(kPremul_SkAlphaType,
                                        sfc->colorInfo().refColorSpace(),
                                        dimensions,
                                        SkBackingFit::kExact,
                                        sfc->asSurfaceProxy()->backendFormat(),
                                        /* sample count*/ 1,
                                        skgpu::Mipmapped::kNo,
                                        sfc->asSurfaceProxy()->isProtected(),
                                        readSwizzle,
                                        writeSwizzle,
                                        kTopLeft_GrSurfaceOrigin,
                                        skgpu::Budgeted::kYes,
                                        /*label=*/{});
    };

    struct {
        SkIRect rect;
        SkPMColor4f color;
    } clears[] {
            {{    0,     0,   kSize,   kSize}, {1, 0, 0, 1}},
            {{kSize,     0, 2*kSize,   kSize}, {0, 1, 0, 1}},
            {{    0, kSize,   kSize, 2*kSize}, {0, 0, 1, 1}},
            {{kSize, kSize, 2*kSize, 2*kSize}, {1, 0, 1, 1}},
    };

    // onscreen for reference
    for (const auto& c : clears) {
        sfc->clear(c.rect, c.color);
    }

    // partial clear offscreen
    auto offscreen = make_offscreen({2*kSize, 2*kSize});
    for (const auto& c : clears) {
        offscreen->clear(c.rect, c.color);
    }
    sfc->blitTexture(offscreen->readSurfaceView(),
                     SkIRect::MakeSize({2*kSize, 2*kSize}),
                     SkIPoint{2*kSize, 0});

    // full offscreen clears
    for (const auto& c : clears) {
        offscreen = make_offscreen(c.rect.size());
        offscreen->clear(SkIRect::MakeSize(c.rect.size()), c.color);
        sfc->blitTexture(offscreen->readSurfaceView(),
                         SkIRect::MakeSize(offscreen->dimensions()),
                         c.rect.topLeft() + SkIPoint{4*kSize, 0});
    }

    return DrawResult::kOk;
}

} // namespace skiagm
