/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkString.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkMathPriv.h"
#include "src/gpu/ganesh/GrClip.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrDrawOpAtlas.h"
#include "src/gpu/ganesh/GrDrawingManager.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrGpuResourceCacheAccess.h"
#include "src/gpu/ganesh/GrMemoryPool.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrRenderTargetProxy.h"
#include "src/gpu/ganesh/GrResourceCache.h"
#include "src/gpu/ganesh/GrSemaphore.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "src/image/SkImage_Gpu.h"
#include "src/text/gpu/StrikeCache.h"
#include "src/text/gpu/TextBlobRedrawCoordinator.h"

#include <algorithm>

//////////////////////////////////////////////////////////////////////////////

#define DRAW_OP_TEST_EXTERN(Op)                                                                 \
    extern GrOp::Owner Op##__Test(GrPaint&&,                                                    \
                                  SkRandom*,                                                    \
                                  GrRecordingContext*,                                          \
                                  skgpu::v1::SurfaceDrawContext*,                               \
                                  int)
#define DRAW_OP_TEST_ENTRY(Op) Op##__Test

DRAW_OP_TEST_EXTERN(AAConvexPathOp);
DRAW_OP_TEST_EXTERN(AAFlatteningConvexPathOp);
DRAW_OP_TEST_EXTERN(AAHairlineOp);
DRAW_OP_TEST_EXTERN(AAStrokeRectOp);
DRAW_OP_TEST_EXTERN(AtlasTextOp);
#if !defined(SK_ENABLE_OPTIMIZE_SIZE)
DRAW_OP_TEST_EXTERN(ButtCapDashedCircleOp);
DRAW_OP_TEST_EXTERN(CircleOp);
#endif
DRAW_OP_TEST_EXTERN(DashOpImpl);
DRAW_OP_TEST_EXTERN(DefaultPathOp);
DRAW_OP_TEST_EXTERN(DrawAtlasOp);
#if !defined(SK_ENABLE_OPTIMIZE_SIZE)
DRAW_OP_TEST_EXTERN(DIEllipseOp);
DRAW_OP_TEST_EXTERN(EllipseOp);
#endif
DRAW_OP_TEST_EXTERN(FillRectOp);
DRAW_OP_TEST_EXTERN(NonAALatticeOp);
DRAW_OP_TEST_EXTERN(NonAAStrokeRectOp);
DRAW_OP_TEST_EXTERN(RegionOp);
#if !defined(SK_ENABLE_OPTIMIZE_SIZE)
DRAW_OP_TEST_EXTERN(RRectOp);
#endif
DRAW_OP_TEST_EXTERN(ShadowRRectOp);
#if !defined(SK_ENABLE_OPTIMIZE_SIZE)
DRAW_OP_TEST_EXTERN(SmallPathOp);
#endif
DRAW_OP_TEST_EXTERN(TextureOpImpl);
#if !defined(SK_ENABLE_OPTIMIZE_SIZE)
DRAW_OP_TEST_EXTERN(TriangulatingPathOp);
#endif

void GrDrawRandomOp(SkRandom* random, skgpu::v1::SurfaceDrawContext* sdc, GrPaint&& paint) {
    auto rContext = sdc->recordingContext();
    using MakeDrawOpFn = GrOp::Owner (GrPaint&&,
                                      SkRandom*,
                                      GrRecordingContext*,
                                      skgpu::v1::SurfaceDrawContext*,
                                      int numSamples);
    static constexpr MakeDrawOpFn* gFactories[] = {
            DRAW_OP_TEST_ENTRY(AAConvexPathOp),
            DRAW_OP_TEST_ENTRY(AAFlatteningConvexPathOp),
            DRAW_OP_TEST_ENTRY(AAHairlineOp),
            DRAW_OP_TEST_ENTRY(AAStrokeRectOp),
            DRAW_OP_TEST_ENTRY(AtlasTextOp),
#if !defined(SK_ENABLE_OPTIMIZE_SIZE)
            DRAW_OP_TEST_ENTRY(ButtCapDashedCircleOp),
            DRAW_OP_TEST_ENTRY(CircleOp),
#endif
            DRAW_OP_TEST_ENTRY(DashOpImpl),
            DRAW_OP_TEST_ENTRY(DefaultPathOp),
            DRAW_OP_TEST_ENTRY(DrawAtlasOp),
#if !defined(SK_ENABLE_OPTIMIZE_SIZE)
            DRAW_OP_TEST_ENTRY(DIEllipseOp),
            DRAW_OP_TEST_ENTRY(EllipseOp),
#endif
            DRAW_OP_TEST_ENTRY(FillRectOp),
            DRAW_OP_TEST_ENTRY(NonAALatticeOp),
            DRAW_OP_TEST_ENTRY(NonAAStrokeRectOp),
            DRAW_OP_TEST_ENTRY(RegionOp),
#if !defined(SK_ENABLE_OPTIMIZE_SIZE)
            DRAW_OP_TEST_ENTRY(RRectOp),
#endif
            DRAW_OP_TEST_ENTRY(ShadowRRectOp),
#if !defined(SK_ENABLE_OPTIMIZE_SIZE)
            DRAW_OP_TEST_ENTRY(SmallPathOp),
#endif
            DRAW_OP_TEST_ENTRY(TextureOpImpl),
#if !defined(SK_ENABLE_OPTIMIZE_SIZE)
            DRAW_OP_TEST_ENTRY(TriangulatingPathOp),
#endif
    };

    static constexpr size_t kTotal = std::size(gFactories);
    uint32_t index = random->nextULessThan(static_cast<uint32_t>(kTotal));
    auto op = gFactories[index](std::move(paint),
                                random,
                                rContext,
                                sdc,
                                sdc->numSamples());

    // Creating a GrAtlasTextOp my not produce an op if for example, it is totally outside the
    // render target context.
    if (op) {
        sdc->addDrawOp(std::move(op));
    }
}
