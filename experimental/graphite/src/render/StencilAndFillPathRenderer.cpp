/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/Renderer.h"

#include "experimental/graphite/src/ContextUtils.h"
#include "experimental/graphite/src/DrawWriter.h"
#include "experimental/graphite/src/UniformManager.h"
#include "experimental/graphite/src/geom/Shape.h"
#include "experimental/graphite/src/geom/Transform_graphite.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkRect.h"
#include "src/gpu/BufferWriter.h"

namespace skgpu {

namespace {

// TODO: These settings are actually shared by tessellating path renderers, so will be exposed later

// Returns the stencil settings to use for a standard Redbook "fill" pass. Allows non-zero
// stencil values to pass and write a color, and resets the stencil value back to zero; discards
// immediately on stencil values of zero (or does the inverse of these operations when the path
// requires filling everything else).
constexpr DepthStencilSettings cover_settings(bool inverse) {
    // Resets non-zero bits to 0, passes when not zero. We set depthFail to kZero because if we
    // encounter that case, the kNotEqual=0 stencil test passed, so it does need to be set back to 0
    // and the dsPass op won't be run. In practice, since the stencil steps will fail the same depth
    // test, the stencil value will likely not be non-zero, but best to be explicit.
    constexpr DepthStencilSettings::Face kNormal = {
        /*stencilFail=*/   StencilOp::kKeep,
        /*depthFail=*/     StencilOp::kZero,
        /*dsPass=*/        StencilOp::kZero,
        /*stencilCompare=*/CompareOp::kEqual, // TODO: Actually kNotEqual once we have stencil steps
        /*readMask=*/      0xffffffff,
        /*writeMask=*/     0xffffffff
    };

    // Resets non-zero bits to 0, passes when zero
    constexpr DepthStencilSettings::Face kInverted = {
        /*stencilFail=*/   StencilOp::kZero,
        /*depthFail=*/     StencilOp::kKeep,
        /*dsPass=*/        StencilOp::kKeep,
        /*stencilCompare=*/CompareOp::kEqual,
        /*readMask=*/      0xffffffff,
        /*writeMask=*/     0xffffffff
    };

    // Always use ref = 0, enabled depth writes, and greater depth test, both
    // front and back use the same stencil settings.
    constexpr DepthStencilSettings kNormalDSS = {
        /*frontStencil=*/kNormal,
        /*frontStencil=*/kNormal,
        /*refValue=*/    0,
        /*stencilTest=*/ true,
        /*depthCompare=*/CompareOp::kAlways, // kGreater once steps know the right depth value
        /*depthTest=*/   true,
        /*depthWrite=*/  true
    };
    constexpr DepthStencilSettings kInvertedDSS = {
        /*frontStencil=*/kInverted,
        /*backStencil=*/ kInverted,
        /*refValue=*/    0,
        /*stencilTest=*/ true,
        /*depthCompare=*/CompareOp::kAlways, // kGreater once steps know the right depth value
        /*depthTest=*/   true,
        /*depthWrite=*/  true
    };
    return inverse ? kInvertedDSS : kNormalDSS;
}

// TODO: Hand off to csmartdalton, this should roughly correspond to the fStencilFanProgram and
// simple triangulator shader stage of the skgpu::v1::PathStencilCoverOp
/*
class StencilFanRenderStep : public RenderStep {
public:
    StencilFanRenderStep() {}

    ~StencilFanRenderStep() override {}

    const char* name()            const override { return "stencil-fan"; }
    bool        requiresStencil() const override { return true; }
    bool        requiresMSAA()    const override { return true; }
    bool        performsShading() const override { return false; }

private:
};
*/

// TODO: Hand off to csmartdalton, this should roughly correspond to the fStencilPathProgram stage
// of skgpu::v1::PathStencilCoverOp using the PathCurveTessellator
/*
class StencilCurvesRenderStep : public RenderStep {
public:
    StencilCurvesRenderStep() {}

    ~StencilCurvesRenderStep() override {}

    const char* name()            const override { return "stencil-curves"; }
    bool        requiresStencil() const override { return true;  }
    bool        requiresMSAA()    const override { return true;  }
    bool        performsShading() const override { return false; }

private:
};
*/

// TODO: Hand off to csmartdalton, this should roughly correspond to the fCoverBBoxProgram stage
// of skgpu::v1::PathStencilCoverOp.
class FillBoundsRenderStep final : public RenderStep {
public:
    // TODO: Will need to add kRequiresStencil when we support specifying stencil settings and
    // the Renderer includes the stenciling step first.
    FillBoundsRenderStep(bool inverseFill)
            : RenderStep(Flags::kPerformsShading,
                         /*uniforms=*/{},
                         PrimitiveType::kTriangles,
                         cover_settings(inverseFill),
                         /*vertexAttrs=*/{{"position", VertexAttribType::kFloat3, SLType::kFloat3}},
                         /*instanceAttrs=*/{})
            , fInverseFill(inverseFill) {}

    ~FillBoundsRenderStep() override {}

    const char* name() const override { return "fill-bounds"; }

    const char* vertexSkSL() const override {
        return "     float4 devPosition = float4(position.xy, 0.0, position.z);\n";
    }

    void writeVertices(DrawWriter* writer,
                       const SkIRect& bounds,
                       const Transform& localToDevice,
                       const Shape& shape) const override {
        SkV4 devPoints[4]; // ordered TL, TR, BR, BL

        if (fInverseFill) {
            // TODO: When we handle local coords, we'd need to map these corners by the inverse.
            devPoints[0] = {(float) bounds.fLeft,  (float) bounds.fTop,    0.f, 1.f};
            devPoints[1] = {(float) bounds.fRight, (float) bounds.fTop,    0.f, 1.f};
            devPoints[2] = {(float) bounds.fRight, (float) bounds.fBottom, 0.f, 1.f};
            devPoints[3] = {(float) bounds.fLeft,  (float) bounds.fBottom, 0.f, 1.f};
        } else {
            localToDevice.mapPoints(shape.bounds(), devPoints);
        }

        auto vw = writer->appendVertices(6);
        vw << devPoints[0].x << devPoints[0].y << devPoints[0].w // TL
           << devPoints[3].x << devPoints[3].y << devPoints[3].w // BL
           << devPoints[1].x << devPoints[1].y << devPoints[1].w // TR
           << devPoints[1].x << devPoints[1].y << devPoints[1].w // TR
           << devPoints[3].x << devPoints[3].y << devPoints[3].w // BL
           << devPoints[2].x << devPoints[2].y << devPoints[2].w;// BR
    }

    sk_sp<UniformData> writeUniforms(Layout layout,
                                     const SkIRect&,
                                     const Transform& localToDevice,
                                     const Shape&) const override {
        // Positions are pre-transformed on the CPU so no uniforms needed
        return nullptr;
    }

private:
    const bool fInverseFill;
};

} // anonymous namespace

const Renderer& Renderer::StencilAndFillPath(SkPathFillType fillType) {
    // TODO: Uncomment and include in kRenderer to draw flattened paths instead of bboxes
    // static const StencilFanRenderStep kStencilFan;
    // TODO: Uncomment and include in kRenderer to draw curved paths
    // static const StencilCurvesRenderStep kStencilCurves;
    // TODO: This could move into a header and be reused across renderers
    static const FillBoundsRenderStep kFill{false};
    static const FillBoundsRenderStep kInverseFill{true};

    // TODO: Combine these two with stencil steps for winding and even-odd.
    static const Renderer kFillRenderer("stencil-and-fill[]", &kFill);
    static const Renderer kInverseFillRenderer("stencil-and-fill[inverse]", &kInverseFill);

    switch (fillType) {
        case SkPathFillType::kWinding: [[fallthrough]];
        case SkPathFillType::kEvenOdd:
            return kFillRenderer;
        case SkPathFillType::kInverseWinding: [[fallthrough]];
        case SkPathFillType::kInverseEvenOdd:
            return kInverseFillRenderer;
    }
    SkUNREACHABLE;
}

} // namespace skgpu
