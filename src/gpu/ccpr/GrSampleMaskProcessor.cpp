/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ccpr/GrSampleMaskProcessor.h"

#include "src/gpu/GrMesh.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"

class GrSampleMaskProcessor::Impl : public GrGLSLGeometryProcessor {
public:
    Impl(std::unique_ptr<Shader> shader) : fShader(std::move(shader)) {}

private:
    void setData(const GrGLSLProgramDataManager&, const GrPrimitiveProcessor&,
                 FPCoordTransformIter&&) override {}

    void onEmitCode(EmitArgs&, GrGPArgs*) override;

    const std::unique_ptr<Shader> fShader;
};

void GrSampleMaskProcessor::Impl::onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) {
    const GrSampleMaskProcessor& proc = args.fGP.cast<GrSampleMaskProcessor>();
    GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
    GrGLSLVertexBuilder* v = args.fVertBuilder;
    int numInputPoints = proc.numInputPoints();
    int inputWidth = (4 == numInputPoints || proc.hasInputWeight()) ? 4 : 3;

    varyingHandler->emitAttributes(proc);
    SkASSERT(!args.fFPCoordTransformHandler->nextCoordTransform());

    if (PrimitiveType::kTriangles == proc.fPrimitiveType) {
        SkASSERT(!proc.hasInstanceAttributes());  // Triangles are drawn with vertex arrays.
        gpArgs->fPositionVar = proc.fInputAttribs.front().asShaderVar();
    } else {
        SkASSERT(!proc.hasVertexAttributes());  // Curves are drawn with instanced rendering.

        // Shaders expect a global "bloat" variable when calculating gradients.
        v->defineConstant("half", "bloat", ".5");

        const char* swizzle = (4 == numInputPoints || proc.hasInputWeight()) ? "xyzw" : "xyz";
        v->codeAppendf("float%ix2 pts = transpose(float2x%i(X.%s, Y.%s));",
                       inputWidth, inputWidth, swizzle, swizzle);

        const char* hullPts = "pts";
        fShader->emitSetupCode(v, "pts", &hullPts);
        v->codeAppendf("float2 vertexpos = %s[sk_VertexID ^ (sk_VertexID >> 1)];", hullPts);
        gpArgs->fPositionVar.set(kFloat2_GrSLType, "vertexpos");

        fShader->emitVaryings(varyingHandler, GrGLSLVarying::Scope::kVertToFrag,
                              &AccessCodeString(v), "vertexpos", nullptr, nullptr, nullptr);
    }

    // Fragment shader.
    fShader->emitSampleMaskCode(args.fFragBuilder);
}

void GrSampleMaskProcessor::reset(PrimitiveType primitiveType, GrResourceProvider* rp) {
    fPrimitiveType = primitiveType;  // This will affect the return values for numInputPoints, etc.
    SkASSERT(PrimitiveType::kWeightedTriangles != fPrimitiveType);

    this->resetCustomFeatures();
    fInputAttribs.reset();

    switch (fPrimitiveType) {
        case PrimitiveType::kTriangles:
        case PrimitiveType::kWeightedTriangles:
            fInputAttribs.emplace_back("point", kFloat2_GrVertexAttribType, kFloat2_GrSLType);
            this->setVertexAttributes(fInputAttribs.begin(), 1);
            this->setInstanceAttributes(nullptr, 0);
            break;
        case PrimitiveType::kQuadratics:
        case PrimitiveType::kCubics:
        case PrimitiveType::kConics: {
            auto instanceAttribType = (PrimitiveType::kQuadratics == fPrimitiveType)
                    ? kFloat3_GrVertexAttribType : kFloat4_GrVertexAttribType;
            auto shaderVarType = (PrimitiveType::kQuadratics == fPrimitiveType)
                    ? kFloat3_GrSLType : kFloat4_GrSLType;
            fInputAttribs.emplace_back("X", instanceAttribType, shaderVarType);
            fInputAttribs.emplace_back("Y", instanceAttribType, shaderVarType);
            this->setVertexAttributes(nullptr, 0);
            this->setInstanceAttributes(fInputAttribs.begin(), fInputAttribs.count());
            this->setWillUseCustomFeature(CustomFeatures::kSampleLocations);
            break;
        }
    }
}

void GrSampleMaskProcessor::appendMesh(sk_sp<const GrGpuBuffer> instanceBuffer, int instanceCount,
                                       int baseInstance, SkTArray<GrMesh>* out) const {
    SkASSERT(PrimitiveType::kWeightedTriangles != fPrimitiveType);

    switch (fPrimitiveType) {
        case PrimitiveType::kTriangles:
        case PrimitiveType::kWeightedTriangles: {
            GrMesh& mesh = out->emplace_back(GrPrimitiveType::kTriangles);
            mesh.setNonIndexedNonInstanced(instanceCount * 3);
            mesh.setVertexData(std::move(instanceBuffer), baseInstance * 3);
            break;
        }
        case PrimitiveType::kQuadratics:
        case PrimitiveType::kCubics:
        case PrimitiveType::kConics: {
            GrMesh& mesh = out->emplace_back(GrPrimitiveType::kTriangleStrip);
            mesh.setInstanced(std::move(instanceBuffer), instanceCount, baseInstance, 4);
            break;
        }
    }
}

GrGLSLPrimitiveProcessor* GrSampleMaskProcessor::onCreateGLSLInstance(
        std::unique_ptr<Shader> shader) const {
    return new Impl(std::move(shader));
}
