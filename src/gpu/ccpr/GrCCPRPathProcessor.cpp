/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCPRPathProcessor.h"

#include "GrOnFlushResourceProvider.h"
#include "GrTexture.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLProgramBuilder.h"
#include "glsl/GrGLSLVarying.h"

// Slightly undershoot an AA bloat radius of 0.5 so vertices that fall on integer boundaries don't
// accidentally reach into neighboring path masks within the atlas.
constexpr float kAABloatRadius = 0.491111f;

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

// Index buffer for the octagon defined above.
static uint16_t kOctoIndices[GrCCPRPathProcessor::kPerInstanceIndexCount] = {
    0, 4, 2,
    0, 6, 4,
    0, 2, 1,
    2, 4, 3,
    4, 6, 5,
    6, 0, 7,
};

GR_DECLARE_STATIC_UNIQUE_KEY(gIndexBufferKey);

GrCCPRPathProcessor::GrCCPRPathProcessor(GrResourceProvider* rp, sk_sp<GrTextureProxy> atlas,
                                         SkPath::FillType fillType, const GrShaderCaps& shaderCaps)
        : fFillType(fillType) {
    this->addInstanceAttrib("devbounds", kVec4f_GrVertexAttribType, kHigh_GrSLPrecision);
    this->addInstanceAttrib("devbounds45", kVec4f_GrVertexAttribType, kHigh_GrSLPrecision);
    this->addInstanceAttrib("view_matrix", kVec4f_GrVertexAttribType, kHigh_GrSLPrecision);
    this->addInstanceAttrib("view_translate", kVec2f_GrVertexAttribType, kHigh_GrSLPrecision);
    // FIXME: this could be a vector of two shorts if it were supported by Ganesh.
    this->addInstanceAttrib("atlas_offset", kVec2i_GrVertexAttribType, kHigh_GrSLPrecision);
    this->addInstanceAttrib("color", kVec4ub_GrVertexAttribType, kLow_GrSLPrecision);

    SkASSERT(offsetof(Instance, fDevBounds) ==
             this->getInstanceAttrib(InstanceAttribs::kDevBounds).fOffsetInRecord);
    SkASSERT(offsetof(Instance, fDevBounds45) ==
             this->getInstanceAttrib(InstanceAttribs::kDevBounds45).fOffsetInRecord);
    SkASSERT(offsetof(Instance, fViewMatrix) ==
             this->getInstanceAttrib(InstanceAttribs::kViewMatrix).fOffsetInRecord);
    SkASSERT(offsetof(Instance, fViewTranslate) ==
             this->getInstanceAttrib(InstanceAttribs::kViewTranslate).fOffsetInRecord);
    SkASSERT(offsetof(Instance, fAtlasOffset) ==
             this->getInstanceAttrib(InstanceAttribs::kAtlasOffset).fOffsetInRecord);
    SkASSERT(offsetof(Instance, fColor) ==
             this->getInstanceAttrib(InstanceAttribs::kColor).fOffsetInRecord);
    SkASSERT(sizeof(Instance) == this->getInstanceStride());

    GR_STATIC_ASSERT(6 == kNumInstanceAttribs);

    this->addVertexAttrib("edge_norms", kVec4f_GrVertexAttribType, kHigh_GrSLPrecision);

    fAtlasAccess.reset(std::move(atlas), GrSamplerParams::FilterMode::kNone_FilterMode,
                       SkShader::TileMode::kClamp_TileMode, kFragment_GrShaderFlag);
    fAtlasAccess.instantiate(rp);
    this->addTextureSampler(&fAtlasAccess);

    this->initClassID<GrCCPRPathProcessor>();
}

void GrCCPRPathProcessor::getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const {
    b->add32((fFillType << 16) | this->atlas()->origin());
}

class GLSLPathProcessor : public GrGLSLGeometryProcessor {
public:
    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override;

private:
    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& primProc,
                 FPCoordTransformIter&& transformIter) override {
        const GrCCPRPathProcessor& proc = primProc.cast<GrCCPRPathProcessor>();
        pdman.set2f(fAtlasAdjustUniform, 1.0f / proc.atlas()->width(),
                    1.0f / proc.atlas()->height());
        this->setTransformDataHelper(SkMatrix::I(), pdman, &transformIter);
    }

    GrGLSLUniformHandler::UniformHandle fAtlasAdjustUniform;

    typedef GrGLSLGeometryProcessor INHERITED;
};

GrGLSLPrimitiveProcessor* GrCCPRPathProcessor::createGLSLInstance(const GrShaderCaps&) const {
    return new GLSLPathProcessor();
}

void GLSLPathProcessor::onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) {
    using InstanceAttribs = GrCCPRPathProcessor::InstanceAttribs;
    const GrCCPRPathProcessor& proc = args.fGP.cast<GrCCPRPathProcessor>();
    GrGLSLUniformHandler* uniHandler = args.fUniformHandler;
    GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;

    const char* atlasAdjust;
    fAtlasAdjustUniform = uniHandler->addUniform(
            kVertex_GrShaderFlag,
            kVec2f_GrSLType, kHigh_GrSLPrecision, "atlas_adjust", &atlasAdjust);

    varyingHandler->emitAttributes(proc);

    GrGLSLVertToFrag texcoord(kVec2f_GrSLType);
    GrGLSLVertToFrag color(kVec4f_GrSLType);
    varyingHandler->addVarying("texcoord", &texcoord, kHigh_GrSLPrecision);
    varyingHandler->addFlatPassThroughAttribute(&proc.getInstanceAttrib(InstanceAttribs::kColor),
                                                args.fOutputColor, kLow_GrSLPrecision);

    // Vertex shader.
    GrGLSLVertexBuilder* v = args.fVertBuilder;

    // Find the intersections of (bloated) devBounds and devBounds45 in order to come up with an
    // octagon that circumscribes the (bloated) path. A vertex is the intersection of two lines:
    // one edge from the path's bounding box and one edge from its 45-degree bounding box.
    v->codeAppendf("highp mat2 N = mat2(%s);", proc.getEdgeNormsAttrib().fName);

    // N[0] is the normal for the edge we are intersecting from the regular bounding box, pointing
    // out of the octagon.
    v->codeAppendf("highp vec2 refpt = (min(N[0].x, N[0].y) < 0) ? %s.xy : %s.zw;",
                   proc.getInstanceAttrib(InstanceAttribs::kDevBounds).fName,
                   proc.getInstanceAttrib(InstanceAttribs::kDevBounds).fName);
    v->codeAppendf("refpt += N[0] * %f;", kAABloatRadius); // bloat for AA.

    // N[1] is the normal for the edge we are intersecting from the 45-degree bounding box, pointing
    // out of the octagon.
    v->codeAppendf("highp vec2 refpt45 = (N[1].x < 0) ? %s.xy : %s.zw;",
                   proc.getInstanceAttrib(InstanceAttribs::kDevBounds45).fName,
                   proc.getInstanceAttrib(InstanceAttribs::kDevBounds45).fName);
    v->codeAppendf("refpt45 *= mat2(.5,.5,-.5,.5);"); // transform back to device space.
    v->codeAppendf("refpt45 += N[1] * %f;", kAABloatRadius); // bloat for AA.

    v->codeAppend ("highp vec2 K = vec2(dot(N[0], refpt), dot(N[1], refpt45));");
    v->codeAppendf("highp vec2 octocoord = K * inverse(N);");

    gpArgs->fPositionVar.set(kVec2f_GrSLType, "octocoord");

    // Convert to atlas coordinates in order to do our texture lookup.
    v->codeAppendf("highp vec2 atlascoord = octocoord + vec2(%s);",
                   proc.getInstanceAttrib(InstanceAttribs::kAtlasOffset).fName);
    if (kTopLeft_GrSurfaceOrigin == proc.atlas()->origin()) {
        v->codeAppendf("%s = atlascoord * %s;", texcoord.vsOut(), atlasAdjust);
    } else {
        SkASSERT(kBottomLeft_GrSurfaceOrigin == proc.atlas()->origin());
        v->codeAppendf("%s = vec2(atlascoord.x * %s.x, 1 - atlascoord.y * %s.y);",
                       texcoord.vsOut(), atlasAdjust, atlasAdjust);
    }

    // Convert to (local) path cordinates.
    v->codeAppendf("highp vec2 pathcoord = inverse(mat2(%s)) * (octocoord - %s);",
                   proc.getInstanceAttrib(InstanceAttribs::kViewMatrix).fName,
                   proc.getInstanceAttrib(InstanceAttribs::kViewTranslate).fName);

    this->emitTransforms(v, varyingHandler, uniHandler, gpArgs->fPositionVar, "pathcoord",
                         args.fFPCoordTransformHandler);

    // Fragment shader.
    GrGLSLPPFragmentBuilder* f = args.fFragBuilder;

    f->codeAppend ("mediump float coverage_count = ");
    f->appendTextureLookup(args.fTexSamplers[0], texcoord.fsIn(), kVec2f_GrSLType);
    f->codeAppend (".a;");

    if (SkPath::kWinding_FillType == proc.fillType()) {
        f->codeAppendf("%s = vec4(min(abs(coverage_count), 1));", args.fOutputCoverage);
    } else {
        SkASSERT(SkPath::kEvenOdd_FillType == proc.fillType());
        f->codeAppend ("mediump float t = mod(abs(coverage_count), 2);");
        f->codeAppendf("%s = vec4(1 - abs(t - 1));", args.fOutputCoverage);
    }
}

sk_sp<GrBuffer> GrCCPRPathProcessor::FindOrMakeIndexBuffer(GrOnFlushResourceProvider* onFlushRP) {
    GR_DEFINE_STATIC_UNIQUE_KEY(gIndexBufferKey);
    return onFlushRP->findOrMakeStaticBuffer(gIndexBufferKey, kIndex_GrBufferType,
                                             sizeof(kOctoIndices), kOctoIndices);
}

sk_sp<GrBuffer> GrCCPRPathProcessor::FindOrMakeVertexBuffer(GrOnFlushResourceProvider* onFlushRP) {
    GR_DEFINE_STATIC_UNIQUE_KEY(gVertexBufferKey);
    return onFlushRP->findOrMakeStaticBuffer(gVertexBufferKey, kVertex_GrBufferType,
                                             sizeof(kOctoEdgeNorms), kOctoEdgeNorms);
}
