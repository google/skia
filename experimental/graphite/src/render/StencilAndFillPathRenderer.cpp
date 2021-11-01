/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/Renderer.h"

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
class FillBoundsRenderStep : public RenderStep {
public:
    FillBoundsRenderStep() {}

    ~FillBoundsRenderStep() override {}

    const char* name()            const override { return "fill-bounds"; }
    // TODO: true when combined with a stencil step
    bool        requiresStencil() const override { return false; }
    bool        requiresMSAA()    const override { return false; }
    bool        performsShading() const override { return true;  }

    size_t requiredVertexSpace(const Shape&) const override {
        return 8 * sizeof(float);
    }

    size_t requiredIndexSpace(const Shape&) const override {
        return 0;
    }

    void writeVertices(VertexWriter vertexWriter,
                       IndexWriter indexWriter,
                       const Shape& shape) const override {
        vertexWriter.writeQuad(VertexWriter::TriStripFromRect(shape.bounds().asSkRect()));
    }


private:
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
