/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/TessellateCurvesRenderStep.h"

#include "include/core/SkPath.h"
#include "include/core/SkPathTypes.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkSpan_impl.h"
#include "src/base/SkEnumBitMask.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/graphite/Attribute.h"
#include "src/gpu/graphite/BufferManager.h"
#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/geom/Geometry.h"
#include "src/gpu/graphite/geom/Shape.h"
#include "src/gpu/graphite/geom/Transform.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"
#include "src/gpu/graphite/render/DynamicInstancesPatchAllocator.h"
#include "src/gpu/tessellate/FixedCountBufferUtils.h"
#include "src/gpu/tessellate/PatchWriter.h"
#include "src/gpu/tessellate/Tessellation.h"
#include "src/gpu/tessellate/WangsFormula.h"
#include "src/sksl/SkSLString.h"

#include <cstddef>
#include <utility>

namespace skgpu::graphite {

namespace {

using namespace skgpu::tess;

// No fan point or stroke params, since this is for filled curves (not strokes or wedges)
// No explicit curve type on platforms that support infinity.
// No color or wide color attribs, since it might always be part of the PaintParams
// or we'll add a color-only fast path to RenderStep later.
static constexpr PatchAttribs kAttribs = PatchAttribs::kPaintDepth |
                                         PatchAttribs::kSsboIndex;
static constexpr PatchAttribs kAttribsWithCurveType = kAttribs | PatchAttribs::kExplicitCurveType;
using Writer = PatchWriter<DynamicInstancesPatchAllocator<FixedCountCurves>,
                           Required<PatchAttribs::kPaintDepth>,
                           Required<PatchAttribs::kSsboIndex>,
                           Optional<PatchAttribs::kExplicitCurveType>,
                           AddTrianglesWhenChopping,
                           DiscardFlatCurves>;

// The order of the attribute declarations must match the order used by
// PatchWriter::emitPatchAttribs, i.e.:
//     join << fanPoint << stroke << color << depth << curveType << ssboIndices
static constexpr Attribute kBaseAttributes[] = {
        {"p01", VertexAttribType::kFloat4, SkSLType::kFloat4},
        {"p23", VertexAttribType::kFloat4, SkSLType::kFloat4},
        {"depth", VertexAttribType::kFloat, SkSLType::kFloat},
        {"ssboIndices", VertexAttribType::kUInt2, SkSLType::kUInt2}};

static constexpr Attribute kAttributesWithCurveType[] = {
        {"p01", VertexAttribType::kFloat4, SkSLType::kFloat4},
        {"p23", VertexAttribType::kFloat4, SkSLType::kFloat4},
        {"depth", VertexAttribType::kFloat, SkSLType::kFloat},
        {"curveType", VertexAttribType::kFloat, SkSLType::kFloat},
        {"ssboIndices", VertexAttribType::kUInt2, SkSLType::kUInt2}};

static constexpr SkSpan<const Attribute> kAttributes[2] = {kAttributesWithCurveType,
                                                           kBaseAttributes};

}  // namespace

TessellateCurvesRenderStep::TessellateCurvesRenderStep(bool evenOdd,
                                                       bool infinitySupport,
                                                       StaticBufferManager* bufferManager)
        : RenderStep(evenOdd ? RenderStepID::kTessellateCurves_EvenOdd
                             : RenderStepID::kTessellateCurves_Winding,
                     Flags::kRequiresMSAA |
                     Flags::kAppendDynamicInstances |
                     Flags::kIgnoreInverseFill,
                     /*uniforms=*/{{"localToDevice", SkSLType::kFloat4x4}},
                     PrimitiveType::kTriangles,
                     evenOdd ? kEvenOddStencilPass : kWindingStencilPass,
                     /*staticAttrs=*/{{"resolveLevel_and_idx",
                                       VertexAttribType::kFloat2, SkSLType::kFloat2}},
                     /*appendAttrs=*/kAttributes[infinitySupport])
        , fInfinitySupport(infinitySupport) {
    SkASSERT(this->appendDataStride() ==
             PatchStride(infinitySupport ? kAttribs : kAttribsWithCurveType));

    // Initialize the static buffers we'll use when recording draw calls.
    // NOTE: Each instance of this RenderStep gets its own copy of the data. If this ends up causing
    // problems, we can modify StaticBufferManager to de-duplicate requests.
    auto vertexData = bufferManager->getVertexWriter(FixedCountCurves::VertexBufferVertexCount(),
                                                     FixedCountCurves::VertexBufferStride(),
                                                     &fVertexBuffer);
    if (vertexData) {
        FixedCountCurves::WriteVertexBuffer(std::move(vertexData),
                                            FixedCountCurves::VertexBufferSize());
    } // otherwise static buffer creation failed, so do nothing; Context initialization will fail.

    const size_t indexSize = FixedCountCurves::IndexBufferSize();
    auto indexData = bufferManager->getIndexWriter(indexSize, &fIndexBuffer);
    if (indexData) {
        FixedCountCurves::WriteIndexBuffer(std::move(indexData), indexSize);
    } // ""
}

TessellateCurvesRenderStep::~TessellateCurvesRenderStep() {}

std::string TessellateCurvesRenderStep::vertexSkSL() const {
    return SkSL::String::printf(
            // TODO: Approximate perspective scaling to match how PatchWriter is configured (or
            // provide explicit tessellation level in instance data instead of replicating
            // work).
            "float2x2 vectorXform = float2x2(localToDevice[0].xy, localToDevice[1].xy);\n"
            "float2 localCoord = tessellate_filled_curve("
                    "vectorXform, resolveLevel_and_idx.x, resolveLevel_and_idx.y, p01, p23, %s);\n"
            "float4 devPosition = localToDevice * float4(localCoord, 0.0, 1.0);\n"
            "devPosition.z = depth;\n"
            "stepLocalCoords = localCoord;\n",
            fInfinitySupport ? "curve_type_using_inf_support(p23)" : "curveType");
}

void TessellateCurvesRenderStep::writeVertices(DrawWriter* dw,
                                               const DrawParams& params,
                                               skvx::uint2 ssboIndices) const {
    SkPath path = params.geometry().shape().asPath(); // TODO: Iterate the Shape directly

    int patchReserveCount = FixedCountCurves::PreallocCount(path.countVerbs());
    Writer writer{fInfinitySupport ? kAttribs : kAttribsWithCurveType,
                  *dw,
                  fVertexBuffer,
                  fIndexBuffer,
                  patchReserveCount};
    writer.updatePaintDepthAttrib(params.order().depthAsFloat());
    writer.updateSsboIndexAttrib(ssboIndices);

    // The vector xform approximates how the control points are transformed by the shader to
    // more accurately compute how many *parametric* segments are needed.
    // TODO: This doesn't account for perspective division yet, which will require updating the
    // approximate transform based on each verb's control points' bounding box.
    SkASSERT(params.transform().type() < Transform::Type::kPerspective);
    writer.setShaderTransform(wangs_formula::VectorXform{params.transform().matrix()},
                              params.transform().maxScaleFactor());

    // TODO: For filled curves, the path verb loop is simple enough that it's not too big a deal
    // to copy the logic from PathCurveTessellator::write_patches. It may be required if we end
    // up switching to a shape iterator in graphite vs. a path iterator in ganesh, or if
    // graphite does not control point transformation on the CPU. On the  other hand, if we
    // provide a templated WritePatches function, the iterator could also be a template arg in
    // addition to PatchWriter's traits. Whatever pattern we choose will be based more on what's
    // best for the wedge and stroke case, which have more complex loops.
    for (auto [verb, pts, w] : SkPathPriv::Iterate(path)) {
        switch (verb) {
            case SkPathVerb::kQuad:  writer.writeQuadratic(pts); break;
            case SkPathVerb::kConic: writer.writeConic(pts, *w); break;
            case SkPathVerb::kCubic: writer.writeCubic(pts);     break;
            default:                                             break;
        }
    }
}

void TessellateCurvesRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                                          PipelineDataGatherer* gatherer) const {
    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, this->uniforms());)

    gatherer->write(params.transform().matrix());
}

}  // namespace skgpu::graphite
