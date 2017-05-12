/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawOpTest.h"
#include "GrRenderTargetContext.h"
#include "GrRenderTargetContextPriv.h"
#include "GrSurfaceContextPriv.h"
#include "SkRandom.h"
#include "SkTypes.h"
#include "ops/GrMeshDrawOp.h"

#if GR_TEST_UTILS

#define DRAW_OP_TEST_EXTERN(Op) \
    extern std::unique_ptr<GrDrawOp> Op##__Test(GrPaint&&, SkRandom*, GrContext*, GrFSAAType);

#define LEGACY_MESH_DRAW_OP_TEST_EXTERN(Op) \
    extern std::unique_ptr<GrLegacyMeshDrawOp> Op##__Test(SkRandom*, GrContext*);

#define DRAW_OP_TEST_ENTRY(Op) Op##__Test

LEGACY_MESH_DRAW_OP_TEST_EXTERN(AAConvexPathOp);
LEGACY_MESH_DRAW_OP_TEST_EXTERN(AAFillRectOp);
LEGACY_MESH_DRAW_OP_TEST_EXTERN(AAFillRectOpLocalMatrix);
LEGACY_MESH_DRAW_OP_TEST_EXTERN(AAFlatteningConvexPathOp)
LEGACY_MESH_DRAW_OP_TEST_EXTERN(AAHairlineOp);
LEGACY_MESH_DRAW_OP_TEST_EXTERN(AAStrokeRectOp);
LEGACY_MESH_DRAW_OP_TEST_EXTERN(AnalyticRectOp);
LEGACY_MESH_DRAW_OP_TEST_EXTERN(CircleOp)
LEGACY_MESH_DRAW_OP_TEST_EXTERN(DashOp);
LEGACY_MESH_DRAW_OP_TEST_EXTERN(DefaultPathOp);
LEGACY_MESH_DRAW_OP_TEST_EXTERN(DIEllipseOp);
LEGACY_MESH_DRAW_OP_TEST_EXTERN(EllipseOp);
LEGACY_MESH_DRAW_OP_TEST_EXTERN(GrDrawAtlasOp);
LEGACY_MESH_DRAW_OP_TEST_EXTERN(NonAAStrokeRectOp);
LEGACY_MESH_DRAW_OP_TEST_EXTERN(RRectOp);
LEGACY_MESH_DRAW_OP_TEST_EXTERN(SmallPathOp);
LEGACY_MESH_DRAW_OP_TEST_EXTERN(TesselatingPathOp);
LEGACY_MESH_DRAW_OP_TEST_EXTERN(TextBlobOp);
LEGACY_MESH_DRAW_OP_TEST_EXTERN(VerticesOp);

DRAW_OP_TEST_EXTERN(NonAAFillRectOp)

const GrUserStencilSettings* GrGetRandomStencil(SkRandom* random, GrContext* context) {
    if (context->caps()->avoidStencilBuffers()) {
        return &GrUserStencilSettings::kUnused;
    }
    static constexpr GrUserStencilSettings kReads(
        GrUserStencilSettings::StaticInit<
            0x8080,
            GrUserStencilTest::kLess,
            0xffff,
            GrUserStencilOp::kKeep,
            GrUserStencilOp::kKeep,
            0xffff>()
    );
    static constexpr GrUserStencilSettings kWrites(
        GrUserStencilSettings::StaticInit<
            0xffff,
            GrUserStencilTest::kAlways,
            0xffff,
            GrUserStencilOp::kReplace,
            GrUserStencilOp::kReplace,
            0xffff>()
    );
    static constexpr GrUserStencilSettings kReadsAndWrites(
        GrUserStencilSettings::StaticInit<
            0x8000,
            GrUserStencilTest::kEqual,
            0x6000,
            GrUserStencilOp::kIncWrap,
            GrUserStencilOp::kInvert,
            0x77ff>()
    );

    static const GrUserStencilSettings* kStencilSettings[] = {
            &GrUserStencilSettings::kUnused,
            &kReads,
            &kWrites,
            &kReadsAndWrites,
    };
    return kStencilSettings[random->nextULessThan(SK_ARRAY_COUNT(kStencilSettings))];
}

void GrDrawRandomOp(SkRandom* random, GrRenderTargetContext* renderTargetContext, GrPaint&& paint) {
    GrContext* context = renderTargetContext->surfPriv().getContext();
    using MakeTestLegacyMeshDrawOpFn = std::unique_ptr<GrLegacyMeshDrawOp>(SkRandom*, GrContext*);
    static constexpr MakeTestLegacyMeshDrawOpFn* gLegacyFactories[] = {
        DRAW_OP_TEST_ENTRY(AAConvexPathOp),
        DRAW_OP_TEST_ENTRY(AAFillRectOp),
        DRAW_OP_TEST_ENTRY(AAFillRectOpLocalMatrix),
        DRAW_OP_TEST_ENTRY(AAFlatteningConvexPathOp),
        DRAW_OP_TEST_ENTRY(AAHairlineOp),
        DRAW_OP_TEST_ENTRY(AAStrokeRectOp),
        DRAW_OP_TEST_ENTRY(AnalyticRectOp),
        DRAW_OP_TEST_ENTRY(CircleOp),
        DRAW_OP_TEST_ENTRY(DashOp),
        DRAW_OP_TEST_ENTRY(DefaultPathOp),
        DRAW_OP_TEST_ENTRY(DIEllipseOp),
        DRAW_OP_TEST_ENTRY(EllipseOp),
        DRAW_OP_TEST_ENTRY(GrDrawAtlasOp),
        DRAW_OP_TEST_ENTRY(NonAAStrokeRectOp),
        DRAW_OP_TEST_ENTRY(RRectOp),
        DRAW_OP_TEST_ENTRY(SmallPathOp),
        DRAW_OP_TEST_ENTRY(TesselatingPathOp),
        DRAW_OP_TEST_ENTRY(TextBlobOp),
        DRAW_OP_TEST_ENTRY(VerticesOp)
    };

    using MakeDrawOpFn = std::unique_ptr<GrDrawOp>(GrPaint&&, SkRandom*, GrContext*, GrFSAAType);
    static constexpr MakeDrawOpFn* gFactories[] = {
        DRAW_OP_TEST_ENTRY(NonAAFillRectOp),
    };

    static constexpr size_t kTotal = SK_ARRAY_COUNT(gLegacyFactories) + SK_ARRAY_COUNT(gFactories);

    uint32_t index = random->nextULessThan(static_cast<uint32_t>(kTotal));
    if (index < SK_ARRAY_COUNT(gLegacyFactories)) {
        const GrUserStencilSettings* uss = GrGetRandomStencil(random, context);
        // We don't use kHW because we will hit an assertion if the render target is not
        // multisampled
        static constexpr GrAAType kAATypes[] = {GrAAType::kNone, GrAAType::kCoverage};
        GrAAType aaType = kAATypes[random->nextULessThan(SK_ARRAY_COUNT(kAATypes))];
        bool snapToCenters = random->nextBool();
        auto op = gLegacyFactories[index](random, context);
        SkASSERT(op);
        renderTargetContext->priv().testingOnly_addLegacyMeshDrawOp(
                std::move(paint), aaType, std::move(op), uss, snapToCenters);
    } else {
        auto op = gFactories[index - SK_ARRAY_COUNT(gLegacyFactories)](
                std::move(paint), random, context, renderTargetContext->fsaaType());
        SkASSERT(op);
        renderTargetContext->priv().testingOnly_addDrawOp(std::move(op));
    }
}
#endif
