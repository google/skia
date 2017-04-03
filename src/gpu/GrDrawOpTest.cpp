/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawOpTest.h"
#include "SkRandom.h"
#include "SkTypes.h"
#include "ops/GrMeshDrawOp.h"

#if GR_TEST_UTILS

#define DRAW_OP_TEST_EXTERN(Op) \
    extern std::unique_ptr<GrLegacyMeshDrawOp> Op##__Test(SkRandom*, GrContext* context);

#define DRAW_OP_TEST_ENTRY(Op) Op##__Test

DRAW_OP_TEST_EXTERN(AAConvexPathOp);
DRAW_OP_TEST_EXTERN(AAFillRectOp);
DRAW_OP_TEST_EXTERN(AAFillRectOpLocalMatrix);
DRAW_OP_TEST_EXTERN(AAFlatteningConvexPathOp)
DRAW_OP_TEST_EXTERN(AAHairlineOp);
DRAW_OP_TEST_EXTERN(AAStrokeRectOp);
DRAW_OP_TEST_EXTERN(AnalyticRectOp);
DRAW_OP_TEST_EXTERN(DashOp);
DRAW_OP_TEST_EXTERN(DefaultPathOp);
DRAW_OP_TEST_EXTERN(CircleOp);
DRAW_OP_TEST_EXTERN(DIEllipseOp);
DRAW_OP_TEST_EXTERN(EllipseOp);
DRAW_OP_TEST_EXTERN(GrDrawAtlasOp);
DRAW_OP_TEST_EXTERN(NonAAStrokeRectOp);
DRAW_OP_TEST_EXTERN(RRectOp);
DRAW_OP_TEST_EXTERN(SmallPathOp);
DRAW_OP_TEST_EXTERN(TesselatingPathOp);
DRAW_OP_TEST_EXTERN(TextBlobOp);
DRAW_OP_TEST_EXTERN(VerticesOp);

std::unique_ptr<GrLegacyMeshDrawOp> GrRandomDrawOp(SkRandom* random, GrContext* context) {
    using MakeTestDrawOpFn =
            std::unique_ptr<GrLegacyMeshDrawOp>(SkRandom * random, GrContext * context);
    static constexpr MakeTestDrawOpFn* gFactories[] = {
        DRAW_OP_TEST_ENTRY(AAConvexPathOp),
        DRAW_OP_TEST_ENTRY(AAFillRectOp),
        DRAW_OP_TEST_ENTRY(AAFillRectOpLocalMatrix),
        DRAW_OP_TEST_ENTRY(AAFlatteningConvexPathOp),
        DRAW_OP_TEST_ENTRY(AAHairlineOp),
        DRAW_OP_TEST_ENTRY(AAStrokeRectOp),
        DRAW_OP_TEST_ENTRY(AnalyticRectOp),
        DRAW_OP_TEST_ENTRY(DashOp),
        DRAW_OP_TEST_ENTRY(DefaultPathOp),
        DRAW_OP_TEST_ENTRY(CircleOp),
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

    uint32_t index = random->nextULessThan(static_cast<uint32_t>(SK_ARRAY_COUNT(gFactories)));
    return gFactories[index](random, context);
}
#endif
