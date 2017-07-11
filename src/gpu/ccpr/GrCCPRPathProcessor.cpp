/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCPRPathProcessor.h"

#include "GrTexture.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLProgramBuilder.h"
#include "glsl/GrGLSLVarying.h"

// Slightly undershoot an AA bloat radius of 0.5 so vertices that fall on integer boundaries don't
// accidentally reach into neighboring path masks within the atlas.
constexpr float kAABloatRadius = 0.491111f;

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
             this->getAttrib(InstanceAttribs::kDevBounds).fOffsetInRecord);
    SkASSERT(offsetof(Instance, fDevBounds45) ==
             this->getAttrib(InstanceAttribs::kDevBounds45).fOffsetInRecord);
    SkASSERT(offsetof(Instance, fViewMatrix) ==
             this->getAttrib(InstanceAttribs::kViewMatrix).fOffsetInRecord);
    SkASSERT(offsetof(Instance, fViewTranslate) ==
             this->getAttrib(InstanceAttribs::kViewTranslate).fOffsetInRecord);
    SkASSERT(offsetof(Instance, fAtlasOffset) ==
             this->getAttrib(InstanceAttribs::kAtlasOffset).fOffsetInRecord);
    SkASSERT(offsetof(Instance, fColor) ==
             this->getAttrib(InstanceAttribs::kColor).fOffsetInRecord);
    SkASSERT(sizeof(Instance) == this->getInstanceStride());

    GR_STATIC_ASSERT(6 == kNumInstanceAttribs);

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
    varyingHandler->addFlatPassThroughAttribute(&proc.getAttrib(InstanceAttribs::kColor),
                                                args.fOutputColor, kLow_GrSLPrecision);

    // Vertex shader.
    GrGLSLVertexBuilder* v = args.fVertBuilder;

    // Find the intersections of (bloated) devBounds and devBounds45 in order to come up with our
    // octagon cover geometry.
    v->defineConstant("lowp vec2[4]", "edge_norms",
                      "vec2[4](vec2(-1,0), vec2(0,-1), vec2(+1,0), vec2(0,+1))");
    v->defineConstant("lowp vec2[4]", "edge_norms45",
                      "vec2[4](vec2(-1,+1), vec2(-1,-1), vec2(+1,-1), vec2(+1,+1))");

    v->codeAppend ("int fanidx = int[8](0,1,7,2,6,3,5,4)[sk_VertexID];"); // because kTriangleStrip.
    v->codeAppend ("int edgeidx = fanidx / 2;");
    v->codeAppend ("int edgeidx45 = ((fanidx + 1) % 8) / 2;");
    v->codeAppend ("highp mat2 N = mat2(edge_norms[edgeidx], edge_norms45[edgeidx45]);");

    v->codeAppendf("highp vec2 refpt = mat2(%s)[edgeidx/2];",
                   proc.getAttrib(InstanceAttribs::kDevBounds).fName);
    v->codeAppendf("refpt += N[0] * %f;", kAABloatRadius); // bloat for AA.

    v->codeAppendf("highp vec2 refpt45 = mat2(%s)[edgeidx45/2];",
                   proc.getAttrib(InstanceAttribs::kDevBounds45).fName);
    v->codeAppendf("refpt45 *= mat2(.5,.5,-.5,.5);"); // xform back to device space.
    v->codeAppendf("refpt45 += N[1] * %f;", kAABloatRadius); // bloat for AA.

    v->codeAppend ("highp vec2 K = vec2(dot(N[0], refpt), dot(N[1], refpt45));");
    v->codeAppendf("highp vec2 octocoord = K * inverse(N);");

    gpArgs->fPositionVar.set(kVec2f_GrSLType, "octocoord");

    // Convert to atlas coordinates in order to do our texture lookup.
    v->codeAppendf("highp vec2 atlascoord = octocoord + vec2(%s);",
                   proc.getAttrib(InstanceAttribs::kAtlasOffset).fName);
    if (kTopLeft_GrSurfaceOrigin == proc.atlas()->origin()) {
        v->codeAppendf("%s = atlascoord * %s;", texcoord.vsOut(), atlasAdjust);
    } else {
        SkASSERT(kBottomLeft_GrSurfaceOrigin == proc.atlas()->origin());
        v->codeAppendf("%s = vec2(atlascoord.x * %s.x, 1 - atlascoord.y * %s.y);",
                       texcoord.vsOut(), atlasAdjust, atlasAdjust);
    }

    // Convert to (local) path cordinates.
    v->codeAppendf("highp vec2 pathcoord = inverse(mat2(%s)) * (octocoord - %s);",
                   proc.getAttrib(InstanceAttribs::kViewMatrix).fName,
                   proc.getAttrib(InstanceAttribs::kViewTranslate).fName);

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
