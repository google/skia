/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCPathProcessor.h"

#include "GrGpuCommandBuffer.h"
#include "GrOnFlushResourceProvider.h"
#include "GrTexture.h"
#include "ccpr/GrCCPerFlushResources.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLProgramBuilder.h"
#include "glsl/GrGLSLVarying.h"

// Paths are drawn as octagons. Each point on the octagon is the intersection of two lines: one edge
// from the path's bounding box and one edge from its 45-degree bounding box. The below inputs
// define a vertex by the two edges that need to be intersected. Normals point out of the octagon,
// and the bounding boxes are sent in as instance attribs.
static constexpr float kOctoEdgeNorms[8 * 4] = {
    // bbox   // bbox45
    -1, 0,    -1,+1,
    -1, 0,    -1,-1,
     0,-1,    -1,-1,
     0,-1,    +1,-1,
    +1, 0,    +1,-1,
    +1, 0,    +1,+1,
     0,+1,    +1,+1,
     0,+1,    -1,+1,
};

GR_DECLARE_STATIC_UNIQUE_KEY(gVertexBufferKey);

sk_sp<const GrGpuBuffer> GrCCPathProcessor::FindVertexBuffer(GrOnFlushResourceProvider* onFlushRP) {
    GR_DEFINE_STATIC_UNIQUE_KEY(gVertexBufferKey);
    return onFlushRP->findOrMakeStaticBuffer(GrGpuBufferType::kVertex, sizeof(kOctoEdgeNorms),
                                             kOctoEdgeNorms, gVertexBufferKey);
}

static constexpr uint16_t kRestartStrip = 0xffff;

static constexpr uint16_t kOctoIndicesAsStrips[] = {
    1, 0, 2, 4, 3, kRestartStrip, // First half.
    5, 4, 6, 0, 7 // Second half.
};

static constexpr uint16_t kOctoIndicesAsTris[] = {
    // First half.
    1, 0, 2,
    0, 4, 2,
    2, 4, 3,

    // Second half.
    5, 4, 6,
    4, 0, 6,
    6, 0, 7,
};

GR_DECLARE_STATIC_UNIQUE_KEY(gIndexBufferKey);

constexpr GrPrimitiveProcessor::Attribute GrCCPathProcessor::kInstanceAttribs[];
constexpr GrPrimitiveProcessor::Attribute GrCCPathProcessor::kEdgeNormsAttrib;

sk_sp<const GrGpuBuffer> GrCCPathProcessor::FindIndexBuffer(GrOnFlushResourceProvider* onFlushRP) {
    GR_DEFINE_STATIC_UNIQUE_KEY(gIndexBufferKey);
    if (onFlushRP->caps()->usePrimitiveRestart()) {
        return onFlushRP->findOrMakeStaticBuffer(GrGpuBufferType::kIndex,
                                                 sizeof(kOctoIndicesAsStrips), kOctoIndicesAsStrips,
                                                 gIndexBufferKey);
    } else {
        return onFlushRP->findOrMakeStaticBuffer(GrGpuBufferType::kIndex,
                                                 sizeof(kOctoIndicesAsTris), kOctoIndicesAsTris,
                                                 gIndexBufferKey);
    }
}

GrCCPathProcessor::GrCCPathProcessor(const GrTextureProxy* atlas,
                                     const SkMatrix& viewMatrixIfUsingLocalCoords)
        : INHERITED(kGrCCPathProcessor_ClassID)
        , fAtlasAccess(atlas->textureType(), atlas->config(), GrSamplerState::Filter::kNearest,
                       GrSamplerState::WrapMode::kClamp)
        , fAtlasSize(atlas->isize())
        , fAtlasOrigin(atlas->origin()) {
    // TODO: Can we just assert that atlas has GrCCAtlas::kTextureOrigin and remove fAtlasOrigin?
    this->setInstanceAttributes(kInstanceAttribs, kNumInstanceAttribs);
    SkASSERT(this->instanceStride() == sizeof(Instance));

    this->setVertexAttributes(&kEdgeNormsAttrib, 1);
    this->setTextureSamplerCnt(1);

    if (!viewMatrixIfUsingLocalCoords.invert(&fLocalMatrix)) {
        fLocalMatrix.setIdentity();
    }
}

class GLSLPathProcessor : public GrGLSLGeometryProcessor {
public:
    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override;

private:
    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& primProc,
                 FPCoordTransformIter&& transformIter) override {
        const GrCCPathProcessor& proc = primProc.cast<GrCCPathProcessor>();
        pdman.set2f(fAtlasAdjustUniform, 1.0f / proc.atlasSize().fWidth,
                    1.0f / proc.atlasSize().fHeight);
        this->setTransformDataHelper(proc.localMatrix(), pdman, &transformIter);
    }

    GrGLSLUniformHandler::UniformHandle fAtlasAdjustUniform;

    typedef GrGLSLGeometryProcessor INHERITED;
};

GrGLSLPrimitiveProcessor* GrCCPathProcessor::createGLSLInstance(const GrShaderCaps&) const {
    return new GLSLPathProcessor();
}

void GrCCPathProcessor::drawPaths(GrOpFlushState* flushState, const GrPipeline& pipeline,
                                  const GrPipeline::FixedDynamicState* fixedDynamicState,
                                  const GrCCPerFlushResources& resources, int baseInstance,
                                  int endInstance, const SkRect& bounds) const {
    const GrCaps& caps = flushState->caps();
    GrPrimitiveType primitiveType = caps.usePrimitiveRestart()
                                            ? GrPrimitiveType::kTriangleStrip
                                            : GrPrimitiveType::kTriangles;
    int numIndicesPerInstance = caps.usePrimitiveRestart()
                                        ? SK_ARRAY_COUNT(kOctoIndicesAsStrips)
                                        : SK_ARRAY_COUNT(kOctoIndicesAsTris);
    GrMesh mesh(primitiveType);
    auto enablePrimitiveRestart = GrPrimitiveRestart(flushState->caps().usePrimitiveRestart());

    mesh.setIndexedInstanced(resources.refIndexBuffer(), numIndicesPerInstance,
                             resources.refInstanceBuffer(), endInstance - baseInstance,
                             baseInstance, enablePrimitiveRestart);
    mesh.setVertexData(resources.refVertexBuffer());

    flushState->rtCommandBuffer()->draw(*this, pipeline, fixedDynamicState, nullptr, &mesh, 1,
                                        bounds);
}

void GLSLPathProcessor::onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) {
    using InstanceAttribs = GrCCPathProcessor::InstanceAttribs;
    using Interpolation = GrGLSLVaryingHandler::Interpolation;

    const GrCCPathProcessor& proc = args.fGP.cast<GrCCPathProcessor>();
    GrGLSLUniformHandler* uniHandler = args.fUniformHandler;
    GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;

    const char* atlasAdjust;
    fAtlasAdjustUniform = uniHandler->addUniform(
            kVertex_GrShaderFlag,
            kFloat2_GrSLType, "atlas_adjust", &atlasAdjust);

    varyingHandler->emitAttributes(proc);

    GrGLSLVarying texcoord(kFloat3_GrSLType);
    GrGLSLVarying color(kHalf4_GrSLType);
    varyingHandler->addVarying("texcoord", &texcoord);
    varyingHandler->addPassThroughAttribute(proc.getInstanceAttrib(InstanceAttribs::kColor),
                                            args.fOutputColor, Interpolation::kCanBeFlat);

    // The vertex shader bloats and intersects the devBounds and devBounds45 rectangles, in order to
    // find an octagon that circumscribes the (bloated) path.
    GrGLSLVertexBuilder* v = args.fVertBuilder;

    // Each vertex is the intersection of one edge from devBounds and one from devBounds45.
    // 'N' holds the normals to these edges as column vectors.
    //
    // NOTE: "float2x2(float4)" is valid and equivalent to "float2x2(float4.xy, float4.zw)",
    // however Intel compilers crash when we use the former syntax in this shader.
    v->codeAppendf("float2x2 N = float2x2(%s.xy, %s.zw);", proc.getEdgeNormsAttrib().name(),
                   proc.getEdgeNormsAttrib().name());

    // N[0] is the normal for the edge we are intersecting from the regular bounding box, pointing
    // out of the octagon.
    v->codeAppendf("float4 devbounds = %s;",
                   proc.getInstanceAttrib(InstanceAttribs::kDevBounds).name());
    v->codeAppend ("float2 refpt = (0 == sk_VertexID >> 2)"
                           "? float2(min(devbounds.x, devbounds.z), devbounds.y)"
                           ": float2(max(devbounds.x, devbounds.z), devbounds.w);");

    // N[1] is the normal for the edge we are intersecting from the 45-degree bounding box, pointing
    // out of the octagon.
    v->codeAppendf("float2 refpt45 = (0 == ((sk_VertexID + 1) & (1 << 2))) ? %s.xy : %s.zw;",
                   proc.getInstanceAttrib(InstanceAttribs::kDevBounds45).name(),
                   proc.getInstanceAttrib(InstanceAttribs::kDevBounds45).name());
    v->codeAppendf("refpt45 *= float2x2(.5,.5,-.5,.5);"); // transform back to device space.

    v->codeAppend ("float2 K = float2(dot(N[0], refpt), dot(N[1], refpt45));");
    v->codeAppendf("float2 octocoord = K * inverse(N);");

    // Round the octagon out to ensure we rasterize every pixel the path might touch. (Positive
    // bloatdir means we should take the "ceil" and negative means to take the "floor".)
    //
    // NOTE: If we were just drawing a rect, ceil/floor would be enough. But since there are also
    // diagonals in the octagon that cross through pixel centers, we need to outset by another
    // quarter px to ensure those pixels get rasterized.
    v->codeAppend ("half2 bloatdir = (0 != N[0].x) "
                           "? half2(half(N[0].x), half(N[1].y))"
                           ": half2(half(N[1].x), half(N[0].y));");
    v->codeAppend ("octocoord = (ceil(octocoord * bloatdir - 1e-4) + 0.25) * bloatdir;");

    gpArgs->fPositionVar.set(kFloat2_GrSLType, "octocoord");

    // Convert to atlas coordinates in order to do our texture lookup.
    v->codeAppendf("float2 atlascoord = octocoord + float2(%s);",
                   proc.getInstanceAttrib(InstanceAttribs::kDevToAtlasOffset).name());
    if (kTopLeft_GrSurfaceOrigin == proc.atlasOrigin()) {
        v->codeAppendf("%s.xy = atlascoord * %s;", texcoord.vsOut(), atlasAdjust);
    } else {
        SkASSERT(kBottomLeft_GrSurfaceOrigin == proc.atlasOrigin());
        v->codeAppendf("%s.xy = float2(atlascoord.x * %s.x, 1 - atlascoord.y * %s.y);",
                       texcoord.vsOut(), atlasAdjust, atlasAdjust);
    }
    // The third texture coordinate is -.5 for even-odd paths and +.5 for winding ones.
    // ("right < left" indicates even-odd fill type.)
    v->codeAppendf("%s.z = sign(devbounds.z - devbounds.x) * .5;", texcoord.vsOut());

    this->emitTransforms(v, varyingHandler, uniHandler, GrShaderVar("octocoord", kFloat2_GrSLType),
                         proc.localMatrix(), args.fFPCoordTransformHandler);

    // Fragment shader.
    GrGLSLFPFragmentBuilder* f = args.fFragBuilder;

    // Look up coverage count in the atlas.
    f->codeAppend ("half coverage = ");
    f->appendTextureLookup(args.fTexSamplers[0], SkStringPrintf("%s.xy", texcoord.fsIn()).c_str(),
                           kFloat2_GrSLType);
    f->codeAppend (".a;");

    // Scale coverage count by .5. Make it negative for even-odd paths and positive for winding
    // ones. Clamp winding coverage counts at 1.0 (i.e. min(coverage/2, .5)).
    f->codeAppendf("coverage = min(abs(coverage) * half(%s.z), .5);", texcoord.fsIn());

    // For negative values, this finishes the even-odd sawtooth function. Since positive (winding)
    // values were clamped at "coverage/2 = .5", this only undoes the previous multiply by .5.
    f->codeAppend ("coverage = 1 - abs(fract(coverage) * 2 - 1);");

    f->codeAppendf("%s = half4(coverage);", args.fOutputCoverage);
}
