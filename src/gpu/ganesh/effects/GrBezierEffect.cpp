/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/effects/GrBezierEffect.h"

#include "include/core/SkColor.h"
#include "src/base/SkRandom.h"
#include "src/gpu/KeyBuilder.h"
#include "src/gpu/ganesh/GrColor.h"
#include "src/gpu/ganesh/GrShaderVar.h"
#include "src/gpu/ganesh/GrTestUtils.h"
#include "src/gpu/ganesh/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ganesh/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/ganesh/glsl/GrGLSLUniformHandler.h"
#include "src/gpu/ganesh/glsl/GrGLSLVarying.h"
#include "src/gpu/ganesh/glsl/GrGLSLVertexGeoBuilder.h"

#include <iterator>

class GrConicEffect::Impl : public ProgramImpl {
public:
    void setData(const GrGLSLProgramDataManager& pdman,
                 const GrShaderCaps& shaderCaps,
                 const GrGeometryProcessor& geomProc) override {
        const GrConicEffect& ce = geomProc.cast<GrConicEffect>();

        SetTransform(pdman, shaderCaps,  fViewMatrixUniform,  ce.fViewMatrix,  &fViewMatrix);
        SetTransform(pdman, shaderCaps, fLocalMatrixUniform, ce.fLocalMatrix, &fLocalMatrix);

        if (fColor != ce.fColor) {
            pdman.set4fv(fColorUniform, 1, ce.fColor.vec());
            fColor = ce.fColor;
        }

        if (ce.fCoverageScale != 0xff && ce.fCoverageScale != fCoverageScale) {
            pdman.set1f(fCoverageScaleUniform, GrNormalizeByteToFloat(ce.fCoverageScale));
            fCoverageScale = ce.fCoverageScale;
        }
    }

private:
    void onEmitCode(EmitArgs&, GrGPArgs*) override;

    SkMatrix    fViewMatrix    = SkMatrix::InvalidMatrix();
    SkMatrix    fLocalMatrix   = SkMatrix::InvalidMatrix();
    SkPMColor4f fColor         = SK_PMColor4fILLEGAL;
    uint8_t     fCoverageScale = 0xFF;

    UniformHandle fColorUniform;
    UniformHandle fCoverageScaleUniform;
    UniformHandle fViewMatrixUniform;
    UniformHandle fLocalMatrixUniform;
};

void GrConicEffect::Impl::onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) {
    GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
    const GrConicEffect& gp = args.fGeomProc.cast<GrConicEffect>();
    GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

    // emit attributes
    varyingHandler->emitAttributes(gp);

    GrGLSLVarying v(SkSLType::kFloat4);
    varyingHandler->addVarying("ConicCoeffs", &v);
    vertBuilder->codeAppendf("%s = %s;", v.vsOut(), gp.inConicCoeffs().name());

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    // Setup pass through color
    fragBuilder->codeAppendf("half4 %s;", args.fOutputColor);
    this->setupUniformColor(fragBuilder, uniformHandler, args.fOutputColor, &fColorUniform);

    // Setup position
    WriteOutputPosition(vertBuilder,
                        uniformHandler,
                        *args.fShaderCaps,
                        gpArgs,
                        gp.inPosition().name(),
                        gp.fViewMatrix,
                        &fViewMatrixUniform);
    if (gp.fUsesLocalCoords) {
        WriteLocalCoord(vertBuilder,
                        uniformHandler,
                        *args.fShaderCaps,
                        gpArgs,
                        gp.inPosition().asShaderVar(),
                        gp.fLocalMatrix,
                        &fLocalMatrixUniform);
    }

    // TODO: we should check on the number of bits float and half provide and use the smallest one
    // that suffices. Additionally we should assert that the upstream code only lets us get here if
    // either float or half provides the required number of bits.

    GrShaderVar edgeAlpha("edgeAlpha", SkSLType::kHalf, 0);
    GrShaderVar dklmdx("dklmdx", SkSLType::kFloat3, 0);
    GrShaderVar dklmdy("dklmdy", SkSLType::kFloat3, 0);
    GrShaderVar dfdx("dfdx", SkSLType::kFloat, 0);
    GrShaderVar dfdy("dfdy", SkSLType::kFloat, 0);
    GrShaderVar gF("gF", SkSLType::kFloat2, 0);
    GrShaderVar gFM("gFM", SkSLType::kFloat, 0);
    GrShaderVar func("func", SkSLType::kFloat, 0);

    fragBuilder->declAppend(edgeAlpha);
    fragBuilder->declAppend(dklmdx);
    fragBuilder->declAppend(dklmdy);
    fragBuilder->declAppend(dfdx);
    fragBuilder->declAppend(dfdy);
    fragBuilder->declAppend(gF);
    fragBuilder->declAppend(gFM);
    fragBuilder->declAppend(func);

    fragBuilder->codeAppendf("%s = dFdx(%s.xyz);", dklmdx.c_str(), v.fsIn());
    fragBuilder->codeAppendf("%s = dFdy(%s.xyz);", dklmdy.c_str(), v.fsIn());
    fragBuilder->codeAppendf("%s = 2.0 * %s.x * %s.x - %s.y * %s.z - %s.z * %s.y;",
                             dfdx.c_str(),
                             v.fsIn(), dklmdx.c_str(),
                             v.fsIn(), dklmdx.c_str(),
                             v.fsIn(), dklmdx.c_str());
    fragBuilder->codeAppendf("%s = 2.0 * %s.x * %s.x - %s.y * %s.z - %s.z * %s.y;",
                             dfdy.c_str(),
                             v.fsIn(), dklmdy.c_str(),
                             v.fsIn(), dklmdy.c_str(),
                             v.fsIn(), dklmdy.c_str());
    fragBuilder->codeAppendf("%s = float2(%s, %s);", gF.c_str(), dfdx.c_str(),
                             dfdy.c_str());
    fragBuilder->codeAppendf("%s = sqrt(dot(%s, %s));",
                             gFM.c_str(), gF.c_str(), gF.c_str());
    fragBuilder->codeAppendf("%s = %s.x*%s.x - %s.y*%s.z;",
                             func.c_str(), v.fsIn(), v.fsIn(), v.fsIn(), v.fsIn());
    fragBuilder->codeAppendf("%s = abs(%s);", func.c_str(), func.c_str());
    fragBuilder->codeAppendf("%s = half(%s / %s);",
                             edgeAlpha.c_str(), func.c_str(), gFM.c_str());
    fragBuilder->codeAppendf("%s = max(1.0 - %s, 0.0);",
                             edgeAlpha.c_str(), edgeAlpha.c_str());
    // Add line below for smooth cubic ramp
    // fragBuilder->codeAppend("edgeAlpha = edgeAlpha*edgeAlpha*(3.0-2.0*edgeAlpha);");

    // TODO should we really be doing this?
    if (gp.fCoverageScale != 0xff) {
        const char* coverageScale;
        fCoverageScaleUniform = uniformHandler->addUniform(nullptr,
                                                           kFragment_GrShaderFlag,
                                                           SkSLType::kFloat,
                                                           "Coverage",
                                                           &coverageScale);
        fragBuilder->codeAppendf("half4 %s = half4(half(%s) * %s);",
                                 args.fOutputCoverage, coverageScale, edgeAlpha.c_str());
    } else {
        fragBuilder->codeAppendf("half4 %s = half4(%s);", args.fOutputCoverage, edgeAlpha.c_str());
    }
}

//////////////////////////////////////////////////////////////////////////////

GrConicEffect::~GrConicEffect() = default;

void GrConicEffect::addToKey(const GrShaderCaps& caps, skgpu::KeyBuilder* b) const {
    uint32_t key = 0;
    key |= fCoverageScale == 0xff ? 0x8  : 0x0;
    key |= fUsesLocalCoords       ? 0x10 : 0x0;
    key = ProgramImpl::AddMatrixKeys(caps,
                                     key,
                                     fViewMatrix,
                                     fUsesLocalCoords ? fLocalMatrix : SkMatrix::I());
    b->add32(key);
}

std::unique_ptr<GrGeometryProcessor::ProgramImpl> GrConicEffect::makeProgramImpl(
        const GrShaderCaps&) const {
    return std::make_unique<Impl>();
}

GrConicEffect::GrConicEffect(const SkPMColor4f& color, const SkMatrix& viewMatrix, uint8_t coverage,
                             const SkMatrix& localMatrix, bool usesLocalCoords)
        : INHERITED(kGrConicEffect_ClassID)
        , fColor(color)
        , fViewMatrix(viewMatrix)
        , fLocalMatrix(viewMatrix)
        , fUsesLocalCoords(usesLocalCoords)
        , fCoverageScale(coverage) {
    this->setVertexAttributesWithImplicitOffsets(kAttributes, std::size(kAttributes));
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(GrConicEffect)

#if defined(GPU_TEST_UTILS)
GrGeometryProcessor* GrConicEffect::TestCreate(GrProcessorTestData* d) {
    GrColor color = GrTest::RandomColor(d->fRandom);
    SkMatrix viewMatrix = GrTest::TestMatrix(d->fRandom);
    SkMatrix localMatrix = GrTest::TestMatrix(d->fRandom);
    bool usesLocalCoords = d->fRandom->nextBool();
    return GrConicEffect::Make(d->allocator(),
                               SkPMColor4f::FromBytes_RGBA(color),
                               viewMatrix,
                               *d->caps(),
                               localMatrix,
                               usesLocalCoords);
}
#endif

//////////////////////////////////////////////////////////////////////////////
// Quad
//////////////////////////////////////////////////////////////////////////////

class GrQuadEffect::Impl : public ProgramImpl {
public:
    void setData(const GrGLSLProgramDataManager& pdman,
                 const GrShaderCaps& shaderCaps,
                 const GrGeometryProcessor& geomProc) override {
        const GrQuadEffect& qe = geomProc.cast<GrQuadEffect>();

        SetTransform(pdman, shaderCaps,  fViewMatrixUniform,  qe.fViewMatrix, &fViewMatrix);
        SetTransform(pdman, shaderCaps, fLocalMatrixUniform, qe.fLocalMatrix, &fLocalMatrix);

        if (qe.fColor != fColor) {
            pdman.set4fv(fColorUniform, 1, qe.fColor.vec());
            fColor = qe.fColor;
        }

        if (qe.fCoverageScale != 0xff && qe.fCoverageScale != fCoverageScale) {
            pdman.set1f(fCoverageScaleUniform, GrNormalizeByteToFloat(qe.fCoverageScale));
            fCoverageScale = qe.fCoverageScale;
        }
    }

private:
    void onEmitCode(EmitArgs&, GrGPArgs*) override;

    SkMatrix    fViewMatrix     = SkMatrix::InvalidMatrix();
    SkMatrix    fLocalMatrix    = SkMatrix::InvalidMatrix();
    SkPMColor4f fColor          = SK_PMColor4fILLEGAL;
    uint8_t     fCoverageScale  = 0xFF;

    UniformHandle fColorUniform;
    UniformHandle fCoverageScaleUniform;
    UniformHandle fViewMatrixUniform;
    UniformHandle fLocalMatrixUniform;
};

void GrQuadEffect::Impl::onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) {
    GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
    const GrQuadEffect& gp = args.fGeomProc.cast<GrQuadEffect>();
    GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

    // emit attributes
    varyingHandler->emitAttributes(gp);

    GrGLSLVarying v(SkSLType::kHalf4);
    varyingHandler->addVarying("HairQuadEdge", &v);
    vertBuilder->codeAppendf("%s = %s;", v.vsOut(), gp.inHairQuadEdge().name());

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    // Setup pass through color
    fragBuilder->codeAppendf("half4 %s;", args.fOutputColor);
    this->setupUniformColor(fragBuilder, uniformHandler, args.fOutputColor, &fColorUniform);

    // Setup position
    WriteOutputPosition(vertBuilder,
                        uniformHandler,
                        *args.fShaderCaps,
                        gpArgs,
                        gp.inPosition().name(),
                        gp.fViewMatrix,
                        &fViewMatrixUniform);
    if (gp.fUsesLocalCoords) {
        WriteLocalCoord(vertBuilder,
                        uniformHandler,
                        *args.fShaderCaps,
                        gpArgs,
                        gp.inPosition().asShaderVar(),
                        gp.fLocalMatrix,
                        &fLocalMatrixUniform);
    }

    fragBuilder->codeAppendf("half edgeAlpha;");

    fragBuilder->codeAppendf("half2 duvdx = half2(dFdx(%s.xy));", v.fsIn());
    fragBuilder->codeAppendf("half2 duvdy = half2(dFdy(%s.xy));", v.fsIn());
    fragBuilder->codeAppendf("half2 gF = half2(2.0 * %s.x * duvdx.x - duvdx.y,"
                             "               2.0 * %s.x * duvdy.x - duvdy.y);",
                             v.fsIn(), v.fsIn());
    fragBuilder->codeAppendf("edgeAlpha = half(%s.x * %s.x - %s.y);",
                             v.fsIn(), v.fsIn(), v.fsIn());
    fragBuilder->codeAppend("edgeAlpha = sqrt(edgeAlpha * edgeAlpha / dot(gF, gF));");
    fragBuilder->codeAppend("edgeAlpha = max(1.0 - edgeAlpha, 0.0);");
    // Add line below for smooth cubic ramp
    // fragBuilder->codeAppend("edgeAlpha = edgeAlpha*edgeAlpha*(3.0-2.0*edgeAlpha);");

    if (gp.fCoverageScale != 0xFF) {
        const char* coverageScale;
        fCoverageScaleUniform = uniformHandler->addUniform(nullptr,
                                                           kFragment_GrShaderFlag,
                                                           SkSLType::kHalf,
                                                           "Coverage",
                                                           &coverageScale);
        fragBuilder->codeAppendf("half4 %s = half4(%s * edgeAlpha);", args.fOutputCoverage,
                                 coverageScale);
    } else {
        fragBuilder->codeAppendf("half4 %s = half4(edgeAlpha);", args.fOutputCoverage);
    }
}

//////////////////////////////////////////////////////////////////////////////

GrQuadEffect::~GrQuadEffect() = default;

void GrQuadEffect::addToKey(const GrShaderCaps& caps, skgpu::KeyBuilder* b) const {
    uint32_t key = 0;
    key |= fCoverageScale != 0xff ? 0x8  : 0x0;
    key |= fUsesLocalCoords       ? 0x10 : 0x0;
    key = ProgramImpl::AddMatrixKeys(caps,
                                     key,
                                     fViewMatrix,
                                     fUsesLocalCoords ? fLocalMatrix : SkMatrix::I());
    b->add32(key);
}

std::unique_ptr<GrGeometryProcessor::ProgramImpl> GrQuadEffect::makeProgramImpl(
        const GrShaderCaps&) const {
    return std::make_unique<Impl>();
}

GrQuadEffect::GrQuadEffect(const SkPMColor4f& color, const SkMatrix& viewMatrix, uint8_t coverage,
                           const SkMatrix& localMatrix, bool usesLocalCoords)
    : INHERITED(kGrQuadEffect_ClassID)
    , fColor(color)
    , fViewMatrix(viewMatrix)
    , fLocalMatrix(localMatrix)
    , fUsesLocalCoords(usesLocalCoords)
    , fCoverageScale(coverage) {
    this->setVertexAttributesWithImplicitOffsets(kAttributes, std::size(kAttributes));
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(GrQuadEffect)

#if defined(GPU_TEST_UTILS)
GrGeometryProcessor* GrQuadEffect::TestCreate(GrProcessorTestData* d) {
    GrColor color = GrTest::RandomColor(d->fRandom);
    SkMatrix viewMatrix = GrTest::TestMatrix(d->fRandom);
    SkMatrix localMatrix = GrTest::TestMatrix(d->fRandom);
    bool usesLocalCoords = d->fRandom->nextBool();
    return GrQuadEffect::Make(d->allocator(),
                              SkPMColor4f::FromBytes_RGBA(color),
                              viewMatrix,
                              *d->caps(),
                              localMatrix,
                              usesLocalCoords);
}
#endif
