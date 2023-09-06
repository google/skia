/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

#include <initializer_list>

struct GrContextOptions;

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(crbug_1271431,
                                       reporter,
                                       context_info,
                                       CtsEnforcement::kApiLevel_T) {
    GrDirectContext* dc = context_info.directContext();

    // Make sure we don't get recycled render targets that already have stencil attachments.
    dc->freeGpuResources();

    SkImageInfo ii = SkImageInfo::Make({100, 100},
                                       kRGBA_8888_SkColorType,
                                       kPremul_SkAlphaType,
                                       nullptr);
    sk_sp<SkSurface> surfs[2]{SkSurfaces::RenderTarget(dc, skgpu::Budgeted::kYes, ii, 1, nullptr),
                              SkSurfaces::RenderTarget(dc, skgpu::Budgeted::kYes, ii, 1, nullptr)};

    // Make sure the surfaces' proxies are instantiated without stencil. Creating textures lazily
    // can invalidate the current tracked FBO since FBO state must be modified to during
    // GrGLRenderTarget creation.
    for (int i = 0; i < 2; ++i) {
        surfs[i]->getCanvas()->clear(SK_ColorWHITE);
        dc->flushAndSubmit(surfs[i].get(), GrSyncCpu::kNo);
    }

    auto drawWithStencilClip = [&](SkSurface& surf, SkColor color) {
        SkPath clip;
        clip.addCircle(50, 50, 50);
        clip.addCircle(50, 50, 10, SkPathDirection::kCCW);
        SkPaint paint;
        paint.setColor(color);
        surf.getCanvas()->clipPath(clip, false);
        surf.getCanvas()->drawRect(SkRect::MakeLTRB(0,0, 100, 100), paint);
    };

    // Use surfs[0] create to create a cached stencil buffer that is also sized for surfs[1].
    drawWithStencilClip(*surfs[0], SK_ColorRED);
    dc->flushAndSubmit(surfs[0].get(), GrSyncCpu::kNo);

    // Make sure surf[1]'s FBO is bound but without using draws that would attach stencil.
    surfs[1]->getCanvas()->clear(SK_ColorGREEN);
    dc->flushAndSubmit(surfs[1].get(), GrSyncCpu::kNo);

    // Now use stencil for clipping. We should now have the following properties:
    // 1) surf[1]'s FBO is already bound
    // 2) surf[1] doesn't have a stencil buffer
    // 3) There is an appropriately sized stencil buffer in the cache (used with surf[0]). This is
    //    important because creating a new stencil buffer will invalidate the bound FBO tracking.
    // The bug was that because the correct FBO was already bound we would not rebind and would
    // skip the lazy stencil attachment in GrGLRenderTarget. This would cause the clip to fail.
    drawWithStencilClip(*surfs[1], SK_ColorBLUE);

    // Check that four pixels that should have been clipped out of the blue draw are green.
    SkAutoPixmapStorage rb;
    rb.alloc(surfs[1]->imageInfo().makeWH(1, 1));
    for (int x : {5, 95}) {
        for (int y : {5, 95}) {
            if (!surfs[1]->readPixels(rb, x, y)) {
                ERRORF(reporter, "readback failed");
                return;
            }
            if (*rb.addr32() != SK_ColorGREEN) {
                ERRORF(reporter,
                       "Expected green at (%d, %d), instead got 0x%08x.",
                       x,
                       y,
                       *rb.addr32());
                return;
            }
        }
    }
}
