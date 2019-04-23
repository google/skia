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
#include "src/gpu/gl/GrGLVaryingHandler.h"
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
        this->emitTransforms(args.fVaryingHandler, args.fFPCoordTransformHandler);

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

    void emitTransforms(GrGLSLVaryingHandler* varyingHandler,
                        FPCoordTransformHandler* transformHandler) {
        int i = 0;
        while (const GrCoordTransform* coordTransform = transformHandler->nextCoordTransform()) {
            GrSLType varyingType =
                    coordTransform->getMatrix().hasPerspective() ? kHalf3_GrSLType
                                                                 : kHalf2_GrSLType;

            SkString strVaryingName;
            strVaryingName.printf("TransformedCoord_%d", i);
            GrGLSLVarying v(varyingType);
            GrGLVaryingHandler* glVaryingHandler = (GrGLVaryingHandler*) varyingHandler;
            fInstalledTransforms.push_back().fHandle =
                    glVaryingHandler->addPathProcessingVarying(strVaryingName.c_str(),
                                                               &v).toIndex();
            fInstalledTransforms.back().fType = varyingType;

            transformHandler->specifyCoordsForCurrCoordTransform(SkString(v.fsIn()), varyingType);
            ++i;
        }
    }

    void setData(const GrGLSLProgramDataManager& pd,
                 const GrPrimitiveProcessor& primProc,
                 FPCoordTransformIter&& transformIter) override {
        const GrPathProcessor& pathProc = primProc.cast<GrPathProcessor>();
        if (pathProc.color() != fColor) {
            pd.set4fv(fColorUniform, 1, pathProc.color().vec());
            fColor = pathProc.color();
        }

        int t = 0;
        while (const GrCoordTransform* coordTransform = transformIter.next()) {
            SkASSERT(fInstalledTransforms[t].fHandle.isValid());
            const SkMatrix& m = GetTransformMatrix(pathProc.localMatrix(), *coordTransform);
            if (fInstalledTransforms[t].fCurrentValue.cheapEqualTo(m)) {
                continue;
            }
            fInstalledTransforms[t].fCurrentValue = m;

            SkASSERT(fInstalledTransforms[t].fType == kHalf2_GrSLType ||
                     fInstalledTransforms[t].fType == kHalf3_GrSLType);
            unsigned components = fInstalledTransforms[t].fType == kHalf2_GrSLType ? 2 : 3;
            pd.setPathFragmentInputTransform(fInstalledTransforms[t].fHandle, components, m);
            ++t;
        }
    }

private:
    typedef GrGLSLProgramDataManager::VaryingHandle VaryingHandle;
    struct TransformVarying {
        VaryingHandle  fHandle;
        SkMatrix       fCurrentValue = SkMatrix::InvalidMatrix();
        GrSLType       fType = kVoid_GrSLType;
    };

    SkTArray<TransformVarying, true> fInstalledTransforms;

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
