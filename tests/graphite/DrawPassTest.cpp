/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/DrawList.h"
#include "src/gpu/graphite/DrawPass.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/RendererProvider.h"

namespace skgpu::graphite {

// Tests that creating a draw pass that fails a dst copy doesn't result in a segfault.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(DrawPassTestFailedDstCopy,
                                   reporter,
                                   context,
                                   CtsEnforcement::kNextRelease) {
    const Caps* caps = context->priv().caps();
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    // Define a paint that requires a dst copy.
    SkPaint paint;
    PaintParams paintParams{paint, nullptr, nullptr, DstReadRequirement::kTextureCopy, false};

    // Define a draw that uses the paint, but is larger than the max texture size. In this case the
    // dst copy will fail.
    const SkIRect drawSize = SkIRect::MakeWH(caps->maxTextureSize() + 1, 1);
    std::unique_ptr<DrawList> drawList = std::make_unique<DrawList>();
    drawList->recordDraw(recorder->priv().rendererProvider()->analyticRRect(),
                         Transform::Identity(),
                         Geometry(Shape(SkRect::Make(drawSize))),
                         Clip(Rect::Infinite(), Rect::Infinite(), drawSize, nullptr),
                         DrawOrder(DrawOrder::kClearDepth.next()),
                         &paintParams,
                         nullptr);

    // Attempt to make a draw pass with the draw.
    static constexpr SkISize targetSize = SkISize::Make(1, 1);
    static constexpr SkColorType targetColorType = kN32_SkColorType;
    static constexpr SkAlphaType targetAlphaType = kPremul_SkAlphaType;
    const SkImageInfo targetInfo = SkImageInfo::Make(targetSize, targetColorType, targetAlphaType);
    sk_sp<TextureProxy> target = TextureProxy::Make(
            caps,
            targetSize,
            caps->getDefaultSampledTextureInfo(
                    targetColorType, Mipmapped::kNo, Protected::kNo, Renderable::kYes),
            Budgeted::kNo);
    std::unique_ptr<DrawPass> drawPass = DrawPass::Make(recorder.get(),
                                                        std::move(drawList),
                                                        target,
                                                        targetInfo,
                                                        {LoadOp::kClear, StoreOp::kStore},
                                                        {0.0f, 0.0f, 0.0f, 0.0f});

    // Make sure creating the draw pass failed.
    REPORTER_ASSERT(reporter, !drawPass);
}

}  // namespace skgpu::graphite
