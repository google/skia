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
#include "src/gpu/tessellate/MiddleOutPolygonTriangulator.h"

namespace skgpu {

namespace {

// TODO: These settings are actually shared by tessellating path renderers, so will be exposed later

// Returns the stencil settings to use for a standard Redbook "stencil" pass.
constexpr DepthStencilSettings fillrule_settings(bool evenOdd) {
    // Increments clockwise triangles and decrements counterclockwise. Used for "winding" fill.
    constexpr DepthStencilSettings::Face kIncCW = {
        /*stencilFail=*/   StencilOp::kKeep,
        /*depthFail=*/     StencilOp::kKeep,
        /*dsPass=*/        StencilOp::kIncWrap,
        /*stencilCompare=*/CompareOp::kAlways,
        /*readMask=*/      0xffffffff,
        /*writeMask=*/     0xffffffff
    };
    constexpr DepthStencilSettings::Face kDecCCW = {
        /*stencilFail=*/   StencilOp::kKeep,
        /*depthFail=*/     StencilOp::kKeep,
        /*dsPass=*/        StencilOp::kDecWrap,
        /*stencilCompare=*/CompareOp::kAlways,
        /*readMask=*/      0xffffffff,
        /*writeMask=*/     0xffffffff
    };

    // Toggles the bottom stencil bit. Used for "even-odd" fill.
    constexpr DepthStencilSettings::Face kToggle = {
        /*stencilFail=*/   StencilOp::kKeep,
        /*depthFail=*/     StencilOp::kKeep,
        /*dsPass=*/        StencilOp::kInvert,
        /*stencilCompare=*/CompareOp::kAlways,
        /*readMask=*/      0xffffffff,
        /*writeMask=*/     0x00000001
    };

    // Always use ref = 0, disable depths, but still use greater depth test.
    constexpr DepthStencilSettings kWindingFill = {
        /*frontStencil=*/kIncCW,
        /*backStencil=*/ kDecCCW,
        /*refValue=*/    0,
        /*stencilTest=*/ true,
        /*depthCompare=*/CompareOp::kAlways, // kGreater once steps know the right depth value
        /*depthTest=*/   true,
        /*depthWrite=*/  false
    };
    constexpr DepthStencilSettings kEvenOddFill = {
        /*frontStencil=*/kToggle,
        /*backStencil=*/ kToggle,
        /*refValue=*/    0,
        /*stencilTest=*/ true,
        /*depthCompare=*/CompareOp::kAlways, // kGreater once steps know the right depth value
        /*depthTest=*/   true,
        /*depthWrite=*/  false
    };

    return evenOdd ? kEvenOddFill : kWindingFill;
}

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
        /*stencilCompare=*/CompareOp::kNotEqual,
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

class StencilFanRenderStep final : public RenderStep {
public:
    StencilFanRenderStep(bool evenOdd)
            : RenderStep(Flags::kRequiresMSAA,
                         /*uniforms=*/{},
                         PrimitiveType::kTriangles,
                         fillrule_settings(evenOdd),
                         /*vertexAttrs=*/{{"position", VertexAttribType::kFloat3, SLType::kFloat3}},
                         /*instanceAttrs=*/{}) {}

    ~StencilFanRenderStep() override {}

    const char* name() const override { return "stencil-fan"; }

    const char* vertexSkSL() const override {
        return "     float4 devPosition = float4(position.xy, 0.0, position.z);\n";
    }

    void writeVertices(DrawWriter* writer,
                       const SkIRect& bounds,
                       const Transform& localToDevice,
                       const Shape& shape) const override {
        // TODO: Have Shape provide a path-like iterator so we don't actually have to convert non
        // paths to SkPath just to iterate their pts/verbs
        SkPath path = shape.asPath();
        for (PathMiddleOutFanIter it(path); !it.done();) {
            for (auto [p0, p1, p2] : it.nextStack()) {
                // TODO: PathMiddleOutFanIter should use SkV2 instead of SkPoint?
                SkV2 p[3] = {{p0.fX, p0.fY}, {p1.fX, p1.fY}, {p2.fX, p2.fY}};
                SkV4 devPoints[3];
                localToDevice.mapPoints(p, devPoints, 3);

                // TODO: Instead do one appendVertices(maxTrianglsInFans*3) and then return vertices
                // at the end to avoid redundant bounds/offset checking in the DrawWriter.
                auto vw = writer->appendVertices(3);

                vw << devPoints[0].x << devPoints[0].y << devPoints[0].w  // p0
                   << devPoints[1].x << devPoints[1].y << devPoints[1].w  // p1
                   << devPoints[2].x << devPoints[2].y << devPoints[2].w; // p2
            }
        }
    }

    sk_sp<UniformData> writeUniforms(Layout layout,
                                     const SkIRect&,
                                     const Transform&,
                                     const Shape&) const override {
        // Control points are pre-transformed to device space on the CPU, so no uniforms needed.
        return nullptr;
    }
};

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
    // Because each fill type uses a different stencil settings, there is one Renderer per type.
    // However, at each stage (stencil vs. cover), there are only two RenderSteps to branch on.
    static const StencilFanRenderStep kWindingStencilFan{false};
    static const StencilFanRenderStep kEvenOddStencilFan{true};
    static const FillBoundsRenderStep kFill{false};
    static const FillBoundsRenderStep kInverseFill{true};

    // TODO: Uncomment and include the curve stenciling steps to draw curved paths
    static const Renderer kWindingRenderer{"stencil-and-fill[winding]",
                                           &kWindingStencilFan,
                                           /*&kWindingStencilCurves,*/
                                           &kFill};
    static const Renderer kInverseWindingRenderer{"stencil-and-fill[inverse-winding]",
                                                  &kWindingStencilFan,
                                                  /*&kWindingStencilCurves,*/
                                                  &kInverseFill};
    static const Renderer kEvenOddRenderer{"stencil-and-fill[evenodd]",
                                           &kEvenOddStencilFan,
                                           /*&kEvenOddStencilCurves,*/
                                           &kFill};
    static const Renderer kInverseEvenOddRenderer{"stencil-and-fill[inverse-evenodd]",
                                                  &kEvenOddStencilFan,
                                                  /*&kEvenOddStencilCurves,*/
                                                  &kInverseFill};

    switch(fillType) {
        case SkPathFillType::kWinding: return kWindingRenderer;
        case SkPathFillType::kEvenOdd: return kEvenOddRenderer;
        case SkPathFillType::kInverseWinding: return kInverseWindingRenderer;
        case SkPathFillType::kInverseEvenOdd: return kInverseEvenOddRenderer;
    }
    SkUNREACHABLE;
}

} // namespace skgpu
