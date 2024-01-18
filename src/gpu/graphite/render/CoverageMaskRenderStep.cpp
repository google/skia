/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/graphite/render/CoverageMaskRenderStep.h"

#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/PathAtlas.h"
#include "src/gpu/graphite/geom/CoverageMaskShape.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"

namespace skgpu::graphite {

// The device origin is applied *before* the maskToDeviceRemainder matrix so that it can be
// combined with the mask atlas origin. This is necessary so that the mask bounds can be inset or
// outset for clamping w/o affecting the alignment of the mask sampling.
static skvx::float2 get_device_translation(const SkM44& localToDevice) {
    float m00 = localToDevice.rc(0,0), m01 = localToDevice.rc(0,1);
    float m10 = localToDevice.rc(1,0), m11 = localToDevice.rc(1,1);

    float det = m00*m11 - m01*m10;
    if (SkScalarNearlyZero(det)) {
        // We can't extract any pre-translation, since the upper 2x2 is not invertible. Return (0,0)
        // so that the maskToDeviceRemainder matrix remains the full transform.
        return {0.f, 0.f};
    }

    // Calculate inv([[m00,m01][m10,m11]])*[[m30][m31]] to get the pre-remainder device translation.
    float tx = localToDevice.rc(0,3), ty = localToDevice.rc(1,3);
    skvx::float4 invT = skvx::float4{m11, -m10, -m01, m00} * skvx::float4{tx,tx,ty,ty};
    return (invT.xy() + invT.zw()) / det;
}

CoverageMaskRenderStep::CoverageMaskRenderStep()
        : RenderStep("CoverageMaskRenderStep",
                     "",
                     // The mask will have AA outsets baked in, but the original bounds for clipping
                     // still require the outset for analytic coverage.
                     Flags::kPerformsShading | Flags::kHasTextures | Flags::kEmitsCoverage |
                     Flags::kOutsetBoundsForAA,
                     /*uniforms=*/{{"maskToDeviceRemainder", SkSLType::kFloat3x3}},
                     PrimitiveType::kTriangleStrip,
                     kDirectDepthGreaterPass,
                     /*vertexAttrs=*/{},
                     /*instanceAttrs=*/
                     // Draw bounds and mask bounds are in normalized relative to the mask texture,
                     // but 'drawBounds' is stored in float since the coords may map outside of
                     // [0,1] for inverse-filled masks. 'drawBounds' is relative to the logical mask
                     // entry's origin, while 'maskBoundsIn' is atlas-relative. Inverse fills swap
                     // the order in 'maskBoundsIn' to be RBLT.
                     {{"drawBounds", VertexAttribType::kFloat4 , SkSLType::kFloat4},  // ltrb
                      {"maskBoundsIn", VertexAttribType::kUShort4_norm, SkSLType::kFloat4},
                      // Remaining translation extracted from actual 'maskToDevice' transform.
                      {"deviceOrigin", VertexAttribType::kFloat2, SkSLType::kFloat2},
                      {"depth"     , VertexAttribType::kFloat, SkSLType::kFloat},
                      {"ssboIndices", VertexAttribType::kUShort2, SkSLType::kUShort2},
                      // deviceToLocal matrix for producing local coords for shader evaluation
                      {"mat0", VertexAttribType::kFloat3, SkSLType::kFloat3},
                      {"mat1", VertexAttribType::kFloat3, SkSLType::kFloat3},
                      {"mat2", VertexAttribType::kFloat3, SkSLType::kFloat3}},
                     /*varyings=*/
                     {// `maskBounds` are the atlas-relative, sorted bounds of the coverage mask.
                      // `textureCoords` are the atlas-relative UV coordinates of the draw, which
                      // can spill beyond `maskBounds` for inverse fills.
                      // TODO: maskBounds is constant for all fragments for a given instance,
                      // could we store them in the draw's SSBO?
                      {"maskBounds"   , SkSLType::kFloat4},
                      {"textureCoords", SkSLType::kFloat2},
                      // 'invert' is set to 0 use unmodified coverage, and set to 1 for "1-c".
                      {"invert", SkSLType::kHalf}}) {}

std::string CoverageMaskRenderStep::vertexSkSL() const {
    // Returns the body of a vertex function, which must define a float4 devPosition variable and
    // must write to an already-defined float2 stepLocalCoords variable.
    return "float4 devPosition = coverage_mask_vertex_fn("
                    "float2(sk_VertexID >> 1, sk_VertexID & 1), "
                    "maskToDeviceRemainder, drawBounds, maskBoundsIn, deviceOrigin, "
                    "depth, float3x3(mat0, mat1, mat2), "
                    "maskBounds, textureCoords, invert, stepLocalCoords);\n";
}

std::string CoverageMaskRenderStep::texturesAndSamplersSkSL(
        const ResourceBindingRequirements& bindingReqs, int* nextBindingIndex) const {
    return EmitSamplerLayout(bindingReqs, nextBindingIndex) + " sampler2D pathAtlas;";
}

const char* CoverageMaskRenderStep::fragmentCoverageSkSL() const {
    return R"(
        half c = sample(pathAtlas, clamp(textureCoords, maskBounds.LT, maskBounds.RB)).r;
        outputCoverage = half4(mix(c, 1 - c, invert));
    )";
}

void CoverageMaskRenderStep::writeVertices(DrawWriter* dw,
                                           const DrawParams& params,
                                           skvx::ushort2 ssboIndices) const {
    const CoverageMaskShape& coverageMask = params.geometry().coverageMaskShape();
    const TextureProxy* proxy = coverageMask.textureProxy();
    SkASSERT(proxy);

    // A quad is a 4-vertex instance. The coordinates are derived from the vertex IDs.
    DrawWriter::Instances instances(*dw, {}, {}, 4);

    // The device origin is the  translation extracted from the mask-to-device matrix so
    // that the remaining matrix uniform has less variance between draws.
    const auto& maskToDevice = params.transform().matrix();
    skvx::float2 deviceOrigin = get_device_translation(maskToDevice);

    // Relative to mask space (device origin and mask-to-device remainder must be applied in shader)
    skvx::float4 maskBounds = coverageMask.bounds().ltrb();
    skvx::float4 drawBounds;

    if (coverageMask.inverted()) {
        // Only mask filters trigger complex transforms, and they are never inverse filled. Since
        // we know this is an inverted mask, then we can exactly map the draw's clip bounds to mask
        // space so that the clip is still fully covered without branching in the vertex shader.
        SkASSERT(maskToDevice == SkM44::Translate(deviceOrigin.x(), deviceOrigin.y()));
        drawBounds = params.clip().drawBounds().makeOffset(-deviceOrigin).ltrb();

        // If the mask is fully clipped out, then the shape's mask info should be (0,0,0,0).
        // If it's not fully clipped out, then the mask info should be non-empty.
        SkASSERT(!params.clip().transformedShapeBounds().isEmptyNegativeOrNaN() ^
                 all(maskBounds == 0.f));

        if (params.clip().transformedShapeBounds().isEmptyNegativeOrNaN()) {
            // The inversion check is strict inequality, so (0,0,0,0) would not be detected. Adjust
            // to (0,0,1/2,1/2) to restrict sampling to the top-left quarter of the top-left pixel,
            // which should have a value of 0 regardless of filtering mode.
            maskBounds = skvx::float4{0.f, 0.f, 0.5f, 0.5f};
        } else {
            // Add 1/2px outset to the mask bounds so that clamped coordinates sample the texel
            // center of the padding around the atlas entry.
            maskBounds += skvx::float4{-0.5f, -0.5f, 0.5f, 0.5f};
        }

        // and store RBLT so that the 'maskBoundsIn' attribute has xy > zw to detect inverse fill.
        maskBounds = skvx::shuffle<2,3,0,1>(maskBounds);
    } else {
        // If we aren't inverted, then the originally assigned values don't need to be adjusted, but
        // also ensure the mask isn't empty (otherwise the draw should have been skipped earlier).
        SkASSERT(!coverageMask.bounds().isEmptyNegativeOrNaN());
        SkASSERT(all(maskBounds.xy() < maskBounds.zw()));

        // Since the mask bounds and draw bounds are 1-to-1 with each other, the clamping of texture
        // coords is mostly a formality. We inset the mask bounds by 1/2px so that we clamp to the
        // texel center of the outer row/column of the mask. This should be a no-op for nearest
        // sampling but prevents any linear sampling from incorporating adjacent data; for atlases
        // this would just be 0 but for non-atlas coverage masks that might not have padding this
        // avoids filtering unknown values in an approx-fit texture.
        drawBounds = maskBounds;
        maskBounds -= skvx::float4{-0.5f, -0.5f, 0.5f, 0.5f};
    }

    // Move 'drawBounds' and 'maskBounds' into the atlas coordinate space, then adjust the
    // device translation to undo the atlas origin automatically in the vertex shader.
    skvx::float2 textureOrigin = skvx::cast<float>(coverageMask.textureOrigin());
    maskBounds += textureOrigin.xyxy();
    drawBounds += textureOrigin.xyxy();
    deviceOrigin -= textureOrigin;

    // Normalize drawBounds and maskBounds after possibly correcting drawBounds for inverse fills.
    // The maskToDevice matrix uniform will handle de-normalizing drawBounds for vertex positions.
    auto atlasSizeInv = skvx::float2{1.f / proxy->dimensions().width(),
                                     1.f / proxy->dimensions().height()};
    drawBounds *= atlasSizeInv.xyxy();
    maskBounds *= atlasSizeInv.xyxy();
    deviceOrigin *= atlasSizeInv;

    // Since the mask bounds define normalized texels of the texture, we can encode them as
    // ushort_norm without losing precision to save space.
    SkASSERT(all((maskBounds >= 0.f) & (maskBounds <= 1.f)));
    maskBounds = 65535.f * maskBounds + 0.5f;

    const SkM44& m = coverageMask.deviceToLocal();
    instances.append(1) << drawBounds << skvx::cast<uint16_t>(maskBounds) << deviceOrigin
                        << params.order().depthAsFloat() << ssboIndices
                        << m.rc(0,0) << m.rc(1,0) << m.rc(3,0)   // mat0
                        << m.rc(0,1) << m.rc(1,1) << m.rc(3,1)   // mat1
                        << m.rc(0,3) << m.rc(1,3) << m.rc(3,3);  // mat2
}

void CoverageMaskRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                                      PipelineDataGatherer* gatherer) const {
    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, this->uniforms());)

    const CoverageMaskShape& coverageMask = params.geometry().coverageMaskShape();
    const TextureProxy* proxy = coverageMask.textureProxy();
    SkASSERT(proxy);

    // Most coverage masks are aligned with the device pixels, so the params' transform is an
    // integer translation matrix. This translation is extracted as an instance attribute so that
    // the remaining transform has a much lower frequency of changing (only complex-transformed
    // mask filters).
    skvx::float2 deviceOrigin = get_device_translation(params.transform().matrix());
    SkMatrix maskToDevice = params.transform().matrix().asM33();
    maskToDevice.preTranslate(-deviceOrigin.x(), -deviceOrigin.y());

    // The mask coordinates in the vertex shader will be normalized, so scale by the proxy size
    // to get back to Skia's texel-based coords.
    maskToDevice.preScale(proxy->dimensions().width(), proxy->dimensions().height());

    // Write uniforms:
    gatherer->write(maskToDevice);

    // Write textures and samplers:
    const bool pixelAligned =
            params.transform().type() <= Transform::Type::kSimpleRectStaysRect &&
            params.transform().maxScaleFactor() == 1.f &&
            all(deviceOrigin == floor(deviceOrigin + SK_ScalarNearlyZero));
    constexpr SkTileMode kTileModes[2] = {SkTileMode::kClamp, SkTileMode::kClamp};
    gatherer->add(pixelAligned ? SkFilterMode::kNearest : SkFilterMode::kLinear,
                  kTileModes, sk_ref_sp(proxy));
}

}  // namespace skgpu::graphite
