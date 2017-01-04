/*
* Copyright 2013 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrPathProcessor.h"

#include "GrShaderCaps.h"
#include "gl/GrGLGpu.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "glsl/GrGLSLVarying.h"

class GrGLPathProcessor : public GrGLSLPrimitiveProcessor {
public:
    GrGLPathProcessor() : fColor(GrColor_ILLEGAL) {}

    static void GenKey(const GrPathProcessor& pathProc,
                       const GrShaderCaps&,
                       GrProcessorKeyBuilder* b) {
        b->add32(SkToInt(pathProc.viewMatrix().hasPerspective()));
    }

    void emitCode(EmitArgs& args) override {
        GrGLSLPPFragmentBuilder* fragBuilder = args.fFragBuilder;
        const GrPathProcessor& pathProc = args.fGP.cast<GrPathProcessor>();

        if (!pathProc.viewMatrix().hasPerspective()) {
            args.fVaryingHandler->setNoPerspective();
        }

        // emit transforms
        this->emitTransforms(args.fVaryingHandler, args.fFPCoordTransformHandler);

        // Setup uniform color
        const char* stagedLocalVarName;
        fColorUniform = args.fUniformHandler->addUniform(kFragment_GrShaderFlag,
                                                         kVec4f_GrSLType,
                                                         kDefault_GrSLPrecision,
                                                         "Color",
                                                         &stagedLocalVarName);
        fragBuilder->codeAppendf("%s = %s;", args.fOutputColor, stagedLocalVarName);

        // setup constant solid coverage
        fragBuilder->codeAppendf("%s = vec4(1);", args.fOutputCoverage);
    }

    void emitTransforms(GrGLSLVaryingHandler* varyingHandler,
                        FPCoordTransformHandler* transformHandler) {
        int i = 0;
        while (const GrCoordTransform* coordTransform = transformHandler->nextCoordTransform()) {
            GrSLType varyingType =
                    coordTransform->getMatrix().hasPerspective() ? kVec3f_GrSLType
                                                                 : kVec2f_GrSLType;

            SkString strVaryingName;
            strVaryingName.printf("TransformedCoord_%d", i);
            GrGLSLVertToFrag v(varyingType);
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
            float c[4];
            GrColorToRGBAFloat(pathProc.color(), c);
            pd.set4fv(fColorUniform, 1, c);
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

            SkASSERT(fInstalledTransforms[t].fType == kVec2f_GrSLType ||
                     fInstalledTransforms[t].fType == kVec3f_GrSLType);
            unsigned components = fInstalledTransforms[t].fType == kVec2f_GrSLType ? 2 : 3;
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
    GrColor fColor;

    typedef GrGLSLPrimitiveProcessor INHERITED;
};

GrPathProcessor::GrPathProcessor(GrColor color,
                                 const SkMatrix& viewMatrix,
                                 const SkMatrix& localMatrix)
        : fColor(color)
        , fViewMatrix(viewMatrix)
        , fLocalMatrix(localMatrix) {
    this->initClassID<GrPathProcessor>();
}

void GrPathProcessor::getGLSLProcessorKey(const GrShaderCaps& caps,
                                          GrProcessorKeyBuilder* b) const {
    GrGLPathProcessor::GenKey(*this, caps, b);
}

GrGLSLPrimitiveProcessor* GrPathProcessor::createGLSLInstance(const GrShaderCaps& caps) const {
    SkASSERT(caps.pathRenderingSupport());
    return new GrGLPathProcessor();
}
