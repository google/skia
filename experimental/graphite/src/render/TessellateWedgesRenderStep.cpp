/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/render/TessellateWedgesRenderStep.h"

#include "experimental/graphite/src/DrawWriter.h"
#include "experimental/graphite/src/geom/Shape.h"
#include "experimental/graphite/src/geom/Transform_graphite.h"
#include "experimental/graphite/src/render/StencilAndCoverDSS.h"

#include "src/gpu/tessellate/AffineMatrix.h"
#include "src/gpu/tessellate/MidpointContourParser.h"
#include "src/gpu/tessellate/PatchWriter.h"
#include "src/gpu/tessellate/PathTessellator.h"
#include "src/gpu/tessellate/PathWedgeTessellator.h"

namespace skgpu {

namespace {

// TODO: This is very similar to DrawWriterAllocator in TessellateCurveRenderStep except that the
// index count is calculated differently. These will be merged once skbug.com/13056 is resolved.
struct DrawWriterAllocator {
    DrawWriterAllocator(size_t stride, // required for PatchAllocator
                        DrawWriter& writer,
                        BindBufferInfo fixedVertexBuffer,
                        BindBufferInfo fixedIndexBuffer,
                        unsigned int reserveCount)
            : fInstances(writer, fixedVertexBuffer, fixedIndexBuffer) {
        SkASSERT(writer.instanceStride() == stride);
        // TODO: Is it worth re-reserving large chunks after this preallocation is used up? Or will
        // appending 1 at a time be fine since it's coming from a large vertex buffer alloc anyway?
        fInstances.reserve(reserveCount);
    }

    VertexWriter append() {
        // TODO (skbug.com/13056): Actually compute optimal minimum required index count based on
        // PatchWriter's tracked segment count^4.
        // Wedges use one extra triangle to connect to the fan point compared to the curve version.
        static constexpr unsigned int kMaxIndexCount =
                3 * (1 + PathTessellator::NumCurveTrianglesAtResolveLevel(
                            PathTessellator::kMaxFixedResolveLevel));
        return fInstances.append(kMaxIndexCount, 1);
    }

    DrawWriter::DynamicInstances fInstances;
};

// Only kFanPoint, no stroke params, since this is for filled wedges.
// No explicit curve type, since we assume infinity is supported on GPUs using graphite
// No color or wide color attribs, since it might always be part of the PaintParams
// or we'll add a color-only fast path to RenderStep later.
static constexpr PatchAttribs kAttribs = PatchAttribs::kFanPoint;
using Writer = PatchWriter<DrawWriterAllocator,
                           Required<PatchAttribs::kFanPoint>>;

size_t fixed_vertex_buffer_size() {
    return PathWedgeTessellator::FixedVertexBufferSize(PathTessellator::kMaxFixedResolveLevel);
}
size_t fixed_index_buffer_size() {
    return PathWedgeTessellator::FixedIndexBufferSize(PathTessellator::kMaxFixedResolveLevel);
}

}  // namespace

TessellateWedgesRenderStep::TessellateWedgesRenderStep(bool evenOdd)
        : RenderStep(Flags::kRequiresMSAA,
                     /*uniforms=*/{},
                     PrimitiveType::kTriangles,
                     evenOdd ? kEvenOddStencilPass : kWindingStencilPass,
                     /*vertexAttrs=*/  {{"resolveLevel_and_idx",
                                         VertexAttribType::kFloat2, SkSLType::kFloat2}},
                     /*instanceAttrs=*/{{"p01", VertexAttribType::kFloat4, SkSLType::kFloat4},
                                        {"p23", VertexAttribType::kFloat4, SkSLType::kFloat4},
                                        {"fanPointAttrib", VertexAttribType::kFloat2,
                                                           SkSLType::kFloat2}}) {
    SkASSERT(this->instanceStride() == PatchStride(kAttribs));
}

TessellateWedgesRenderStep::~TessellateWedgesRenderStep() {}

const char* TessellateWedgesRenderStep::vertexSkSL() const {
    // TODO: Share SkSL with GrPathTessellationShader_MiddleOut
    // TODO: This SkSL depends on wangs_formula::as_sksl(), which is currently manually added in
    // MtlGraphicsPipeline but could be handled nicer.
    return R"(
        float resolveLevel = resolveLevel_and_idx.x;
        float idxInResolveLevel = resolveLevel_and_idx.y;
        float2 localcoord;

        // A negative resolve level means this is the fan point.
        // TODO: This "if" handling a negative resolve level and reading 'fanPointAttrib' is the
        // only difference between this SkSL and TessellateCurvesRenderStep's SkSL.
        if (resolveLevel < 0) {
            localcoord = fanPointAttrib;
        } else if (isinf(p23.z)) {
            // This patch is an exact triangle.
            localcoord = (resolveLevel != 0)      ? p01.zw
                       : (idxInResolveLevel != 0) ? p23.xy
                                                  : p01.xy;
        } else {
            float2 p0=p01.xy, p1=p01.zw, p2=p23.xy, p3=p23.zw;
            float w = -1;  // w < 0 tells us to treat the instance as an integral cubic.
            float maxResolveLevel;
            if (isinf(p23.w)) {
                // Conics are 3 points, with the weight in p3.
                w = p3.x;
                maxResolveLevel = wangs_formula_conic_log2(4, p0, p1, p2, w);
                p1 *= w;  // Unproject p1.
                p3 = p2;  // Duplicate the endpoint for shared code that also runs on cubics.
            } else {
                // The patch is an integral cubic.
                maxResolveLevel = wangs_formula_cubic_log2(4, p0, p1, p2, p3,
                                                           float2x2(1.0, 0.0, 0.0, 1.0));
            }
            if (resolveLevel > maxResolveLevel) {
                // This vertex is at a higher resolve level than we need. Demote to a lower
                // resolveLevel, which will produce a degenerate triangle.
                idxInResolveLevel = floor(ldexp(idxInResolveLevel,
                                                int(maxResolveLevel - resolveLevel)));
                resolveLevel = maxResolveLevel;
            }
            // Promote our location to a discrete position in the maximum fixed resolve level.
            // This is extra paranoia to ensure we get the exact same fp32 coordinates for
            // colocated points from different resolve levels (e.g., the vertices T=3/4 and
            // T=6/8 should be exactly colocated).
            float fixedVertexID = floor(.5 + ldexp(idxInResolveLevel, int(5 - resolveLevel)));
            if (0 < fixedVertexID && fixedVertexID < 32) {
                float T = fixedVertexID * (1 / 32.0);

                // Evaluate at T. Use De Casteljau's for its accuracy and stability.
                float2 ab = mix(p0, p1, T);
                float2 bc = mix(p1, p2, T);
                float2 cd = mix(p2, p3, T);
                float2 abc = mix(ab, bc, T);
                float2 bcd = mix(bc, cd, T);
                float2 abcd = mix(abc, bcd, T);

                // Evaluate the conic weight at T.
                float u = mix(1.0, w, T);
                float v = w + 1 - u;  // == mix(w, 1, T)
                float uv = mix(u, v, T);

                localcoord = (w < 0) ? /*cubic*/ abcd : /*conic*/ abc/uv;
            } else {
                localcoord = (fixedVertexID == 0) ? p0.xy : p3.xy;
            }
        }
        float4 devPosition = float4(localcoord.xy, 0.0, 1.0);)";
}

void TessellateWedgesRenderStep::writeVertices(DrawWriter* dw,
                                               const SkIRect& bounds,
                                               const Transform& localToDevice,
                                               const Shape& shape) const {
    // TODO: Caps check
    static constexpr int kMaxTessellationSegments = 1 << PathTessellator::kMaxFixedResolveLevel;
    SkPath path = shape.asPath(); // TODO: Iterate the Shape directly

    BindBufferInfo fixedVertexBuffer = dw->bufferManager()->getStaticBuffer(
            BufferType::kVertex,
            PathWedgeTessellator::WriteFixedVertexBuffer,
            fixed_vertex_buffer_size);
    BindBufferInfo fixedIndexBuffer = dw->bufferManager()->getStaticBuffer(
            BufferType::kIndex,
            PathWedgeTessellator::WriteFixedIndexBuffer,
            fixed_index_buffer_size);

    int patchReserveCount = PathWedgeTessellator::PatchPreallocCount(path.countVerbs());
    Writer writer{kAttribs, kMaxTessellationSegments,
                  *dw, fixedVertexBuffer, fixedIndexBuffer, patchReserveCount};

    // TODO: Is it better to pre-transform on the CPU and only have a matrix uniform to compute
    // local coords, or is it better to always transform on the GPU (less CPU usage, more
    // uniform data to upload, dependent on push constants or storage buffers for good batching)

    // Currently no additional transform is applied by the GPU.
    wangs_formula::VectorXform shaderXform(SkMatrix::I());
    // TODO: This doesn't handle perspective yet, and ideally wouldn't go through SkMatrix.
    // It may not be relevant, though, if transforms are applied on the GPU and we only need to
    // determine an approximate 2x2 for 'shaderXform' and Wang's formula evaluation.
    AffineMatrix m(localToDevice.matrix().asM33());

    // TODO: Essentially the same as PathWedgeTessellator::write_patches but with a different
    // PatchWriter template.
    // For wedges, we iterate over each contour explicitly, using a fan point position that is in
    // the midpoint of the current contour.
    MidpointContourParser parser{path};
    while (parser.parseNextContour()) {
        writer.updateFanPointAttrib(m.mapPoint(parser.currentMidpoint()));
        float2 lastPoint = {0, 0};
        float2 startPoint = {0, 0};
        for (auto [verb, pts, w] : parser.currentContour()) {
            switch (verb) {
                case SkPathVerb::kMove: {
                    startPoint = lastPoint = m.map1Point(pts);
                    break;
                }

                case SkPathVerb::kLine: {
                    // Unlike curve tessellation, wedges have to handle lines as part of the patch,
                    // effectively forming a single triangle with the fan point.
                    auto [p0, p1] = m.map2Points(pts);
                    writer.writeLine(p0, p1);
                    lastPoint = p1;
                    break;
                }

                case SkPathVerb::kQuad: {
                    auto [p0, p1] = m.map2Points(pts);
                    auto p2 = m.map1Point(pts+2);

                    writer.writeQuadratic(p0, p1, p2, shaderXform);
                    lastPoint = p2;
                    break;
                }

                case SkPathVerb::kConic: {
                    auto [p0, p1] = m.map2Points(pts);
                    auto p2 = m.map1Point(pts+2);

                    writer.writeConic(p0, p1, p2, *w, shaderXform);
                    lastPoint = p2;
                    break;
                }

                case SkPathVerb::kCubic: {
                    auto [p0, p1] = m.map2Points(pts);
                    auto [p2, p3] = m.map2Points(pts+2);

                    writer.writeCubic(p0, p1, p2, p3, shaderXform);
                    lastPoint = p3;
                    break;
                }

                default: break;
            }
        }

        // Explicitly close the contour with another line segment, which also differs from curve
        // tessellation since that approach's triangle step automatically closes the contour.
        if (any(lastPoint != startPoint)) {
            writer.writeLine(lastPoint, startPoint);
        }
    }
}

sk_sp<SkUniformData> TessellateWedgesRenderStep::writeUniforms(Layout,
                                                              const SkIRect&,
                                                              const Transform&,
                                                              const Shape&) const {
    // Control points are pre-transformed to device space on the CPU, so no uniforms needed.
    return nullptr;
}

}  // namespace skgpu
