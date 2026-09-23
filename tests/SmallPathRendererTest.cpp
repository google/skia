/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPathBuilder.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "src/gpu/ganesh/GrStyle.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "tests/Test.h"
#include "tools/gpu/ContextType.h"

static void only_allow_small(GrContextOptions* options) {
    options->fGpuPathRenderers = GpuPathRenderers::kSmall;
}

DEF_GANESH_TEST_FOR_CONTEXTS(SmallPathRenderer_crbug505876830,
                             skgpu::IsRenderingContext,
                             reporter,
                             ctxInfo,
                             only_allow_small,
                             CtsEnforcement::kNever) {
    auto ctx = ctxInfo.directContext();
    auto sdc = skgpu::ganesh::SurfaceDrawContext::Make(
                ctx, GrColorType::kRGBA_8888, nullptr, SkBackingFit::kExact,
                {100, 100}, SkSurfaceProps(), /*label=*/{});
    if (!sdc) {
        return;
    }
    sdc->clear(SK_PMColor4fBLACK);

    GrStyle style(SkStrokeRec::kFill_InitStyle);
    const SkPath smolpath = SkPathBuilder()
        .moveTo(-0.25000017881393432617f, -0.25000017881393432617f)
        .lineTo( 0.25000017881393432617f, -0.25000017881393432617f)
        .lineTo( 0.25000017881393432617f,  0.25000017881393432617f)
        .close()
        .detach();
    const SkMatrix m = SkMatrix::MakeAll(
                      -0.0f, 70368341861093281422320336896.0f, -590295810358705651712.0f,
                       2.0f,          17469300226849243136.0f,                      0.0f,
         868052350533632.0f,     4220791729285326409039872.0f,           3572.759765625f
    );

    // Passes if we don't assert.
    sdc->drawPath(nullptr, GrPaint(), GrAA::kYes, m, smolpath, style);
    ctx->flushAndSubmit();
}

#if !defined(SK_ENABLE_OPTIMIZE_SIZE)

#include "src/core/SkFloatBits.h"
#include "src/gpu/ganesh/geometry/GrStyledShape.h"
#include "src/gpu/ganesh/ops/SmallPathShapeData.h"

DEF_TEST(SmallPathShapeDataKey_DomainSeparation, reporter) {
    SkPath path = SkPath::Polygon({{-1, -1}, {1, -1}, {1, 1}, {-1, 1}}, true);
    GrStyledShape shape(path);
    skgpu::ganesh::SmallPathShapeDataKey sdfKey(shape, 32);
    skgpu::ganesh::SmallPathShapeDataKey bmKey(shape, SkMatrix::I());

    REPORTER_ASSERT(reporter, !(sdfKey == bmKey));
    REPORTER_ASSERT(reporter, sdfKey.data()[0] ==
            static_cast<uint32_t>(skgpu::ganesh::SmallPathShapeDataKey::Type::kSDF));
    REPORTER_ASSERT(reporter, bmKey.data()[0] ==
            static_cast<uint32_t>(skgpu::ganesh::SmallPathShapeDataKey::Type::kBitmap));
}

DEF_TEST(SmallPathShapeDataKey_CollisionReproduction, reporter) {
    // 1. Construct Shape B (Bitmap side)
    const float p0_x = SkBits2Float(0x28);
    const float p0_y = 0.45f;
    const float p1_x = 0.2f; // "free"
    const float p1_y = 0.45f;
    const float p2_x = 0.49f;
    const float p2_y = SkBits2Float(0x700000);
    const float p3_x = 65.0f;
    const float p3_y = SkBits2Float(0x6);
    const float p4_x = -1.0f;
    const float p4_y = 1.5f;

    SkPath pathB = SkPathBuilder()
        .moveTo(p0_x, p0_y)
        .lineTo(p1_x, p1_y)
        .lineTo(p2_x, p2_y)
        .quadTo(p3_x, p3_y, p4_x, p4_y)
        .detach();
    pathB.setFillType(SkPathFillType::kWinding);

    SkMatrix ctmB = SkMatrix::MakeAll(
        SkBits2Float(162), -1.0f, 0.0f,
        -1.0f, SkBits2Float(0x10C), 0.0f,
        0.0f, 0.0f, 1.0f
    );

    GrStyledShape shapeB(pathB);
    skgpu::ganesh::SmallPathShapeDataKey keyB(shapeB, ctmB);

    // 2. Construct Shape A (DF side)
    SkRect rectA = SkRect::MakeLTRB(-1.0f, -1.0f, 0.0f, SkBits2Float(0x110));
    SkVector radiiA[4] = {
        { SkBits2Float(4),    SkBits2Float(0x02010100) }, // UL
        { SkBits2Float(0x28), 0.45f },                    // UR
        { 0.2f,               0.45f },                    // LR
        { 0.49f,              SkBits2Float(0x700000)   }  // LL
    };
    SkRRect rrectA;
    rrectA.setRectRadii(rectA, radiiA);

    SkPaint strokePaint;
    strokePaint.setStyle(SkPaint::kStroke_Style);
    strokePaint.setStrokeWidth(1.5f);
    strokePaint.setStrokeJoin(SkPaint::kRound_Join);
    strokePaint.setStrokeCap(SkPaint::kButt_Cap);
    GrStyle styleA(strokePaint);

    GrStyledShape parentShapeA(rrectA, styleA);
    GrStyledShape shapeA = parentShapeA.applyStyle(GrStyle::Apply::kPathEffectAndStrokeRec, 65.0f);
    skgpu::ganesh::SmallPathShapeDataKey keyA(shapeA, 162);

    // Both keys have identical lengths (19 words: 1 word tag + 18 payload words)
    REPORTER_ASSERT(reporter, keyA.count32() == 19);
    REPORTER_ASSERT(reporter, keyB.count32() == 19);

    // Verify the 18 payload words match byte-for-byte!
    // Without the domain tag, this would have been an exact collision.
    int payloadDiff = memcmp(&keyA.data()[1], &keyB.data()[1], 18 * sizeof(uint32_t));
    REPORTER_ASSERT(reporter, payloadDiff == 0);

    // With the domain tag, keyA != keyB:
    REPORTER_ASSERT(reporter, !(keyA == keyB));
    REPORTER_ASSERT(reporter, keyA.data()[0] ==
            static_cast<uint32_t>(skgpu::ganesh::SmallPathShapeDataKey::Type::kSDF));
    REPORTER_ASSERT(reporter, keyB.data()[0] ==
            static_cast<uint32_t>(skgpu::ganesh::SmallPathShapeDataKey::Type::kBitmap));
}

#endif // !defined(SK_ENABLE_OPTIMIZE_SIZE)

