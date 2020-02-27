/*
* Copyright 2013 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/GrPathProcessor.h"

#include "include/private/SkTo.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/gl/GrGLGpu.h"
#ifdef SK_GL
#include "src/gpu/gl/GrGLVaryingHandler.h"
#endif
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLPrimitiveProcessor.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"
#include "src/gpu/glsl/GrGLSLVarying.h"

class GrGLPathProcessor : public GrGLSLPrimitiveProcessor {
public:
    GrGLPathProcessor() : fColor(SK_PMColor4fILLEGAL) {}

    static void GenKey(const GrPathProcessor& pathProc,
                       const GrShaderCaps&,
                       GrProcessorKeyBuilder* b) {
        b->add32(SkToInt(pathProc.viewMatrix().hasPerspective()));
    }

    void emitCode(EmitArgs& args) override {
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        const GrPathProcessor& pathProc = args.fGP.cast<GrPathProcessor>();

        if (!pathProc.viewMatrix().hasPerspective()) {
            args.fVaryingHandler->setNoPerspective();
        }

        // emit transforms
        this->emitTransforms(args.fVaryingHandler, args.fUniformHandler,
                             args.fFPCoordTransformHandler);

        // Setup uniform color
        const char* stagedLocalVarName;
        fColorUniform = args.fUniformHandler->addUniform(kFragment_GrShaderFlag,
                                                         kHalf4_GrSLType,
                                                         "Color",
                                                         &stagedLocalVarName);
        fragBuilder->codeAppendf("%s = %s;", args.fOutputColor, stagedLocalVarName);

        // setup constant solid coverage
        fragBuilder->codeAppendf("%s = half4(1);", args.fOutputCoverage);
    }

    SkString matrix_to_sksl(const SkMatrix& m) {
        return SkStringPrintf("float3x3(%f, %f, %f, %f, %f, %f, %f, %f, %f)", m[0], m[1], m[2],
                              m[3], m[4], m[5], m[6], m[7], m[8]);
    }

    void emitTransforms(GrGLSLVaryingHandler* varyingHandler,
                        GrGLSLUniformHandler* uniformHandler,
                        FPCoordTransformHandler* transformHandler) {
        for (int i = 0; *transformHandler; ++*transformHandler, ++i) {
            auto [coordTransform, fp] = transformHandler->get();
            GrSLType varyingType =
                    coordTransform.matrix().hasPerspective() ? kHalf3_GrSLType : kHalf2_GrSLType;

            SkString matrix;
            UniformHandle uniformHandle;
            GrShaderVar fragmentVar;
            if (fp.isSampledWithExplicitCoords()) {
                if (coordTransform.isNoOp()) {
                    transformHandler->omitCoordsForCurrCoordTransform();
                    continue;
                } else {
                    const char* name;
                    SkString strUniName;
                    strUniName.printf("CoordTransformMatrix_%d", i);
                    fUniformTransform.push_back().fHandle =
                            uniformHandler
                                    ->addUniform(kFragment_GrShaderFlag,
                                                 kFloat3x3_GrSLType,
                                                 strUniName.c_str(),
                                                 &name)
                                    .toIndex();
                    uniformHandle = fUniformTransform.back().fHandle;
                    matrix = name;
                }
            } else {
                SkString strVaryingName;
                strVaryingName.printf("TransformedCoord_%d", i);
                GrGLSLVarying v(varyingType);
#ifdef SK_GL
                GrGLVaryingHandler* glVaryingHandler = (GrGLVaryingHandler*)varyingHandler;
                fVaryingTransform.push_back().fHandle =
                        glVaryingHandler->addPathProcessingVarying(strVaryingName.c_str(), &v)
                                .toIndex();
#endif
                fVaryingTransform.back().fType = varyingType;
                matrix = matrix_to_sksl(coordTransform.matrix());
                fragmentVar = {SkString(v.fsIn()), varyingType};
            }
            transformHandler->specifyCoordsForCurrCoordTransform(matrix, uniformHandle,
                                                                 fragmentVar);
        }
    }

    void setData(const GrGLSLProgramDataManager& pd,
                 const GrPrimitiveProcessor& primProc,
                 const CoordTransformRange& transformRange) override {
        const GrPathProcessor& pathProc = primProc.cast<GrPathProcessor>();
        if (pathProc.color() != fColor) {
            pd.set4fv(fColorUniform, 1, pathProc.color().vec());
            fColor = pathProc.color();
        }

        int v = 0, u = 0;
        for (auto [transform, fp] : transformRange) {
            if (fp.isSampledWithExplicitCoords()) {
                if (transform.isNoOp()) {
                    continue;
                }
                if (fUniformTransform[u].fHandle.isValid()) {
                    SkMatrix m = GetTransformMatrix(transform, SkMatrix::I());
                    if (!SkMatrixPriv::CheapEqual(fUniformTransform[u].fCurrentValue, m)) {
                        fUniformTransform[u].fCurrentValue = m;
                        pd.setSkMatrix(fUniformTransform[u].fHandle.toIndex(), m);
                    }
                }
                ++u;
            } else {
                SkASSERT(fVaryingTransform[v].fHandle.isValid());
                SkMatrix m = GetTransformMatrix(transform, pathProc.localMatrix());
                if (!SkMatrixPriv::CheapEqual(fVaryingTransform[v].fCurrentValue, m)) {
                    fVaryingTransform[v].fCurrentValue = m;
                    SkASSERT(fVaryingTransform[v].fType == kHalf2_GrSLType ||
                             fVaryingTransform[v].fType == kHalf3_GrSLType);
                    int components = fVaryingTransform[v].fType == kHalf2_GrSLType ? 2 : 3;
                    pd.setPathFragmentInputTransform(fVaryingTransform[v].fHandle, components, m);
                }
                ++v;
            }
        }
    }

private:
    using VaryingHandle = GrGLSLProgramDataManager::VaryingHandle;

    // Varying transforms are used for non-explicitly sampled FPs. We provide a matrix
    // to GL as fixed function state and it uses it to compute a varying that we pick up
    // in the FS as the output of the coord transform.
    struct TransformVarying {
        VaryingHandle fHandle;
        SkMatrix      fCurrentValue = SkMatrix::InvalidMatrix();
        GrSLType      fType = kVoid_GrSLType;
    };
    // For explicitly sampled FPs we stick a uniform in the FS and apply it to the explicit coords
    // to implement the CoordTransform.
    struct TransformUniform {
        UniformHandle fHandle;
        SkMatrix      fCurrentValue = SkMatrix::InvalidMatrix();
    };

    SkTArray<TransformVarying, true> fVaryingTransform;
    SkTArray<TransformUniform, true> fUniformTransform;

    UniformHandle fColorUniform;
    SkPMColor4f fColor;

    typedef GrGLSLPrimitiveProcessor INHERITED;
};

GrPathProcessor::GrPathProcessor(const SkPMColor4f& color,
                                 const SkMatrix& viewMatrix,
                                 const SkMatrix& localMatrix)
        : INHERITED(kGrPathProcessor_ClassID)
        , fColor(color)
        , fViewMatrix(viewMatrix)
        , fLocalMatrix(localMatrix) {}

void GrPathProcessor::getGLSLProcessorKey(const GrShaderCaps& caps,
                                          GrProcessorKeyBuilder* b) const {
    GrGLPathProcessor::GenKey(*this, caps, b);
}

GrGLSLPrimitiveProcessor* GrPathProcessor::createGLSLInstance(const GrShaderCaps& caps) const {
    SkASSERT(caps.pathRenderingSupport());
    return new GrGLPathProcessor();
}
