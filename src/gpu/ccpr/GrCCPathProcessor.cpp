/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCPathProcessor.h"

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

sk_sp<const GrBuffer> GrCCPathProcessor::FindVertexBuffer(GrOnFlushResourceProvider* onFlushRP) {
    GR_DEFINE_STATIC_UNIQUE_KEY(gVertexBufferKey);
    return onFlushRP->findOrMakeStaticBuffer(kVertex_GrBufferType, sizeof(kOctoEdgeNorms),
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

sk_sp<const GrBuffer> GrCCPathProcessor::FindIndexBuffer(GrOnFlushResourceProvider* onFlushRP) {
    GR_DEFINE_STATIC_UNIQUE_KEY(gIndexBufferKey);
    if (onFlushRP->caps()->usePrimitiveRestart()) {
        return onFlushRP->findOrMakeStaticBuffer(kIndex_GrBufferType, sizeof(kOctoIndicesAsStrips),
                                                 kOctoIndicesAsStrips, gIndexBufferKey);
    } else {
        return onFlushRP->findOrMakeStaticBuffer(kIndex_GrBufferType, sizeof(kOctoIndicesAsTris),
                                                 kOctoIndicesAsTris, gIndexBufferKey);
    }
}

int GrCCPathProcessor::NumIndicesPerInstance(const GrCaps& caps) {
    return caps.usePrimitiveRestart() ? SK_ARRAY_COUNT(kOctoIndicesAsStrips)
                                      : SK_ARRAY_COUNT(kOctoIndicesAsTris);
}

GrCCPathProcessor::GrCCPathProcessor(GrResourceProvider* resourceProvider,
                                     sk_sp<GrTextureProxy> atlas, SkPath::FillType fillType)
        : INHERITED(kGrCCPathProcessor_ClassID)
        , fFillType(fillType)
        , fAtlasAccess(std::move(atlas), GrSamplerState::Filter::kNearest,
                       GrSamplerState::WrapMode::kClamp, kFragment_GrShaderFlag) {
    this->addInstanceAttrib("devbounds", kFloat4_GrVertexAttribType);
    this->addInstanceAttrib("devbounds45", kFloat4_GrVertexAttribType);
    this->addInstanceAttrib("view_matrix", kFloat4_GrVertexAttribType);
    this->addInstanceAttrib("view_translate", kFloat2_GrVertexAttribType);
    this->addInstanceAttrib("atlas_offset", kShort2_GrVertexAttribType);
    this->addInstanceAttrib("color", kUByte4_norm_GrVertexAttribType);

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

    this->addVertexAttrib("edge_norms", kFloat4_GrVertexAttribType);

    fAtlasAccess.instantiate(resourceProvider);
    this->addTextureSampler(&fAtlasAccess);

    if (resourceProvider->caps()->usePrimitiveRestart()) {
        this->setWillUsePrimitiveRestart();
    }
}

void GrCCPathProcessor::getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const {
    b->add32((fFillType << 16) | this->atlasProxy()->origin());
}

class GLSLPathProcessor : public GrGLSLGeometryProcessor {
public:
    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override;

private:
    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& primProc,
                 FPCoordTransformIter&& transformIter) override {
        const GrCCPathProcessor& proc = primProc.cast<GrCCPathProcessor>();
        pdman.set2f(fAtlasAdjustUniform, 1.0f / proc.atlas()->width(),
                    1.0f / proc.atlas()->height());
        this->setTransformDataHelper(SkMatrix::I(), pdman, &transformIter);
    }

    GrGLSLUniformHandler::UniformHandle fAtlasAdjustUniform;

    typedef GrGLSLGeometryProcessor INHERITED;
};

GrGLSLPrimitiveProcessor* GrCCPathProcessor::createGLSLInstance(const GrShaderCaps&) const {
    return new GLSLPathProcessor();
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

    GrGLSLVarying texcoord(kFloat2_GrSLType);
    GrGLSLVarying color(kHalf4_GrSLType);
    varyingHandler->addVarying("texcoord", &texcoord);
    varyingHandler->addPassThroughAttribute(&proc.getInstanceAttrib(InstanceAttribs::kColor),
                                            args.fOutputColor, Interpolation::kCanBeFlat);

    // The vertex shader bloats and intersects the devBounds and devBounds45 rectangles, in order to
    // find an octagon that circumscribes the (bloated) path.
    GrGLSLVertexBuilder* v = args.fVertBuilder;

    // Each vertex is the intersection of one edge from devBounds and one from devBounds45.
    // 'N' holds the normals to these edges as column vectors.
    //
    // NOTE: "float2x2(float4)" is valid and equivalent to "float2x2(float4.xy, float4.zw)",
    // however Intel compilers crash when we use the former syntax in this shader.
    v->codeAppendf("float2x2 N = float2x2(%s.xy, %s.zw);",
                   proc.getEdgeNormsAttrib().fName, proc.getEdgeNormsAttrib().fName);

    // N[0] is the normal for the edge we are intersecting from the regular bounding box, pointing
    // out of the octagon.
    v->codeAppendf("float2 refpt = float2[2](%s.xy, %s.zw)[sk_VertexID >> 2];",
                   proc.getInstanceAttrib(InstanceAttribs::kDevBounds).fName,
                   proc.getInstanceAttrib(InstanceAttribs::kDevBounds).fName);
    v->codeAppendf("refpt += N[0] * %f;", kAABloatRadius); // bloat for AA.

    // N[1] is the normal for the edge we are intersecting from the 45-degree bounding box, pointing
    // out of the octagon.
    v->codeAppendf("float2 refpt45 = float2[2](%s.xy, %s.zw)[((sk_VertexID + 1) >> 2) & 1];",
                   proc.getInstanceAttrib(InstanceAttribs::kDevBounds45).fName,
                   proc.getInstanceAttrib(InstanceAttribs::kDevBounds45).fName);
    v->codeAppendf("refpt45 *= float2x2(.5,.5,-.5,.5);"); // transform back to device space.
    v->codeAppendf("refpt45 += N[1] * %f;", kAABloatRadius); // bloat for AA.

    v->codeAppend ("float2 K = float2(dot(N[0], refpt), dot(N[1], refpt45));");
    v->codeAppendf("float2 octocoord = K * inverse(N);");

    gpArgs->fPositionVar.set(kFloat2_GrSLType, "octocoord");

    // Convert to atlas coordinates in order to do our texture lookup.
    v->codeAppendf("float2 atlascoord = octocoord + float2(%s);",
                   proc.getInstanceAttrib(InstanceAttribs::kAtlasOffset).fName);
    if (kTopLeft_GrSurfaceOrigin == proc.atlasProxy()->origin()) {
        v->codeAppendf("%s = atlascoord * %s;", texcoord.vsOut(), atlasAdjust);
    } else {
        SkASSERT(kBottomLeft_GrSurfaceOrigin == proc.atlasProxy()->origin());
        v->codeAppendf("%s = float2(atlascoord.x * %s.x, 1 - atlascoord.y * %s.y);",
                       texcoord.vsOut(), atlasAdjust, atlasAdjust);
    }

    // Convert to path/local cordinates.
    v->codeAppendf("float2x2 viewmatrix = float2x2(%s.xy, %s.zw);", // float2x2(float4) busts Intel.
                   proc.getInstanceAttrib(InstanceAttribs::kViewMatrix).fName,
                   proc.getInstanceAttrib(InstanceAttribs::kViewMatrix).fName);
    v->codeAppendf("float2 pathcoord = inverse(viewmatrix) * (octocoord - %s);",
                   proc.getInstanceAttrib(InstanceAttribs::kViewTranslate).fName);

    this->emitTransforms(v, varyingHandler, uniHandler, GrShaderVar("pathcoord", kFloat2_GrSLType),
                         args.fFPCoordTransformHandler);

    // Fragment shader.
    GrGLSLFPFragmentBuilder* f = args.fFragBuilder;

    f->codeAppend ("half coverage_count = ");
    f->appendTextureLookup(args.fTexSamplers[0], texcoord.fsIn(), kFloat2_GrSLType);
    f->codeAppend (".a;");

    if (SkPath::kWinding_FillType == proc.fillType()) {
        f->codeAppendf("%s = half4(min(abs(coverage_count), 1));", args.fOutputCoverage);
    } else {
        SkASSERT(SkPath::kEvenOdd_FillType == proc.fillType());
        f->codeAppend ("half t = mod(abs(coverage_count), 2);");
        f->codeAppendf("%s = half4(1 - abs(t - 1));", args.fOutputCoverage);
    }
}
