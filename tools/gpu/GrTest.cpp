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
#include "include/private/SkTo.h"
#include "src/core/SkMathPriv.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrDrawOpAtlas.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrGpuResourceCacheAccess.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetProxy.h"
#include "src/gpu/GrResourceCache.h"
#include "src/gpu/GrSemaphore.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/ccpr/GrCoverageCountingPathRenderer.h"
#include "src/gpu/ops/GrMeshDrawOp.h"
#include "src/gpu/text/GrStrikeCache.h"
#include "src/gpu/text/GrTextBlobCache.h"
#include "src/image/SkImage_Gpu.h"

#include <algorithm>

///////////////////////////////////////////////////////////////////////////////

void GrResourceCache::changeTimestamp(uint32_t newTimestamp) { fTimestamp = newTimestamp; }

#ifdef SK_DEBUG
int GrResourceCache::countUniqueKeysWithTag(const char* tag) const {
    int count = 0;
    fUniqueHash.foreach([&](const GrGpuResource& resource){
        if (0 == strcmp(tag, resource.getUniqueKey().tag())) {
            ++count;
        }
    });
    return count;
}
#endif

//////////////////////////////////////////////////////////////////////////////

#define DRAW_OP_TEST_EXTERN(Op) \
    extern GrOp::Owner Op##__Test(GrPaint&&, SkRandom*, \
                                    GrRecordingContext*, int numSamples)
#define DRAW_OP_TEST_ENTRY(Op) Op##__Test

DRAW_OP_TEST_EXTERN(AAConvexPathOp);
DRAW_OP_TEST_EXTERN(AAFlatteningConvexPathOp);
DRAW_OP_TEST_EXTERN(AAHairlineOp);
DRAW_OP_TEST_EXTERN(AAStrokeRectOp);
DRAW_OP_TEST_EXTERN(CircleOp);
DRAW_OP_TEST_EXTERN(DashOp);
DRAW_OP_TEST_EXTERN(DefaultPathOp);
DRAW_OP_TEST_EXTERN(DIEllipseOp);
DRAW_OP_TEST_EXTERN(EllipseOp);
DRAW_OP_TEST_EXTERN(FillRectOp);
DRAW_OP_TEST_EXTERN(GrAtlasTextOp);
DRAW_OP_TEST_EXTERN(DrawAtlasOp);
DRAW_OP_TEST_EXTERN(DrawVerticesOp);
DRAW_OP_TEST_EXTERN(NonAALatticeOp);
DRAW_OP_TEST_EXTERN(NonAAStrokeRectOp);
DRAW_OP_TEST_EXTERN(ShadowRRectOp);
DRAW_OP_TEST_EXTERN(SmallPathOp);
DRAW_OP_TEST_EXTERN(RegionOp);
DRAW_OP_TEST_EXTERN(RRectOp);
DRAW_OP_TEST_EXTERN(TriangulatingPathOp);
DRAW_OP_TEST_EXTERN(TextureOp);

void GrDrawRandomOp(SkRandom* random, GrSurfaceDrawContext* surfaceDrawContext, GrPaint&& paint) {
    auto context = surfaceDrawContext->recordingContext();
    using MakeDrawOpFn = GrOp::Owner (GrPaint&&, SkRandom*,
                                      GrRecordingContext*, int numSamples);
    static constexpr MakeDrawOpFn* gFactories[] = {
            DRAW_OP_TEST_ENTRY(AAConvexPathOp),
            DRAW_OP_TEST_ENTRY(AAFlatteningConvexPathOp),
            DRAW_OP_TEST_ENTRY(AAHairlineOp),
            DRAW_OP_TEST_ENTRY(AAStrokeRectOp),
            DRAW_OP_TEST_ENTRY(CircleOp),
            DRAW_OP_TEST_ENTRY(DashOp),
            DRAW_OP_TEST_ENTRY(DefaultPathOp),
            DRAW_OP_TEST_ENTRY(DIEllipseOp),
            DRAW_OP_TEST_ENTRY(EllipseOp),
            DRAW_OP_TEST_ENTRY(FillRectOp),
            DRAW_OP_TEST_ENTRY(GrAtlasTextOp),
            DRAW_OP_TEST_ENTRY(DrawAtlasOp),
            DRAW_OP_TEST_ENTRY(DrawVerticesOp),
            DRAW_OP_TEST_ENTRY(NonAALatticeOp),
            DRAW_OP_TEST_ENTRY(NonAAStrokeRectOp),
            DRAW_OP_TEST_ENTRY(ShadowRRectOp),
            DRAW_OP_TEST_ENTRY(SmallPathOp),
            DRAW_OP_TEST_ENTRY(RegionOp),
            DRAW_OP_TEST_ENTRY(RRectOp),
            DRAW_OP_TEST_ENTRY(TriangulatingPathOp),
            DRAW_OP_TEST_ENTRY(TextureOp),
    };

    static constexpr size_t kTotal = SK_ARRAY_COUNT(gFactories);
    uint32_t index = random->nextULessThan(static_cast<uint32_t>(kTotal));
    auto op = gFactories[index](
            std::move(paint), random, context, surfaceDrawContext->numSamples());

    // Creating a GrAtlasTextOp my not produce an op if for example, it is totally outside the
    // render target context.
    if (op) {
        surfaceDrawContext->addDrawOp(std::move(op));
    }
}
