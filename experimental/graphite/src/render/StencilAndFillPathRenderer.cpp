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
#include "src/gpu/BufferWriter.h"

namespace skgpu {

namespace {

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
    FillBoundsRenderStep()
            : RenderStep(Flags::kPerformsShading,
                         /*uniforms=*/{},
                         PrimitiveType::kTriangleStrip,
                         /*vertexAttrs=*/{{"position", VertexAttribType::kFloat2, SLType::kFloat2}},
                         /*instanceAttrs=*/{}) {}

    ~FillBoundsRenderStep() override {}

    const char* name() const override { return "fill-bounds"; }

    const char* vertexMSL() const override {
        // TODO: apply transform matrix from uniform data
        // TODO: RenderSteps should not worry about RTAdjust, but currently the mtl pipeline does
        // account for it, so this geometry won't be in the right coordinate system yet.
        return "out.position.xy = vtx.position;\n"
               "out.position.zw = float2(0.0, 1.0);\n";
    }

    void writeVertices(DrawWriter* writer, const Shape& shape) const override {
        // TODO: Need to account for the transform eventually, but that requires more plumbing
        writer->appendVertices(4)
               .writeQuad(VertexWriter::TriStripFromRect(shape.bounds().asSkRect()));
        // TODO: triangle strip/fan is actually tricky to merge correctly, since we want strips
        // for everything that was appended by the RenderStep, but no connection across RenderSteps,
        // so either we need a way to end it here, or switch to instance rendering w/o instance
        // data so that vertices are still clustered appripriately. But that would require updating
        // the DrawWriter to support appending both vertex and instance data simultaneously, which
        // would need to return 2 vertex writers?
    }

    sk_sp<UniformData> writeUniforms(Layout layout, const Shape&) const override {
        // TODO: Return the uniform data that is needed for this draw, but since there are no
        // declared uniforms right now, just return nullptr. Eventually, the uniforms should include
        // the draw's transform (at least until we use storage buffers).
        return nullptr;
    }
};

} // anonymous namespace

const Renderer& Renderer::StencilAndFillPath() {
    // TODO: Uncomment and include in kRenderer to draw flattened paths instead of bboxes
    // static const StencilFanRenderStep kStencilFan;
    // TODO: Uncomment and include in kRenderer to draw curved paths
    // static const StencilCurvesRenderStep kStencilCurves;
    // TODO: This could move into a header and be reused across renderers
    static const FillBoundsRenderStep kCover;
    static const Renderer kRenderer("stencil-and-fill",
                                    /*&kStencilFan,*/ /*&kStencilCurves,*/ &kCover);

    return kRenderer;
}

} // namespace skgpu
